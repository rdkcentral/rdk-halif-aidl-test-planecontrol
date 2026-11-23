/**
 *  If not stated otherwise in this file or this component's LICENSE
 *  file the following copyright and licenses apply:
 *
 *  Copyright 2025 RDK Management
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "vts_planecontrol_test.h"
#include "vts_planecontrol.h"
#include "vts_hdmioutput.h"

#include <algorithm>
#include <chrono>
#include <thread>

bool waitForHDMIOutputState(const android::sp<hdmioutput::IHDMIOutput>& output, hdmioutput::State target, int32_t timeoutMs)
{
    hdmioutput::State state = hdmioutput::State::UNKNOWN;
    /* steady-clock deadline, so the time spent in getState() also counts toward the timeout */
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (true)
    {
        if (output->getState(&state).isOk() && state == target)
        {
            return true;
        }
        if (std::chrono::steady_clock::now() >= deadline)
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    UT_LOG_WARNING("HDMI output did not reach state %d within %d ms (state %d)",
                   static_cast<int>(target), timeoutMs, static_cast<int>(state));
    return false;
}

/* STB display path (same as the audio mixer test): the planes are shown on the HDMI output, so before the test
 * each HDMI output with a connected display (HPD) is started (IHDMIOutputController.start(), allowed only in READY).
 * Outputs already running are left as they are; only the outputs started here are stopped afterwards. */
void PlaneControlTest::startOutputPaths()
{
    for (const auto& output : mHDMIOutputList)
    {
        if (startHdmiOutputPath(output))
        {
            mStartedHDMIOutputs.push_back(output.id.value);
        }
    }
}

void PlaneControlTest::stopOutputPaths()
{
    for (const auto outputId : mStartedHDMIOutputs)
    {
        const auto output = std::find_if(mHDMIOutputList.begin(), mHDMIOutputList.end(),
                                         [outputId](const auto& entry) { return entry.id.value == outputId; });
        if (output != mHDMIOutputList.end())
        {
            HDMIOUTPUT_STOP(output->controller);
            /* stop() goes through STOPPING; close() at teardown needs READY */
            waitForHDMIOutputState(output->output, hdmioutput::State::READY);
        }
    }
    mStartedHDMIOutputs.clear();
}

/* Returns true if the HDMI output was started here (and must be stopped after the test) */
bool PlaneControlTest::startHdmiOutputPath(const PlaneControlHDMIOutputEntry& output)
{
    bool hpdState = false;
    HDMIOUTPUT_GET_HPD_STATE(output.controller, hpdState);
    if (!hpdState)
    {
        /* The plane control spec does not require a display, so the test still runs; only checks that need a
         * displayed picture (e.g. first frame rendered) may fail on platforms that stop the display path */
        UT_LOG_WARNING("HDMI output %d: no display connected (HPD low), output not started; results that need a "
                       "displayed picture may fail", output.id.value);
        return false;
    }

    hdmioutput::HDCPStatus hdcpStatus = hdmioutput::HDCPStatus::UNKNOWN;
    HDMIOUTPUT_GET_HDCP_STATUS(output.controller, hdcpStatus);
    if (hdcpStatus == hdmioutput::HDCPStatus::AUTHENTICATION_FAILURE)
    {
        UT_ASSERT_TRUE(false) << "HDCP authentication failed on HDMI output " << output.id.value;
        return false;
    }
    if (hdcpStatus == hdmioutput::HDCPStatus::AUTHENTICATION_IN_PROGRESS)
    {
        UT_LOG_INFO("HDMI output %d: HDCP authentication in progress", output.id.value);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    hdmioutput::State state = hdmioutput::State::UNKNOWN;
    HDMIOUTPUT_GET_STATE(output.output, state);
    if (state == hdmioutput::State::STARTED)
    {
        UT_LOG_INFO("HDMI output %d is already started, left as it is", output.id.value);
        return false;
    }
    /* start() is allowed only in READY: wait if the output is still in a transitional state (OPENING/STOPPING) */
    if (state != hdmioutput::State::READY && !waitForHDMIOutputState(output.output, hdmioutput::State::READY))
    {
        UT_LOG_WARNING("HDMI output %d is not READY, output not started", output.id.value);
        return false;
    }

    HDMIOUTPUT_START(output.controller);
    return true;
}
