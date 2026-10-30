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

/**
 * @addtogroup Plane_Control Plane Control Module
 * @{
 */

/**
 * @addtogroup Plane_Control_Halif_Test Plane Control HALIF Tests
 * @{
 */

/**
 * @defgroup Plane_Control_Halif_Test_TestClass Plane Control Test Class Header
 * @{
 * @parblock
 *
 * ### PlaneControlTest class for Plane Control VTS HALIF AIDL tests :
 *
 * Encapsulates a complete plane control test session (capabilities discovery,
 * plane mapping, property setting, graphics rendering and validation).
 * This class mirrors the VideoSinkTest pattern so that other test modules
 * can include this header to drive a plane control pipeline.
 *
 * **Pre-Conditions:**  None@n
 * **Dependencies:** IPlaneControl service must be initialized via VTS_PlaneControl_init()@n
 *
 * @endparblock
 */

/**
 * @file vts_planecontrol_test.h
 */

#ifndef __VTS_PLANECONTROL_TEST_H__
#define __VTS_PLANECONTROL_TEST_H__

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>

#include "vts_planecontrol.h"
#include "vts_planecontrol_parse_testconfig.h"
#include "vts_planecontrol_graphicscontroller.h"
#include "vts_videosink.h"

struct PlaneControlHDMIInputEntry
{
    hdmiinput::IHDMIInput::Id id;
    android::sp<hdmiinput::IHDMIInput> input;
    android::sp<HDMIInputControllerListener> controlListener;
    android::sp<HDMIInputEventListener> eventListener;
    android::sp<hdmiinput::IHDMIInputController> controller;
    hdmiinput::Capabilities capabilities;
};

struct PlaneControlCompositeInputEntry
{
    int32_t portId;
    android::sp<compositeinput::ICompositeInputPort> inputPort;
    android::sp<CompositeInputControllerListener> controlListener;
    android::sp<CompositeInputEventListener> eventListener;
    android::sp<ICompositeInputController> controller;
};

/**
 * @brief Thread-safe listener that records IGraphicsFbProviderListener callbacks.
 *
 * Used by PlaneControlTest to verify that onGraphicsFbReleased fires after a
 * commitGraphicsFb() call, and to retrieve the old frame-buffer id and the
 * CLOCK_MONOTONIC commit-to-present timestamp.
 *
 * Mirrors the listener pattern used by VideoSinkTest.
 */
class RecordingGraphicsFbProviderListener : public BnGraphicsFbProviderListener
{
public:
    ::android::binder::Status onGraphicsFbReleased(int32_t oldGraphicsFbId,
                                                    int64_t elapsedRealtimeNanos) override
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mCallbackReceived     = true;
        mOldGraphicsFbId      = oldGraphicsFbId;
        mElapsedRealtimeNanos = elapsedRealtimeNanos;
        mCondition.notify_all();
        return ::android::binder::Status::ok();
    }

    /**
     * @brief Block until onGraphicsFbReleased fires or @p timeoutMs elapses.
     * @param[out] oldGraphicsFbId        Id of the buffer replaced (−1 on first commit).
     * @param[out] elapsedRealtimeNanos   CLOCK_MONOTONIC timestamp of the flip.
     * @param[in]  timeoutMs             Maximum wait time in milliseconds.
     * @return true if the callback was received within the timeout.
     */
    bool waitForCallback(int32_t& oldGraphicsFbId, int64_t& elapsedRealtimeNanos,
                         int timeoutMs = 1200)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        const bool ready = mCondition.wait_for(
            lock, std::chrono::milliseconds(timeoutMs),
            [this]() { return mCallbackReceived; });
        if (!ready)
        {
            return false;
        }
        oldGraphicsFbId      = mOldGraphicsFbId;
        elapsedRealtimeNanos = mElapsedRealtimeNanos;
        return true;
    }

    /** @brief Reset state so the listener can be reused for the next commit. */
    void reset()
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mCallbackReceived     = false;
        mOldGraphicsFbId      = -1;
        mElapsedRealtimeNanos = -1;
    }

private:
    std::mutex              mMutex;
    std::condition_variable mCondition;
    bool    mCallbackReceived     = false;
    int32_t mOldGraphicsFbId      = -1;
    int64_t mElapsedRealtimeNanos = -1;
};

/**
 * @brief Validates live plane-to-source mappings against an expected list.
 *
 * Calls getVideoSourceDestinationPlaneMapping(), verifies that the HAL returns
 * one entry per advertised VIDEO plane, and finds each expected mapping by
 * destinationPlaneIndex before comparing its source type and index.
 *
 * @param[in] planeMapping   Expected mappings to verify against.
 */
void checkPlaneMappings(
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> planeMapping);

/**
 * @brief Encapsulates a single Plane Control L3 test run.
 *
 * Mirrors the VideoSinkTest pattern so that other test modules can include
 * this header to drive plane mapping, property changes and graphics rendering
 * as part of a broader AV pipeline test.
 *
 * Typical usage:
 * @code
 *   PlaneControlTest test(planeConfig);
 *   test.run();
 * @endcode
 *
 * The constructor calls getCapabilities() and populates the module-level
 * videoPlaneIndex / graphicsPlaneIndex vectors; run() executes the scenario
 * described by planeConfig.testName.
 */
class PlaneControlTest
{
public:
    /**
     * @brief Construct and discover plane capabilities.
     *
     * Calls getCapabilities() and partitions the results into the module-level
     * videoPlaneIndex and graphicsPlaneIndex vectors used by run().
     *
     * @param[in] planeConfig   Test configuration parsed from the YAML file.
     */
    PlaneControlTest(
        planeControlTestConfig_t planeConfig,
        const std::vector<PlaneControlHDMIInputEntry>& hdmiInputList,
        const std::vector<PlaneControlCompositeInputEntry>& compositeInputList);
    ~PlaneControlTest();

    /**
     * @brief Execute the test scenario described by the config passed at
     *        construction time.
     *
     * Dispatches to the appropriate sub-path based on planeConfig.testName
     * (e.g. Planecontrol_Alpha, Planecontrol_Graphicsplane_Client, …).
     */
    void run();

private:
    void startInputSources();
    void stopInputSources();
    bool startHdmiInputPath(int32_t hdmiInputIndex);
    void stopHdmiInputPath(int32_t hdmiInputIndex);
    bool startCompositeInputPath(int32_t compositeInputIndex);
    void stopCompositeInputPath(int32_t compositeInputIndex);

    ::com::rdk::hal::videosink::IVideoSink::Id mSinkId;
    planeControlTestConfig_t                   mPlaneConfig;
    const std::vector<PlaneControlHDMIInputEntry>& mHDMIInputList;
    const std::vector<PlaneControlCompositeInputEntry>& mCompositeInputList;
    std::vector<int32_t> mStartedHDMIInputs;
    std::vector<int32_t> mStartedCompositeInputs;
};

#endif // __VTS_PLANECONTROL_TEST_H__

/** @} */ // End of Plane_Control_Halif_Test_TestClass
/** @} */ // End of Plane_Control_Halif_Test
/** @} */ // End of Plane_Control
