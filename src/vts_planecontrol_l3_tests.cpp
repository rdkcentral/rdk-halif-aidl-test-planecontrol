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
#include "vts_videosink_parse_testconfig.h"
#include "vts_planecontrol_parse_testconfig.h"
#include "vts_planecontrol_graphicscontroller.h"
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
static const char *gTestGroup = "L3";
static IAVBufferHelper* gAVbufferHelper;

static std::vector<int> videoPlaneIndex;
static std::vector<int> graphicsPlaneIndex;
static std::vector<com::rdk::hal::planecontrol::SourcePlaneMapping> planeMapping;

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
    PlaneControlTest(planeControlTestConfig_t planeConfig);
    void run ();


private:
    IVideoSink::Id mSinkId;
    planeControlTestConfig_t mPlaneConfig;

};

PlaneControlTest::PlaneControlTest(planeControlTestConfig_t planeConfig)
                        :mPlaneConfig(planeConfig)
{
    std::vector<::com::rdk::hal::planecontrol::Capabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);
    for (const auto& plane : capabilities) {
        if(plane.type == com::rdk::hal::planecontrol::PlaneType::VIDEO)
            videoPlaneIndex.push_back(plane.planeIndex);
        if(plane.type == com::rdk::hal::planecontrol::PlaneType::GRAPHICS)
            graphicsPlaneIndex.push_back(plane.planeIndex);
    }

}

static void checkPlaneMappings(vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> planeMapping)
{
    std::vector<SourcePlaneMapping> retrievedMappings;
    UT_LOG_INFO("Calling getVideoSourceDestinationPlaneMapping(OUT:retrievedMappings[])");
    auto status = gPlaneControlservice->getVideoSourceDestinationPlaneMapping(&retrievedMappings);
    UT_LOG_INFO("Result getVideoSourceDestinationPlaneMapping(OUT:retrievedMappings[], status:[isOK:%s, exception:%s])",
                UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    UT_ASSERT_TRUE(status.isOk());
    //ASSERT TO check the set and get mappings
}


void PlaneControlTest::run ()
{

    bool aidl_return = false;
    PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_POS(mPlaneConfig.planeMappings, aidl_return);
    checkPlaneMappings(mPlaneConfig.planeMappings);
    std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
    if (mPlaneConfig.testName == "Planecontrol_Swapping")
    {
        // Swap planes
        aidl_return = false;

        for (int i = 0; i < mPlaneConfig.planeMappings.size()-1; i++)
        {
            SourcePlaneMapping temp = mPlaneConfig.planeMappings[i];
            mPlaneConfig.planeMappings[i] = mPlaneConfig.planeMappings[i+1];
            mPlaneConfig.planeMappings[i+1] = temp;
        }
        PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_POS(mPlaneConfig.planeMappings, aidl_return);
        checkPlaneMappings(mPlaneConfig.planeMappings);
        std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
    }
    //Adjust Aspect Ratio
    if (mPlaneConfig.testName == "Planecontrol_Aspectratio")
    {
        std::vector<::com::rdk::hal::planecontrol::AspectRatio> aspectRatios = {
            ::com::rdk::hal::planecontrol::AspectRatio::FULL_WITH_ASPECT,
            ::com::rdk::hal::planecontrol::AspectRatio::FULL_STRETCH,
            ::com::rdk::hal::planecontrol::AspectRatio::ZOOM_WITH_ASPECT
        };
        aidl_return = false;
        for (const auto& map: mPlaneConfig.planeMappings){
            for(const auto& aspectRatio : aspectRatios)
            {
                ::com::rdk::hal::PropertyValue propertyValue;
                propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(static_cast<uint32_t>(aspectRatio));
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::ASPECT_RATIO,propertyValue,aidl_return);
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
            }
        }
    }
    //Change Z Order
    if (mPlaneConfig.testName == "Planecontrol_Zorder")
    {
        aidl_return = false;
        for (const auto& map: mPlaneConfig.planeMappings){
            ::com::rdk::hal::PropertyValue propertyValue;
            for(uint32_t zorder = 0; zorder < mPlaneConfig.planeMappings.size();zorder++)
            {
                propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(zorder);
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::ZORDER,propertyValue,aidl_return);
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
            }
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::ZORDER,propertyValue,aidl_return);
        }
    }
    if (mPlaneConfig.testName == "Planecontrol_Visibility")
    {
        //Visibility of the planes
        aidl_return = false;
        for (const auto& map: mPlaneConfig.planeMappings){
            ::com::rdk::hal::PropertyValue propertyValue;
            for(uint32_t visibility = 0; visibility < 2;visibility++)
            {
                propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(visibility);
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::VISIBLE,propertyValue,aidl_return);
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
            }
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::VISIBLE,propertyValue,aidl_return);
        }
    }
    //Overscan
    if (mPlaneConfig.testName == "Planecontrol_Overscan")
    {
        aidl_return = false;
        for (const auto& map: mPlaneConfig.planeMappings){
            ::com::rdk::hal::PropertyValue propertyValue;
            for(uint32_t overscan = 0; overscan < 100;overscan+=10)
            {
                propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(overscan);
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::OVERSCAN,propertyValue,aidl_return);
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
            }
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::OVERSCAN,propertyValue,aidl_return);
        }
    }
    //Alpha
    if (mPlaneConfig.testName == "Planecontrol_Alpha")
    {
        aidl_return = false;
        for (const auto& map: mPlaneConfig.planeMappings){
            ::com::rdk::hal::PropertyValue propertyValue;
            for(uint32_t alpha = 1; alpha < 256; alpha<<=1)
            {
                propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>((alpha-1));
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::ALPHA,propertyValue,aidl_return);
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
            }
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(255);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::ALPHA,propertyValue,aidl_return);
        }
    }

    if (mPlaneConfig.testName == "Planecontrol_Dynamicrange")
    {
    //Dynamic Range
    // play different stream with dynamic range in test config.
    }
    if (mPlaneConfig.testName == "Planecontrol_Pixelformat")
    {
    //Resolution - pixel format
    // play different stream with different resolution in test config.
    }
    if (mPlaneConfig.testName == "Planecontrol_Position")
    {
        aidl_return = false;
        for (int i=0;i< mPlaneConfig.planeMappings.size();i++){
            ::com::rdk::hal::planecontrol::SourcePlaneMapping map = mPlaneConfig.planeMappings[i];
            std::pair position = mPlaneConfig.positions[i];
            ::com::rdk::hal::PropertyValue propertyValue;
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(position.first);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::X,propertyValue,aidl_return);
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(position.second);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::Y,propertyValue,aidl_return);
        }
    }
    //Geometry - positions

    if (mPlaneConfig.testName == "Planecontrol_Size")
    {
        aidl_return = false;
        for (int i=0;i< mPlaneConfig.planeMappings.size();i++){
            ::com::rdk::hal::planecontrol::SourcePlaneMapping map = mPlaneConfig.planeMappings[i];
            std::pair size = mPlaneConfig.size[i];
            ::com::rdk::hal::PropertyValue propertyValue;
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(size.first);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::HEIGHT,propertyValue,aidl_return);
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(size.second);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::WIDTH,propertyValue,aidl_return);
        }
    //Geometry - Plane size
    }
    if (mPlaneConfig.testName == "Planecontrol_Graphicsplane")
    {
        //Graphics - graphics window Handle
        int64_t native_handle;
        PLANE_CONTROL_GET_NATIVE_GRAPHICS_WINDOW_POS(mPlaneConfig.graphicsPlaneIndex,native_handle);
        PlaneControlGraphicsController controller;
        controller.setWindowHandle((void*) native_handle);
        controller.startRender();
        std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
        controller.stopRender();
        PLANE_CONTROL_RELEASE_NATIVE_GRAPHICS_WINDOW_POS(mPlaneConfig.graphicsPlaneIndex,native_handle);
    }

    if (mPlaneConfig.testName == "Planecontrol_Vsync")
    {
    //Vsync
    }
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

    void runPlaneControlTest(videoDecoderTestConfiguration_t testConfig, videoTestConfiguration_t sinkConfig,planeControlTestConfig_t planeConfig)
    {
        try
        {
            for (int i = 0; i< planeConfig.planeMappings.size();i++)
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
                bool foundDecoder = false;
                bool foundSink = false;
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
                            foundDecoder = true;
                            break;
                        }
                    }

                    if (!foundDecoder)
                    {
                        UT_LOG_WARNING("No usable decoder found");
                        continue;
                    }


                    SinkTest sinkTest(sinkId, sinkHandle, sinkControlListener, sinkEventListener,
                                      decoderControlListener, testConfig, sinkConfig);

                    sinkThread = std::make_unique<std::thread>(&SinkTest::run, &sinkTest, true);
                    videoDecoderTest decodeTest(decoderId, decoderHandle, decoderControlListener,
                                                decoderEventListener, mAVBufferManager, false, false, testConfig, mAVbufferHelper);
                    decoderThread= std::make_unique<std::thread>(&videoDecoderTest::run, &decodeTest, true);

                    std::this_thread::sleep_for(std::chrono::milliseconds(500));

                }
                PlaneControlTest *planeControlTest = new PlaneControlTest(planeConfig);
                planeControlTest->run();
                if ((sinkThread != nullptr) && sinkThread->joinable())
                {
                    sinkThread->join();
                }

                if ((decoderThread != nullptr) && decoderThread->joinable())
                {
                    decoderThread->join();
                }
            }
        }
        catch (const std::exception &ex)
        {
            UT_LOG_ERROR("Exception in runSequentialSink: %s", ex.what());
            UT_ASSERT_TRUE(false);
        }
        catch (...)
        {
            UT_LOG_ERROR("Unknown exception in runSequentialSink");
            UT_ASSERT_TRUE(false);
        }
    }

    void runSingleProcessTest(const char *testName)
    {
        vector<videoDecoderTestConfiguration_t> decoderConfigs;
        vector<videoTestConfiguration_t> sinkConfigs;
        vector<planeControlTestConfig_t> planeConfigs;

        vts_planecontrol_parse_testconfig(gPlaneControlTestConfiguration, PLANE_CONTROL_MGR, gTestGroup, testName, planeConfigs);

        vts_videodecoder_parse_testconfig(gPlaneControlTestConfiguration, PLANE_CONTROL_MGR, gTestGroup, testName, decoderConfigs);

        vts_videosink_parse_testconfig(gPlaneControlTestConfiguration, VIDEODECODERMGR, gTestGroup, "Sink_QueueDepth", sinkConfigs);


        for (size_t i = 0; i < decoderConfigs.size(); ++i)
        {
            auto& decoderConfig = decoderConfigs[i];
            auto& sinkConfig = sinkConfigs[0];
            for (size_t j = 0; j < planeConfigs.size(); ++j)
            {
                auto& planeConfig = planeConfigs[j];
                runPlaneControlTest(decoderConfig, sinkConfig,planeConfig);
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
        PLANE_CONTROL_UNREGISTER_LISTENER_POS(mPlaneControlListener,aidlReturn);
        mVideoSinkList.clear();

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
 * @brief Test the sink's ability to report correct queue depth
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
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_Size)
{
    const char *testName = "Planecontrol_Size";

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
 * @brief Test the Controllers's ability to set Pixelformat
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
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_Graphicsplane)
{
    const char *testName = "Planecontrol_Graphicsplane";

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
