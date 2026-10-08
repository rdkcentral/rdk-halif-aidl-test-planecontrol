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
#include "vts_input_path_helpers.h"

#include <algorithm>
#include <chrono>
#include <thread>

bool PlaneControlTest::startHdmiInputPath(int32_t hdmiInputIndex)
{
    const auto input = vts::input_path::findInputPathEntry(
        mHDMIInputList, hdmiInputIndex,
        [](const auto& entry) { return entry.id.value; });
    UT_ASSERT_TRUE(input != mHDMIInputList.end())
        << "No opened HDMI input found for source index " << hdmiInputIndex;
    if (input == mHDMIInputList.end())
    {
        return false;
    }

    bool connectionState = false;
    HDMIINPUT_GETCONNECTIONSTATE(input->controller, connectionState);
    UT_ASSERT_TRUE(connectionState)
        << "HDMI input is not connected for source index " << hdmiInputIndex;
    if (!connectionState)
    {
        return false;
    }

    hdmiinput::HDCPStatus hdcpStatus;
    HDMIINPUT_GETHDCPSTATUS(input->input, hdcpStatus);
    if (hdcpStatus == hdmiinput::HDCPStatus::AUTHENTICATION_FAILURE)
    {
        UT_ASSERT_TRUE(false) << "HDCP authentication failed for HDMI input " << hdmiInputIndex;
        return false;
    }
    if (hdcpStatus == hdmiinput::HDCPStatus::AUTHENTICATION_IN_PROGRESS)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    HDMIINPUT_START_SUCCESS(input->input, input->controller, true);
    return true;
}

void PlaneControlTest::stopHdmiInputPath(int32_t hdmiInputIndex)
{
    const auto input = vts::input_path::findInputPathEntry(
        mHDMIInputList, hdmiInputIndex,
        [](const auto& entry) { return entry.id.value; });
    if (input != mHDMIInputList.end())
    {
        HDMIINPUT_STOP_SUCCESS(input->input, input->controller, true);
    }
}