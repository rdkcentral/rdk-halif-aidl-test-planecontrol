/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**
 * @addtogroup HPK Hardware Porting Kit
 * @{
 *
 */

/**
 * @addtogroup PLANE_CONTROL Plane Control Module
 * @{
 *
 */

/**
 * @defgroup PLANE_CONTROL_HALTEST Plane Control HAL Tests
 * @{
 *
 */

/**
 * @defgroup PLANE_CONTROL_HALTEST_L3 Plane Control HAL Tests L3 File
 * @{
 * @parblock
 *
 * ### L3 Test Cases for Plane Control HAL :
 *
 * This module includes Level 3 functional tests (success and failure scenarios).
 * This is to ensure that the API meets the operational requirements of the module across all vendors.
 *
 * **Pre-Conditions:** @n
 * **Dependencies:** None@n
 *
 * @endparblock
 *
 */

/**
 * @file vts_planecontrol_l3_tests.cpp
 *
 */
#include "vts_videosink_helper_functions.h"
#include "vts_videosink_parse_hfp.h"
#include "vts_videosink_test.h"
#include "vts_planecontrol_parse_testconfig.h"
#include "vts_planecontrol_graphicscontroller.h"
#include "vts_videosink.h"
#include "vts_videodecoder.h"
#include "vts_planecontrol.h"
#include "vts_planecontrol_test.h"
#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <set>
#include <thread>
#include <vector>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>
#include <cerrno>
#include <cinttypes>
#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <cstdio>

using namespace std::chrono;
using namespace std;


/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L3_PlaneControl, UT_TESTS_L3)

static const char *gModuleName = "planecontrol";
static const char *gVideoDecoderModuleName = "videodecoder";
static const char *gTestGroup = "L3";
static IAVBufferHelper* gAVbufferHelper;

static const char *kPlanecontrolGraphicsplaneClient = "Planecontrol_Graphicsplane_Client";

// videoPlaneIndex, graphicsPlaneIndex and planeMapping are defined in vts_planecontrol_test.cpp
extern std::vector<int> videoPlaneIndex;
extern std::vector<int> graphicsPlaneIndex;
extern std::vector<com::rdk::hal::planecontrol::SourcePlaneMapping> planeMapping;

static sp<IAVBuffer> vts_avbuffer_getService()
{
    return
        android::interface_cast<IAVBuffer>(
            android::defaultServiceManager()->getService(
                android::String16(IAVBuffer::serviceName().c_str())
            )
        );
}


// PlaneControlTest constructor, checkPlaneMappings and PlaneControlTest::run()
// are implemented in vts_planecontrol_test.cpp

class VTS_L3_PlaneControl : public UTCore
{
public:
    VTS_L3_PlaneControl() : UTCore() {};

    ~VTS_L3_PlaneControl() override = default;

    vector<tuple<IVideoSink::Id, sp<IVideoSink>, sp<VideoSinkControllerListener>, sp<VideoSinkEventListener>>> mVideoSinkList;
    vector<tuple<IVideoDecoder::Id, sp<IVideoDecoder>, sp<VideoDecoderControllerListener>, sp<VideoDecoderEventListener>, videodecoder::Capabilities*>> mVideoDecoderList;
    sp<IAVBuffer> mAVBufferManager;
    sp<ValidPlaneControlListener> mPlaneControlListener;
    bool aidlReturn = false;
    bool mPlaneListenerRegistered = false;
    bool mIsTunnelSupported = false;
    IAVBufferHelper* mAVbufferHelper;

    void SetUp() override
    {
        mAVBufferManager = vts_avbuffer_getService();
        mPlaneControlListener = new ValidPlaneControlListener();

        if (mAVBufferManager == nullptr) {
                UT_FAIL_FATAL("AVBuffer Service not available");
        }

        if (gVideoDecoderManager == nullptr) {
                UT_FAIL_FATAL("VideoDecoder Service not available");
        }

        if (gVideoSinkManager == nullptr) {
                UT_FAIL_FATAL("VideoSink Service not available");
        }

        if (gPlaneControlservice == nullptr) {
                UT_FAIL_FATAL("PlaneControl Service not available");
        }

        if (gPlaneControlTestConfiguration == nullptr) {
            UT_FAIL_FATAL("PlaneControl test configuration handle is null. Ensure vts_l3_PlaneControl_test_config.yaml is available from the test runtime working directory.");
        }

        vector<IVideoSink::Id> sinkIds;
        vector<IVideoDecoder::Id> videoDecoderIds;

        mAVbufferHelper = getAVBufferHelperInstance();

        if (mAVbufferHelper == nullptr) {
                UT_FAIL_FATAL("AVBuffer Helper not available");
        }

        /*Get the Video Sink IDs*/
        VIDEOSINK_MANAGER_GETVIDEOSINKIDS(sinkIds);

        if(getVideoDecoderService() != UT_STATUS_OK)
        {
            UT_LOG_ERROR("getVideoDecoderService failed %d",__LINE__);
        }
        else
        {
            /*Get the Video Decoder IDs*/
            VIDEODECODER_MANAGER_GETVIDEODECODERIDS(videoDecoderIds);

            vector<videodecoder::OperationalMode> supportedModes;
            Status operationalModeStatus =
                gVideoDecoderManager->getSupportedOperationalModes(&supportedModes);
            if (operationalModeStatus.isOk() &&
                operationalModeStatus.exceptionCode() == Status::EX_NONE)
            {
                mIsTunnelSupported = std::find(supportedModes.begin(), supportedModes.end(),
                    videodecoder::OperationalMode::TUNNELLED) != supportedModes.end();
            }
            else
            {
                UT_LOG_WARNING("Failed to query decoder operational modes. Tunnel tests will be skipped.");
            }

            android::ProcessState::self()->startThreadPool();

            for (const auto& _videoDecoderId : videoDecoderIds)
            {
                sp<IVideoDecoder> videoDecoder;

                videodecoder::Capabilities* videoDecoderCapabilities = new videodecoder::Capabilities();

                sp<VideoDecoderEventListener> videoDecoderEventListener = new VideoDecoderEventListener();

                sp<VideoDecoderControllerListener> videoDecoderControlListener = new VideoDecoderControllerListener();

                /*Get Video Decoder details*/
                VIDEODECODER_MANAGER_GETVIDEODECODER(_videoDecoderId, videoDecoder);

                /*Register the events */
                VIDEODECODER_REGISTER_EVENT_SUCCESS(videoDecoder, videoDecoderEventListener);

                /*Get PlaneCapabilities of Video Decoder */
                VIDEODECODER_GETCAPABILITIES(videoDecoder, videoDecoderCapabilities);

                mVideoDecoderList.push_back({_videoDecoderId, videoDecoder, videoDecoderControlListener, videoDecoderEventListener, videoDecoderCapabilities});
            }
        }

        for(const auto& _sinkId: sinkIds)
        {

            sp<IVideoSink> videoSink;

            sp<VideoSinkEventListener> videoSinkEventListener = new VideoSinkEventListener();

            sp<VideoSinkControllerListener> videoSinkControlListener = new VideoSinkControllerListener();

            videoSinkControlListener->setSinkID(_sinkId.value);
            videoSinkEventListener->setSinkID(_sinkId.value);

            /* Get Video Sink handle */
            VIDEOSINK_MANAGER_GETVIDEOSINK(_sinkId, videoSink);

            /* Register events */
            VIDEOSINK_REGISTER_EVENT_SUCCESS(videoSink, videoSinkEventListener);

            //TODO: VideoSink Get capabilities are yet to implement in the video sink

            mVideoSinkList.push_back({_sinkId, videoSink, videoSinkControlListener, videoSinkEventListener});
        }

        ::android::binder::Status planeListenerStatus =
            gPlaneControlservice->registerListener(mPlaneControlListener, &aidlReturn);
        if (!planeListenerStatus.isOk() || !aidlReturn)
        {
            bool unregisterResult = false;
            ::android::binder::Status unregisterStatus =
                gPlaneControlservice->unregisterListener(mPlaneControlListener, &unregisterResult);
            UT_LOG_WARNING("registerListener first-attempt failed (isOk=%d, exception=%d, result=%d); trying after unregister", planeListenerStatus.isOk(), planeListenerStatus.exceptionCode(), aidlReturn);
            UT_LOG_WARNING("cleanup unregister status (isOk=%d, exception=%d, result=%d)", unregisterStatus.isOk(), unregisterStatus.exceptionCode(), unregisterResult);
            planeListenerStatus = gPlaneControlservice->registerListener(mPlaneControlListener, &aidlReturn);
        }

        mPlaneListenerRegistered = planeListenerStatus.isOk() && (planeListenerStatus.exceptionCode() == Status::EX_NONE) && aidlReturn;
        UT_ASSERT_TRUE(planeListenerStatus.isOk()) << "registerListener binder call failed";
        UT_ASSERT_EQUAL(Status::EX_NONE, planeListenerStatus.exceptionCode()) << "registerListener returned an exception";
        UT_ASSERT_TRUE(aidlReturn) << "registerEventListener returned false";
    }

    bool isVideoSinkUsable(const IVideoSink::Id& sinkId)
    {
        return std::any_of(mVideoSinkList.begin(), mVideoSinkList.end(),
            [&sinkId](const auto& sinkEntry)
            {
                const auto& [id, sinkHandle, controlListener, eventListener] = sinkEntry;
                (void)controlListener;
                (void)eventListener;
                return id.value == sinkId.value && sinkHandle != nullptr;
            });
    }

    bool isVideoDecoderUsable(const IVideoDecoder::Id& decoderId)
    {
        return std::any_of(mVideoDecoderList.begin(), mVideoDecoderList.end(),
            [&decoderId](const auto& decoderEntry)
            {
                const auto& [id, decoderHandle, controlListener, eventListener, capabilities] = decoderEntry;
                (void)controlListener;
                (void)eventListener;
                (void)capabilities;
                return id.value == decoderId.value && decoderHandle != nullptr;
            });
    }

    void runPlaneControlTest(videoDecoderTestConfiguration_t testConfig,
                             videoDecoderTestConfiguration_t sinkConfig,
                             planeControlTestConfig_t planeConfig,
                             bool secureDecode, bool isTunnel)
    {
        try
        {
            for (size_t i = 0; i< planeConfig.planeMappings.size();i++)
            {
                IVideoSink::Id sinkId;
                sp<IVideoSink> sinkHandle;
                sp<VideoSinkControllerListener> sinkControlListener;
                sp<VideoSinkEventListener> sinkEventListener;
                IVideoDecoder::Id decoderId;
                sp<IVideoDecoder> decoderHandle;
                sp<VideoDecoderControllerListener> decoderControlListener;
                sp<VideoDecoderEventListener> decoderEventListener;
                std::unique_ptr<std::thread> sinkThread = nullptr;
                std::unique_ptr<std::thread> decoderThread = nullptr;
                std::unique_ptr<VideoSinkTest> sinkTest = nullptr;
                std::unique_ptr<videoDecoderTest> decodeTest = nullptr;
                bool foundDecoder = false;
                bool foundSink = false;
                bool decoderSupportsSecure = false;
                if (planeConfig.planeMappings[i].sourceType == ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK)
                {
                    for (const auto &[_sinkId, _sinkHandle, _controlListener, _eventListener] : mVideoSinkList)
                    {
                        if (planeConfig.planeMappings[i].sourceIndex != _sinkId.value)
                        {
                            UT_LOG_INFO("[SinkID: %d] is not as per configuration, skipping.", _sinkId.value);
                            continue;
                        }
                        if (isVideoSinkUsable(_sinkId))
                        {
                            sinkId = _sinkId;
                            sinkHandle = _sinkHandle;
                            sinkControlListener = _controlListener;
                            sinkEventListener = _eventListener;
                            foundSink = true;
                            UT_LOG_INFO("[SinkID: %d] is available", _sinkId.value);
                            break;
                        }
                    }
                    if (!foundSink)
                    {
                        UT_LOG_WARNING("No usable Sink found");
                        continue;
                    }
                    // Find a usable decoder

                    for (const auto &[_decoderId, _decoderHandle, _controlListener, _eventListener, _capabilities] : mVideoDecoderList)
                    {
                        if (planeConfig.decoderIds[i] != _decoderId.value)
                        {
                            UT_LOG_INFO("[DecoderID: %d] is not as per configuration, skipping.", _decoderId.value);
                            continue;
                        }
                        if (isVideoDecoderUsable(_decoderId))
                        {
                            decoderId = _decoderId;
                            decoderHandle = _decoderHandle;
                            decoderControlListener = _controlListener;
                            decoderEventListener = _eventListener;
                            decoderSupportsSecure =
                                _capabilities != nullptr && _capabilities->supportsSecure;
                            foundDecoder = true;
                            break;
                        }
                    }

                    if (!foundDecoder)
                    {
                        UT_LOG_WARNING("No usable decoder found");
                        continue;
                    }


                    IAVClock::Id clockId;
                    if (!gVideosinks_hfp.availableClocks_hfp.empty())
                    {
                        clockId.value = gVideosinks_hfp.availableClocks_hfp[0];
                    }
                    else
                    {
                        clockId.value = IAVClock::Id::UNDEFINED;
                    }

                    sinkTest = std::make_unique<VideoSinkTest>(sinkId, sinkHandle, sinkControlListener, sinkEventListener,
                                       decoderControlListener, mAVBufferManager, decoderId, clockId, sinkConfig, false);

                    bool isSinkAttached = (sinkHandle != nullptr);
                    const bool effectiveSecureDecode = secureDecode && decoderSupportsSecure;
                    const bool effectiveTunnel = isTunnel && mIsTunnelSupported;
                    UT_LOG_INFO("[DecoderID: %d] requested secure=%d tunnel=%d; using secure=%d tunnel=%d",
                                decoderId.value, secureDecode, isTunnel,
                                effectiveSecureDecode, effectiveTunnel);
                    decodeTest = std::make_unique<videoDecoderTest>(decoderId, decoderHandle, decoderControlListener,
                                           decoderEventListener, mAVBufferManager, effectiveSecureDecode,
                                           effectiveTunnel, testConfig, mAVbufferHelper, isSinkAttached);

                    decoderControlListener->resetQueues();
                    sinkThread = std::make_unique<std::thread>(&VideoSinkTest::run, sinkTest.get(), effectiveTunnel);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    decoderThread= std::make_unique<std::thread>(&videoDecoderTest::run, decodeTest.get(), true);

                    #if 0
                    const bool firstFrameQueued = sinkTest->waitForFirstFrameQueued(std::chrono::seconds(10));
                    if (!firstFrameQueued)
                    {
                        UT_FAIL("PlaneControl controls require a successful queueVideoFrame");
                        if (sinkThread->joinable())
                        {
                            sinkThread->join();
                        }
                        if (decoderThread->joinable())
                        {
                            decoderThread->join();
                        }
                        decodeTest->closeDecoderSession();
                        continue;
                    }
                    #endif

                }
                try
                {
                    PlaneControlTest planeControlTest(planeConfig);
                    planeControlTest.run();
                }
                catch (...)
                {
                    if ((sinkThread != nullptr) && sinkThread->joinable())
                    {
                        sinkThread->join();
                    }
                    if ((decoderThread != nullptr) && decoderThread->joinable())
                    {
                        decoderThread->join();
                    }
                    if (decodeTest != nullptr)
                    {
                        decodeTest->closeDecoderSession();
                    }
                    throw;
                }
                if ((sinkThread != nullptr) && sinkThread->joinable())
                {
                    sinkThread->join();
                }
                if ((decoderThread != nullptr) && decoderThread->joinable())
                {
                    decoderThread->join();
                }
                if (decodeTest != nullptr)
                {
                    decodeTest->closeDecoderSession();
                }
                if (foundSink)
                {
                    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> unmapMappings;
                    for (const auto& mapping : planeConfig.planeMappings)
                    {
                        if (mapping.sourceType ==
                            ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK)
                        {
                            auto unmap = mapping;
                            unmap.destinationPlaneIndex = -1;
                            unmapMappings.push_back(unmap);
                        }
                    }
                    if (!unmapMappings.empty())
                    {
                        bool unmapResult = false;
                        UT_LOG_INFO("Unmapping %zu VIDEO_SINK source(s) after stream %s",
                                    unmapMappings.size(), testConfig.streamName.c_str());
                        PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_POS(unmapMappings, unmapResult);
                    }
                }
            }
        }
        catch (const std::exception &ex)
        {
            UT_LOG_ERROR("Exception in runPlaneControlTest: %s", ex.what());
            UT_ASSERT_TRUE(false);
        }
        catch (...)
        {
            UT_LOG_ERROR("Unknown exception in runPlaneControlTest");
            UT_ASSERT_TRUE(false);
        }
    }

    void runSingleProcessTest(const char *testName, bool secureDecode = false, bool isTunnel = false)
    {
        vector<videoDecoderTestConfiguration_t> decoderConfigs;
        vector<planeControlTestConfig_t> planeConfigs;

        const bool graphicsOnlyTest = isGraphicsPlaneTestName(testName != nullptr ? testName : "");

        UT_status_t planeConfigStatus =
                vts_planecontrol_parse_testconfig(gPlaneControlTestConfiguration, PLANE_CONTROL_MGR, gTestGroup, testName, planeConfigs);

        if (planeConfigs.empty())
        {
            std::string errorMsg = std::string("No valid plane control test configurations found - missing test streams or invalid configuration for test: ") + testName;
            UT_FAIL_FATAL(errorMsg.c_str());
            return;
        }

        if (graphicsOnlyTest)
        {
            std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
            PLANE_CONTROL_GET_CAPABILITIES(capabilities);
            const bool hasGraphicsPlane = std::any_of(
                capabilities.begin(), capabilities.end(),
                [](const auto& capability)
                {
                    return capability.type ==
                           ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS;
                });
            if (!hasGraphicsPlane)
            {
                UT_IGNORE_TEST() << "No GRAPHICS plane is advertised in the capabilities";
                return;
            }
        }

        UT_LOG_INFO("vts_videodecoder_parse_testconfig for testName=%s, gTestGroup=%s, gVideoDecoderModuleName=%s",
                testName, gTestGroup, gVideoDecoderModuleName);
        vts_videodecoder_parse_testconfig(gPlaneControlTestConfiguration, PLANE_CONTROL_MGR,
                          gTestGroup, testName, decoderConfigs);

        UT_LOG_INFO("decoderConfigs.size()=%zu, planeConfigs.size()=%zu", decoderConfigs.size(), planeConfigs.size());
        if (decoderConfigs.empty())
        {
            UT_FAIL_FATAL("No decoder configurations found for Plane Control test");
            return;
        }
        if (std::strcmp(testName, "Planecontrol_Mapping") == 0)
        {
            std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
            PLANE_CONTROL_GET_CAPABILITIES(capabilities);
            planeConfigs.erase(
                std::remove_if(planeConfigs.begin(), planeConfigs.end(),
                    [&capabilities](const auto& planeConfig)
                    {
                        for (const auto& mapping : planeConfig.planeMappings)
                        {
                            const bool sourceSupported = std::any_of(
                                capabilities.begin(), capabilities.end(),
                                [&mapping](const auto& capability)
                                {
                                    return capability.type ==
                                               ::com::rdk::hal::planecontrol::PlaneType::VIDEO &&
                                           std::find(capability.sourceTypes.begin(),
                                                     capability.sourceTypes.end(),
                                                     mapping.sourceType) != capability.sourceTypes.end();
                                });
                            if (!sourceSupported)
                            {
                                UT_LOG_INFO("Skipping unsupported mapping sourceType=%d, sourceIndex=%d",
                                            static_cast<int>(mapping.sourceType), mapping.sourceIndex);
                                return true;
                            }
                        }
                        return false;
                    }),
                planeConfigs.end());
        }
        for (size_t i = 0; i < decoderConfigs.size(); ++i)
        {
            std::set<std::tuple<int32_t, int32_t, uint32_t>> streamVideoMappings;
            auto& decoderConfig = decoderConfigs[i];
            for (size_t j = 0; j < planeConfigs.size(); ++j)
            {
                auto& planeConfig = planeConfigs[j];
                if (std::strcmp(testName, "Planecontrol_Mapping") == 0 &&
                    planeConfig.planeMappings.size() == 1 &&
                    planeConfig.decoderIds.size() == 1 &&
                    planeConfig.planeMappings[0].sourceType ==
                        ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK)
                {
                    const auto& mapping = planeConfig.planeMappings[0];
                    const auto mappingKey = std::make_tuple(
                        static_cast<int32_t>(mapping.sourceType), mapping.sourceIndex,
                        planeConfig.decoderIds[0]);
                    if (!streamVideoMappings.insert(mappingKey).second)
                    {
                        UT_LOG_INFO("Skipping duplicate VIDEO_SINK mapping for sourceIndex=%d, decoderId=%u",
                                    mapping.sourceIndex, planeConfig.decoderIds[0]);
                        continue;
                    }
                }
                runPlaneControlTest(decoderConfig, decoderConfig, planeConfig, secureDecode, isTunnel);
            }
        }
    }

    void TearDown() override
    {
        for (const auto& [_sinkId, sink, videoSinkControlListener, videoSinkEventListener] : mVideoSinkList)
        {
            /* Un-register the events */
            VIDEOSINK_UNREGISTER_EVENT_SUCCESS(sink, videoSinkEventListener);
        }
        for (const auto& [_videoDecoderId, videoDecoder, videoDecoderControlListener, videoDecoderEventListener, videoDecoderCapabilities] : mVideoDecoderList)
        {
            VIDEODECODER_UNREGISTER_EVENT_SUCCESS(videoDecoder,videoDecoderEventListener);
        }
        if (mPlaneControlListener != nullptr)
        {
            bool unregisterResult = false;
            ::android::binder::Status status =
                gPlaneControlservice->unregisterListener(mPlaneControlListener, &unregisterResult);

            if (mPlaneListenerRegistered)
            {
                UT_ASSERT_TRUE(status.isOk()) << "unregisterEventListener binder call failed";
                UT_ASSERT_EQUAL(Status::EX_NONE, status.exceptionCode()) << "unregisterEventListener returned an exception";
                UT_ASSERT_TRUE(unregisterResult) << "unregisterEventListener returned false";
            }
            else
            {
                UT_LOG_INFO("Skipping strict unregister assertions because listener registration was not successful (isOk=%d, exception=%d, result=%d)",
                            status.isOk(), status.exceptionCode(), unregisterResult);
            }
        }
        mVideoSinkList.clear();
        mVideoDecoderList.clear();
        //TODO: check right place to call joinThreadPool
        //android::IPCThreadState::self()->joinThreadPool();

    }
};

/**
 * @brief Test the plane's ability to map video sources
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 001@n
 */
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_Mapping)
{
    const char *testName = "Planecontrol_Mapping";

    runSingleProcessTest(testName);
}

/**
 * @brief Test the plane's ability to swap video sources to plane
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 002@n
 */
UT_ADD_TEST(VTS_L3_PlaneControl,Planecontrol_Swapping)
{
    const char *testName = "Planecontrol_Swapping";

    runSingleProcessTest(testName);
}
/**
 * @brief Test the plane's ability to set correct Aspect Ratio
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 003@n
 */
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_Aspectratio)
{
    const char *testName = "Planecontrol_Aspectratio";

    runSingleProcessTest(testName);
}
/**
 * @brief Test the plane's ability to set Z Order
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 004@n
 */
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_Zorder)
{
    const char *testName = "Planecontrol_Zorder";

    runSingleProcessTest(testName);
}
/**
 * @brief Test the plane's ability to set visibility of the plane
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 005@n
 */
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_Visibility)
{
    const char *testName = "Planecontrol_Visibility";

    runSingleProcessTest(testName);
}
/**
 * @brief Test the plane's ability to set overscan
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 006@n
 */
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_Overscan)
{
    const char *testName = "Planecontrol_Overscan";

    runSingleProcessTest(testName);
}
/**
 * @brief Test the plane's ability to set Alpha transparency
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 007@n
 */
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_Alpha)
{
    const char *testName = "Planecontrol_Alpha";

    runSingleProcessTest(testName);
}
/**
 * @brief Test the controller's ability to set plane position
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 008@n
 */
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_Position)
{
    const char *testName = "Planecontrol_Position";

    runSingleProcessTest(testName);
}
/**
 * @brief Test the controller's ability to set plane size
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 009@n
 */
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_WindowSize)
{
    const char *testName = "Planecontrol_WindowSize";

    runSingleProcessTest(testName);
}
/**
 * @brief Test the controller's ability to set the Dynamic Range
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 010@n
 */
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_Dynamicrange)
{
    const char *testName = "Planecontrol_Dynamicrange";

    runSingleProcessTest(testName);
}
/**
 * @brief Test the Controller's ability to set Pixelformat
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 011@n
 */
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_Pixelformat)
{
    const char *testName = "Planecontrol_Pixelformat";

    runSingleProcessTest(testName);
}
/**
 * @brief Test the controller's ability to render graphics plane
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 012@n
 */
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_Graphicsplane_Client)
{
    const char *testName = kPlanecontrolGraphicsplaneClient;

    runSingleProcessTest(testName);
}
/**
 * @brief Test the controller's ability to report vsync delay
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 013@n
 */
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_Vsync)
{
    const char *testName = "Planecontrol_Vsync";

    runSingleProcessTest(testName);
}
/** @} */ // End of PLANE_CONTROL_HALTEST L3 File
/** @} */ // End of PLANE_CONTROL_HALTEST
/** @} */ // End of PLANE_CONTROL Module
/** @} */ // End of HPK
