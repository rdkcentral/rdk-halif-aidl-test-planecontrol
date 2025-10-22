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
 * @addtogroup PLANE_CONTROL Video Sink Module
 * @{
 *
 */

/**
 * @defgroup PLANE_CONTROL_HALTEST Video Sink HAL Tests
 * @{
 *
 */

/**
 * @defgroup PLANE_CONTROL_HALTEST_L3 Video Sink HAL Tests L3 File
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
 * @file vts_planecontrol_l3_tests.c
 *
 */
#include "vts_videosink_helper_functions.h"
#include "vts_videosink_parse_hfp.h"
#include "vts_videosink_parse_testconfig.h"
#include "vts_videosink.h"
#include "vts_videodecoder.h"
#include "vts_planecontrol.h"

using namespace std::chrono;
using namespace std;

#define UT_LOG_WARN

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L3_PlaneControl, UT_TESTS_L3)

static const char *gModuleName = "IVideoSinkManager";
static const char *gVideoDecoderModuleName = "IVideoDecoderManager";
const char *gTestGroup = "L3";
IAVBufferHelper* gAVbufferHelper;

extern std::vector<int32_t> gAvailableVideosinks_hfp;
extern std::vector<int32_t> gAvailableVideodecoders_hfp;

static sp<IAVBuffer> vts_avbuffer_getService()
{
    return
        android::interface_cast<IAVBuffer>(
            android::defaultServiceManager()->getService(
                android::String16(IAVBuffer::serviceName().c_str())
            )
        );
}

class PlaneControlTest
{
public:
    PlaneControlTest();
    void run ();



private:
    IVideoSink::Id mSinkId;

};

PlaneControlTest::PlaneControlTest()
{
}

void PlaneControlTest::run ()
{
    std::vector<int> videoPlaneIndex;
    std::vector<int> graphicsPlaneIndex;
    std::vector<::com::rdk::hal::planecontrol::SourceType> allSourceTypes = {
        ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK,
        ::com::rdk::hal::planecontrol::SourceType::HDMI,
        ::com::rdk::hal::planecontrol::SourceType::COMPOSITE
    };
    std::vector<::com::rdk::hal::planecontrol::Capabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);
    for (const auto& plane : capabilities) {
        if(plane.type == com::rdk::hal::planecontrol::PlaneType::VIDEO)
            videoPlaneIndex.push_back(plane.planeIndex);
        if(plane.type == com::rdk::hal::planecontrol::PlaneType::GRAPHICS)
            graphicsPlaneIndex.push_back(plane.planeIndex);
    }

    std::vector<com::rdk::hal::planecontrol::SourcePlaneMapping> planeMapping;
    for(const auto& index: videoPlaneIndex)
    {
        PLANE_CONTROL_ADD_SOURCE_PLANE_MAPPING(planeMapping, ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK, 0, index);
    }
    bool aidl_return = false;
    PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_POS(planeMapping, aidl_return);
}

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

                /*Get Capabilities of Video Decoder */
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

        PLANE_CONTROL_REGISTER_LISTENER_POS(mPlaneControlListener,aidlReturn);
    }

    bool isVideoSinkUsable(const IVideoSink::Id& sinkId)
    {
        return std::find(gAvailableVideosinks_hfp.begin(), gAvailableVideosinks_hfp.end(), sinkId.value) != gAvailableVideosinks_hfp.end();
    }

    bool isVideoDecoderUsable(const IVideoDecoder::Id& decoderId)
    {
        return std::find(gAvailableVideodecoders_hfp.begin(),
                            gAvailableVideodecoders_hfp.end(),
                            decoderId.value) != gAvailableVideodecoders_hfp.end();
    }

    void runPlaneControlTest(videoDecoderTestConfiguration_t testConfig, videoTestConfiguration_t sinkConfig)
    {
        try {
            for (const auto& [_sinkId, sinkHandle, controlListener, eventListener] : mVideoSinkList)
            {
                if (!isVideoSinkUsable(_sinkId))
                {
                    UT_LOG_INFO("[SinkID: %d] is not usable, skipping.", _sinkId.value);
                    continue;
                }

                // Find a usable decoder
                IVideoDecoder::Id decoderId;
                sp<IVideoDecoder> decoderHandle;
                sp<VideoDecoderControllerListener> decoderControlListener;
                sp<VideoDecoderEventListener> decoderEventListener;
                bool foundDecoder = false;

                for (const auto& [_decoderId, _decoderHandle, _controlListener, _eventListener, _capabilities] : mVideoDecoderList)
                {
                    if (isVideoDecoderUsable(_decoderId))
                    {
                        decoderId = _decoderId;
                        decoderHandle = _decoderHandle;
                        decoderControlListener = _controlListener;
                        decoderEventListener = _eventListener;
                        foundDecoder = true;
                        break;
                    }
                }

                if (!foundDecoder)
                {
                    UT_LOG_WARNING("No usable decoder found for Sink ID %d, skipping.", _sinkId.value);
                    continue;
                }

                videoDecoderTest decodeTest(decoderId, decoderHandle, decoderControlListener,
                                        decoderEventListener, mAVBufferManager, false, false, testConfig, mAVbufferHelper);

                std::thread decodeThread(&videoDecoderTest::run, &decodeTest, true);

                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                SinkTest sinkTest(_sinkId, sinkHandle, controlListener, eventListener,
                                                decoderControlListener, testConfig, sinkConfig);

                std::thread sinkThread(&SinkTest::run,&sinkTest,true);

                PlaneControlTest *planeControlTest = new PlaneControlTest();

                planeControlTest->run();

                if (sinkThread.joinable())
                {
                    sinkThread.join();
                }

                if (decodeThread.joinable())
                {
                    decodeThread.join();
                }
            }
        } catch (const std::exception& ex) {
            UT_LOG_ERROR("Exception in runSequentialSink: %s", ex.what());
            UT_ASSERT_TRUE(false);
        } catch (...) {
            UT_LOG_ERROR("Unknown exception in runSequentialSink");
            UT_ASSERT_TRUE(false);
        }
    }

    void runSingleProcessTest(const char *testName)
    {
        vector<videoDecoderTestConfiguration_t> decoderConfigs;
        vector<videoTestConfiguration_t> sinkConfigs;

        vts_videodecoder_parse_testconfig(gVideoSinkTestConfiguration, VIDEODECODERMGR, gTestGroup, testName, decoderConfigs);

        vts_videosink_parse_testconfig(gVideoSinkTestConfiguration, VIDEODECODERMGR, gTestGroup, testName, sinkConfigs);

        for (size_t i = 0; i < decoderConfigs.size(); ++i)
        {
            auto& decoderConfig = decoderConfigs[i];
            auto& sinkConfig = sinkConfigs[i];

            runPlaneControlTest(decoderConfig, sinkConfig);
        }
    }

    void TearDown() override
    {
        for (const auto& [_sinkId, sink, videoSinkControlListener, videoSinkEventListener] : mVideoSinkList)
        {
            /* Un-register the events */
            VIDEOSINK_UNREGISTER_EVENT_SUCCESS(sink, videoSinkEventListener);
        }
        PLANE_CONTROL_UNREGISTER_LISTENER_POS(mPlaneControlListener,aidlReturn);
        mVideoSinkList.clear();

        //TODO: check right place to call joinThreadPool
        //android::IPCThreadState::self()->joinThreadPool();

    }
};

/**
 * @brief Test the sink's ability to report correct queue depth
 *        Type: Sequential
 *        Modes: Non-Tunnel, Non-Secure
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 001@n
 */
UT_ADD_TEST(VTS_L3_PlaneControl, PlaneControl_PlaneMapping)
{
    const char *testName = "Sink_QueueDepth";

    runSingleProcessTest(testName);
}

/** @} */ // End of VIDEO_SINK_HALTEST L3 File
/** @} */ // End of VIDEO_SINK_HALTEST
/** @} */ // End of VIDEO_SINK Module
/** @} */ // End of HPK
