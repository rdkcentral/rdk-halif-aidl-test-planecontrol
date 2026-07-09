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
#include <algorithm>
#include <condition_variable>
#include <mutex>
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

#define UT_LOG_WARN

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L3_PlaneControl, UT_TESTS_L3)

static const char *gModuleName = "IPlaneControl";
static const char *gVideoDecoderModuleName = "IVideoDecoderManager";
static const char *gTestGroup = "L3";
static IAVBufferHelper* gAVbufferHelper;
static const char *kPlanecontrolGraphicsplane = "Planecontrol_Graphicsplane";
static const char *kPlanecontrolGraphicsplaneAuto = "Planecontrol_Graphicsplane_Auto";
static const char *kPlanecontrolGraphicsplaneClient = "Planecontrol_Graphicsplane_Client";

static std::vector<int> videoPlaneIndex;
static std::vector<int> graphicsPlaneIndex;
static std::vector<com::rdk::hal::planecontrol::SourcePlaneMapping> planeMapping;

extern std::vector<int32_t> gAvailableVideosinks_hfp;
extern std::vector<int32_t> gAvailableVideodecoders_hfp;



class RecordingGraphicsFbProviderListenerL3 : public BnGraphicsFbProviderListener {
public:
    ::android::binder::Status onGraphicsFbReleased(int32_t oldGraphicsFbId, int64_t elapsedRealtimeNanos) override {
        std::lock_guard<std::mutex> lock(mMutex);
        mCallbackReceived = true;
        mOldGraphicsFbId = oldGraphicsFbId;
        mElapsedRealtimeNanos = elapsedRealtimeNanos;
        mCondition.notify_all();
        return ::android::binder::Status::ok();
    }

    bool waitForCallback(int32_t& oldGraphicsFbId, int64_t& elapsedRealtimeNanos, int timeoutMs = 1200) {
        std::unique_lock<std::mutex> lock(mMutex);
        const bool ready = mCondition.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() {
            return mCallbackReceived;
        });
        if (!ready) {
            return false;
        }

        oldGraphicsFbId = mOldGraphicsFbId;
        elapsedRealtimeNanos = mElapsedRealtimeNanos;
        return true;
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mMutex);
        mCallbackReceived = false;
        mOldGraphicsFbId = -1;
        mElapsedRealtimeNanos = -1;
    }

private:
    std::mutex mMutex;
    std::condition_variable mCondition;
    bool mCallbackReceived = false;
    int32_t mOldGraphicsFbId = -1;
    int64_t mElapsedRealtimeNanos = -1;
};

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
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    videoPlaneIndex.clear();
    graphicsPlaneIndex.clear();
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
    UT_ASSERT_EQUAL(retrievedMappings.size(), planeMapping.size());
    for (size_t i = 0; i < planeMapping.size(); ++i) {
        const auto& expected = planeMapping[i];
        const auto& actual = retrievedMappings[i];
        UT_ASSERT_EQUAL(expected.sourceIndex, actual.sourceIndex);
        UT_ASSERT_EQUAL(expected.destinationPlaneIndex, actual.destinationPlaneIndex);
    }
}


void PlaneControlTest::run ()
{
    if (isGraphicsPlaneTestName(mPlaneConfig.testName) && graphicsPlaneIndex.empty())
    {
        UT_LOG_INFO("Skipping %s because no GRAPHICS plane is advertised by the platform HFP/capabilities",
                    mPlaneConfig.testName.c_str());
        return;
    }

    bool aidl_return = false;
    if (!isGraphicsPlaneTestName(mPlaneConfig.testName))
    {
        PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_POS(mPlaneConfig.planeMappings, aidl_return);
        checkPlaneMappings(mPlaneConfig.planeMappings);
        std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
    }
    if (mPlaneConfig.testName == "Planecontrol_Swapping")
    {
        // Swap planes
        aidl_return = false;
        if (mPlaneConfig.planeMappings.size() >= 2)
        {
            for (size_t i = 0; i < mPlaneConfig.planeMappings.size()-1; i++)
            {
                SourcePlaneMapping temp = mPlaneConfig.planeMappings[i];
                mPlaneConfig.planeMappings[i] = mPlaneConfig.planeMappings[i+1];
                mPlaneConfig.planeMappings[i+1] = temp;
            }
        }
        PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_POS(mPlaneConfig.planeMappings, aidl_return);
        checkPlaneMappings(mPlaneConfig.planeMappings);
        std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
        char prompt[256];
        snprintf(prompt, sizeof(prompt),
                "Validate %s: Did the planes get swapped ?",
                    mPlaneConfig.testName.c_str());
        bool result = askUserYesNo(prompt);
        UT_ASSERT_TRUE(result);
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
                char prompt[256];
                snprintf(prompt, sizeof(prompt),
                            "Validate %s: did AspectRatio changed ?",
                            mPlaneConfig.testName.c_str());
                bool result = askUserYesNo(prompt);
                UT_ASSERT_TRUE(result);
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
                char prompt[256];
                snprintf(prompt, sizeof(prompt),
                            "Validate %s: did Zorder of the plane changed?",
                            mPlaneConfig.testName.c_str());
                bool result = askUserYesNo(prompt);
                UT_ASSERT_TRUE(result);
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
                char prompt[256];
                snprintf(prompt, sizeof(prompt),
                            "Validate %s: did plane visibility changed?",
                            mPlaneConfig.testName.c_str());
                bool result = askUserYesNo(prompt);
                UT_ASSERT_EQUAL(result,(bool)visibility);
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
            for(uint32_t overscan = 0; overscan <=100;overscan+=10)
            {
                propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(overscan);
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::OVERSCAN,propertyValue,aidl_return);
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
                char prompt[256];
                snprintf(prompt, sizeof(prompt),
                            "Validate %s: did plane overscan changed ?",
                            mPlaneConfig.testName.c_str());
                bool result = askUserYesNo(prompt);
                UT_ASSERT_TRUE(result);
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
            for(uint32_t alpha = 1; alpha <=256; alpha<<=1)
            {
                propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>((alpha-1));
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::ALPHA,propertyValue,aidl_return);
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
                char prompt[256];
                snprintf(prompt, sizeof(prompt),
                            "Validate %s: did plane alpha changed ?",
                            mPlaneConfig.testName.c_str());
                bool result = askUserYesNo(prompt);
                UT_ASSERT_TRUE(result);
            }
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(255);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::ALPHA,propertyValue,aidl_return);
        }
    }

    if (mPlaneConfig.testName == "Planecontrol_Dynamicrange")
    {
    //Dynamic Range
        char prompt[256];
        snprintf(prompt, sizeof(prompt),
                    "Validate %s: is plane displaying video properly ?",
                     mPlaneConfig.testName.c_str());
        bool result = askUserYesNo(prompt);
        UT_ASSERT_TRUE(result);
    // play different stream with dynamic range in test config.
    }
    if (mPlaneConfig.testName == "Planecontrol_Pixelformat")
    {
    //Resolution - pixel format
        char prompt[256];
        snprintf(prompt, sizeof(prompt),
                    "Validate %s: is plane displaying video properly ?",
                     mPlaneConfig.testName.c_str());
        bool result = askUserYesNo(prompt);
        UT_ASSERT_TRUE(result);
    // play different stream with different resolution in test config.
    }
    if (mPlaneConfig.testName == "Planecontrol_Position")
    {
        aidl_return = false;
        for (size_t i=0;i< mPlaneConfig.planeMappings.size();i++){
            ::com::rdk::hal::planecontrol::SourcePlaneMapping map = mPlaneConfig.planeMappings[i];
            if(i >= mPlaneConfig.positions.size())
            {
                continue;
            }
            std::pair position = mPlaneConfig.positions[i];
            ::com::rdk::hal::PropertyValue propertyValue;
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(position.first);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::X,propertyValue,aidl_return);
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(position.second);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::Y,propertyValue,aidl_return);
            std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
            char prompt[256];
            snprintf(prompt, sizeof(prompt),
                        "Validate %s: did plane position change ?",
                        mPlaneConfig.testName.c_str());
            bool result = askUserYesNo(prompt);
            UT_ASSERT_TRUE(result);
        }
    }
    //Geometry - positions

    if (mPlaneConfig.testName == "Planecontrol_Size")
    {
        aidl_return = false;
        for (size_t i=0;i< mPlaneConfig.planeMappings.size();i++){
            ::com::rdk::hal::planecontrol::SourcePlaneMapping map = mPlaneConfig.planeMappings[i];
            if(i>= mPlaneConfig.size.size())
            {
                continue;
            }
            std::pair size = mPlaneConfig.size[i];
            ::com::rdk::hal::PropertyValue propertyValue;
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(size.first);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::HEIGHT,propertyValue,aidl_return);
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(size.second);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,::com::rdk::hal::planecontrol::Property::WIDTH,propertyValue,aidl_return);
            std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
            char prompt[256];
            snprintf(prompt, sizeof(prompt),
                        "Validate %s: did plane Size change ?",
                        mPlaneConfig.testName.c_str());
            bool result = askUserYesNo(prompt);
            UT_ASSERT_TRUE(result);
        }
    //Geometry - Plane size
    }
    if (isGraphicsPlaneTestName(mPlaneConfig.testName))
    {
        const bool autoMode = isGraphicsPlaneAutoTest(mPlaneConfig.testName);
        const bool clientMode = isGraphicsPlaneClientTest(mPlaneConfig.testName);
        UT_ASSERT_TRUE(!graphicsPlaneIndex.empty()) << "No GRAPHICS plane found for " << mPlaneConfig.testName;

        for (const auto& index : graphicsPlaneIndex)
        {
            if (clientMode)
            {
                const GraphicsClientPattern clientPattern = getGraphicsClientPattern();
                PlaneControlGraphicsController graphicsClient;
                const bool connected = graphicsClient.connectPlaneControlProvider(index);
                UT_ASSERT_TRUE(connected) << "connectPlaneControlProvider failed for plane: " << index;

                PlaneControlGraphicsController::GraphicsFbClientBuffer fb1;
                PlaneControlGraphicsController::GraphicsFbClientBuffer fb2;
                UT_ASSERT_TRUE(graphicsClient.createGraphicsFbBuffer(1920, 1080, fb1))
                    << "createGraphicsFbBuffer FB1 failed for plane: " << index;
                UT_ASSERT_TRUE(graphicsClient.createGraphicsFbBuffer(1920, 1080, fb2))
                    << "createGraphicsFbBuffer FB2 failed for plane: " << index;
                UT_ASSERT_NOT_EQUAL(fb1.graphicsFbId, fb2.graphicsFbId)
                    << "Client buffers must have unique graphicsFbId for plane: " << index;

                const size_t mapLen1 = static_cast<size_t>(fb1.offset) +
                                       static_cast<size_t>(fb1.stride) * static_cast<size_t>(fb1.height);
                void* map1 = mmap(nullptr, mapLen1, PROT_READ | PROT_WRITE, MAP_SHARED, fb1.fd, 0);
                if (map1 != MAP_FAILED)
                {
                    drawClientPatternToDmaBuf(clientPattern,
                                              static_cast<uint8_t*>(map1),
                                              fb1.stride, fb1.width, fb1.height, fb1.offset,
                                              255, 165, 0);
                    munmap(map1, mapLen1);
                }

                const size_t mapLen2 = static_cast<size_t>(fb2.offset) +
                                       static_cast<size_t>(fb2.stride) * static_cast<size_t>(fb2.height);
                void* map2 = mmap(nullptr, mapLen2, PROT_READ | PROT_WRITE, MAP_SHARED, fb2.fd, 0);
                if (map2 != MAP_FAILED)
                {
                    drawClientPatternToDmaBuf(clientPattern,
                                              static_cast<uint8_t*>(map2),
                                              fb2.stride, fb2.width, fb2.height, fb2.offset,
                                              0, 255, 255);
                    munmap(map2, mapLen2);
                }

                const int32_t pairCount = 4;
                for (int32_t pair = 0; pair < pairCount; ++pair)
                {
                    auto& firstFb = (pair % 2 == 0) ? fb1 : fb2;
                    auto& secondFb = (pair % 2 == 0) ? fb2 : fb1;

                    bool accepted = false;
                    UT_ASSERT_TRUE(graphicsClient.commitGraphicsFbBuffer(firstFb.graphicsFbId, accepted));
                    UT_ASSERT_TRUE(accepted) << "commitGraphicsFbBuffer first failed for plane: " << index;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

                    accepted = false;
                    UT_ASSERT_TRUE(graphicsClient.commitGraphicsFbBuffer(secondFb.graphicsFbId, accepted));
                    UT_ASSERT_TRUE(accepted) << "commitGraphicsFbBuffer second failed for plane: " << index;

                    int32_t releasedId = -1;
                    int64_t elapsedNs = -1;
                    UT_ASSERT_TRUE(graphicsClient.waitForReleasedGraphicsFb(releasedId, elapsedNs))
                        << "waitForReleasedGraphicsFb failed for plane: " << index << " pair=" << pair;
                    if (pair == 0)
                    {
                        UT_ASSERT_TRUE(releasedId == -1 || releasedId == firstFb.graphicsFbId)
                            << "Unexpected released buffer id for plane: " << index;
                    }
                    else
                    {
                        UT_ASSERT_EQUAL(releasedId, firstFb.graphicsFbId)
                            << "Unexpected released buffer id for plane: " << index << " pair=" << pair;
                    }
                    UT_ASSERT_TRUE(elapsedNs >= 0) << "Invalid elapsedNs for plane: " << index;
                }

                if (!autoMode)
                {
                    std::vector<GraphicsClientPattern> interactivePatterns = {
                        GraphicsClientPattern::TRIANGLE,
                        GraphicsClientPattern::RECTANGLE,
                        GraphicsClientPattern::SOLID_YELLOW
                    };
                    if (hasGraphicsClientImagePath())
                    {
                        interactivePatterns.push_back(GraphicsClientPattern::IMAGE);
                    }

                    for (const auto patternToShow : interactivePatterns)
                    {
                        const size_t redrawLen = static_cast<size_t>(fb1.offset) +
                                                 static_cast<size_t>(fb1.stride) * static_cast<size_t>(fb1.height);
                        void* redrawMap = mmap(nullptr, redrawLen, PROT_READ | PROT_WRITE, MAP_SHARED, fb1.fd, 0);
                        if (redrawMap != MAP_FAILED)
                        {
                            drawClientPatternToDmaBuf(patternToShow,
                                                      static_cast<uint8_t*>(redrawMap),
                                                      fb1.stride, fb1.width, fb1.height, fb1.offset,
                                                      255, 165, 0);
                            munmap(redrawMap, redrawLen);
                        }

                        bool accepted = false;
                        UT_ASSERT_TRUE(graphicsClient.commitGraphicsFbBuffer(fb1.graphicsFbId, accepted));
                        UT_ASSERT_TRUE(accepted) << "commitGraphicsFbBuffer interactive pattern failed for plane: " << index;
                        std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));

                        char prompt[256];
                        snprintf(prompt, sizeof(prompt),
                                 "Validate %s (plane %d): Is the %s visible? (yes = next image)",
                                 mPlaneConfig.testName.c_str(), index,
                                 getGraphicsClientPatternLabel(patternToShow));
                        while (!askUserYesNo(prompt))
                        {
                            UT_LOG_INFO("Waiting for user confirmation. Keeping %s on plane %d...",
                                        getGraphicsClientPatternLabel(patternToShow), index);
                            std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
                        }
                    }
                }

                UT_ASSERT_TRUE(graphicsClient.destroyGraphicsFbBuffer(fb1.graphicsFbId))
                    << "destroyGraphicsFbBuffer FB1 failed for plane: " << index;
                UT_ASSERT_TRUE(graphicsClient.destroyGraphicsFbBuffer(fb2.graphicsFbId))
                    << "destroyGraphicsFbBuffer FB2 failed for plane: " << index;
                close(fb1.fd);
                close(fb2.fd);
                graphicsClient.disconnectPlaneControlProvider();
                continue;
            }

#if 0
            // -----------------------------------------------------------------------
            // Step 1: Get IGraphicsFbProvider and validate capabilities
            // -----------------------------------------------------------------------
            ::android::sp<RecordingGraphicsFbProviderListenerL3> listener = new RecordingGraphicsFbProviderListenerL3();
            ::android::sp<::com::rdk::hal::planecontrol::IGraphicsFbProvider> provider;
            PLANE_CONTROL_GET_GRAPHICS_FB_PROVIDER_WITH_LISTENER_POS(index, listener, provider);

            ::com::rdk::hal::planecontrol::GraphicsFbCapabilities fbCaps;
            PLANE_CONTROL_GRAPHICS_FB_PROVIDER_GET_CAPABILITIES_POS(provider, fbCaps, index);
            UT_ASSERT_TRUE(fbCaps.maxGraphicsFrameBuffers > 0)
                << "Invalid maxGraphicsFrameBuffers for plane index: " << index;
            UT_ASSERT_TRUE(fbCaps.maxGraphicsFbWidth > 0)
                << "Invalid maxGraphicsFbWidth for plane index: " << index;
            UT_ASSERT_TRUE(fbCaps.maxGraphicsFbHeight > 0)
                << "Invalid maxGraphicsFbHeight for plane index: " << index;
            UT_LOG_INFO("Graphics plane %d capabilities: maxFBs=%d maxW=%d maxH=%d format=0x%x modifier=0x%" PRIx64,
                        index, fbCaps.maxGraphicsFrameBuffers,
                        fbCaps.maxGraphicsFbWidth, fbCaps.maxGraphicsFbHeight,
                        fbCaps.format, (uint64_t)fbCaps.modifier);
            UT_ASSERT_TRUE(fbCaps.format != 0)
                << "Invalid graphics format for plane index: " << index;

            const int32_t fbWidth  = std::min<int32_t>(fbCaps.maxGraphicsFbWidth,  1920);
            const int32_t fbHeight = std::min<int32_t>(fbCaps.maxGraphicsFbHeight, 1080);

            // -----------------------------------------------------------------------
            // Step 2: Single-frame orange triangle via DMA-BUF software rendering
            //   createGraphicsFb → mmap → drawTriangleToDmaBuf → munmap → commitGraphicsFb
            // -----------------------------------------------------------------------
            if (!clientMode)
            {
                ::com::rdk::hal::planecontrol::GraphicsFbInfo fbInfo;
                ::android::os::ParcelFileDescriptor fd;
                PLANE_CONTROL_GRAPHICS_FB_PROVIDER_CREATE_FB_POS(provider, fbWidth, fbHeight, fbInfo, fd, index);
                UT_ASSERT_TRUE(fbInfo.graphicsFbId >= 0)
                    << "createGraphicsFb returned invalid id for plane index: " << index;
                UT_LOG_INFO("Created FB id=%d size=%dx%d stride=%d offset=%d for plane %d",
                            fbInfo.graphicsFbId, fbInfo.pixelWidth, fbInfo.pixelHeight,
                            fbInfo.stride, fbInfo.offset, index);

                const size_t mapLength = getGraphicsFbMapLength(fbInfo);
                void* mapped = mmap(nullptr, mapLength, PROT_READ | PROT_WRITE, MAP_SHARED, fd.get(), 0);
                if (mapped != MAP_FAILED)
                {
                    UT_LOG_INFO("Drawing orange triangle into DMA-BUF (plane %d)", index);
                    drawTriangleToDmaBuf(static_cast<uint8_t*>(mapped),
                                         fbInfo.stride, fbInfo.pixelWidth, fbInfo.pixelHeight,
                                         fbInfo.offset,
                                         255, 165, 0);  // RGB: orange
                    verifyTriangleSpotPixels(static_cast<uint8_t*>(mapped),
                                             fbInfo.stride, fbInfo.pixelWidth, fbInfo.pixelHeight,
                                             fbInfo.offset,
                                             255, 165, 0);
                    munmap(mapped, mapLength);
                }
                else
                {
                    UT_LOG_WARN("mmap failed for DMA-BUF on plane %d (errno=%d) — pixel draw skipped", index, errno);
                }

                bool commitResult = false;
                PLANE_CONTROL_GRAPHICS_FB_PROVIDER_COMMIT_FB_POS(provider, fbInfo.graphicsFbId, commitResult, index);
                UT_ASSERT_TRUE(commitResult)
                    << "commitGraphicsFb (orange triangle) returned false for plane: " << index;

                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));

                if (!autoMode)
                {
                    char prompt[256];
                    snprintf(prompt, sizeof(prompt),
                             "Validate %s (plane %d): Is an orange triangle visible on the graphics plane?",
                             mPlaneConfig.testName.c_str(), index);
                    bool result = askUserYesNo(prompt);
                    UT_ASSERT_TRUE(result) << "Orange triangle not confirmed visible on plane " << index;
                }

                PLANE_CONTROL_GRAPHICS_FB_PROVIDER_DESTROY_FB_POS(provider, fbInfo.graphicsFbId, index);
            }

            // -----------------------------------------------------------------------
            // Step 3: Double-buffered frame flip — orange FB1 → cyan FB2
            //   Validates onGraphicsFbReleased callback fires with correct oldGraphicsFbId
            // -----------------------------------------------------------------------
            {
                ::com::rdk::hal::planecontrol::GraphicsFbInfo fbInfo1, fbInfo2;
                ::android::os::ParcelFileDescriptor fd1, fd2;

                PLANE_CONTROL_GRAPHICS_FB_PROVIDER_CREATE_FB_POS(provider, fbWidth, fbHeight, fbInfo1, fd1, index);
                UT_ASSERT_TRUE(fbInfo1.graphicsFbId >= 0)
                    << "createGraphicsFb (FB1) invalid id for plane: " << index;

                PLANE_CONTROL_GRAPHICS_FB_PROVIDER_CREATE_FB_POS(provider, fbWidth, fbHeight, fbInfo2, fd2, index);
                UT_ASSERT_TRUE(fbInfo2.graphicsFbId >= 0)
                    << "createGraphicsFb (FB2) invalid id for plane: " << index;
                UT_ASSERT_NOT_EQUAL(fbInfo1.graphicsFbId, fbInfo2.graphicsFbId)
                    << "FB1 and FB2 must have unique IDs on plane: " << index;

                // Draw orange into FB1
                const size_t mapLen1 = getGraphicsFbMapLength(fbInfo1);
                void* map1 = mmap(nullptr, mapLen1, PROT_READ | PROT_WRITE, MAP_SHARED, fd1.get(), 0);
                if (map1 != MAP_FAILED)
                {
                    UT_LOG_INFO("Drawing orange triangle into FB1 (plane %d)", index);
                    drawTriangleToDmaBuf(static_cast<uint8_t*>(map1),
                                         fbInfo1.stride, fbInfo1.pixelWidth, fbInfo1.pixelHeight,
                                         fbInfo1.offset,
                                         255, 165, 0);  // RGB: orange
                    verifyTriangleSpotPixels(static_cast<uint8_t*>(map1),
                                             fbInfo1.stride, fbInfo1.pixelWidth, fbInfo1.pixelHeight,
                                             fbInfo1.offset,
                                             255, 165, 0);
                    munmap(map1, mapLen1);
                }
                else
                {
                    UT_LOG_WARN("mmap failed for FB1 on plane %d (errno=%d) — pixel draw skipped", index, errno);
                }

                // Draw cyan into FB2
                const size_t mapLen2 = getGraphicsFbMapLength(fbInfo2);
                void* map2 = mmap(nullptr, mapLen2, PROT_READ | PROT_WRITE, MAP_SHARED, fd2.get(), 0);
                if (map2 != MAP_FAILED)
                {
                    UT_LOG_INFO("Drawing cyan triangle into FB2 (plane %d)", index);
                    drawTriangleToDmaBuf(static_cast<uint8_t*>(map2),
                                         fbInfo2.stride, fbInfo2.pixelWidth, fbInfo2.pixelHeight,
                                         fbInfo2.offset,
                                         0, 255, 255);  // RGB: cyan
                    verifyTriangleSpotPixels(static_cast<uint8_t*>(map2),
                                             fbInfo2.stride, fbInfo2.pixelWidth, fbInfo2.pixelHeight,
                                             fbInfo2.offset,
                                             0, 255, 255);
                    munmap(map2, mapLen2);
                }
                else
                {
                    UT_LOG_WARN("mmap failed for FB2 on plane %d (errno=%d) — pixel draw skipped", index, errno);
                }

                // Commit sequence. Client mode repeats pairs to validate release callbacks over time.
                const int32_t pairCount = clientMode ? 4 : 1;
                for (int32_t pair = 0; pair < pairCount; ++pair)
                {
                    const int32_t firstId = (pair % 2 == 0) ? fbInfo1.graphicsFbId : fbInfo2.graphicsFbId;
                    const int32_t secondId = (pair % 2 == 0) ? fbInfo2.graphicsFbId : fbInfo1.graphicsFbId;

                    bool commitResult = false;
                    PLANE_CONTROL_GRAPHICS_FB_PROVIDER_COMMIT_FB_POS(provider, firstId, commitResult, index);
                    UT_ASSERT_TRUE(commitResult)
                        << "commitGraphicsFb (first frame) returned false for plane: " << index
                        << " pair=" << pair;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

                    listener->reset();
                    commitResult = false;
                    PLANE_CONTROL_GRAPHICS_FB_PROVIDER_COMMIT_FB_POS(provider, secondId, commitResult, index);
                    UT_ASSERT_TRUE(commitResult)
                        << "commitGraphicsFb (second frame) returned false for plane: " << index
                        << " pair=" << pair;

                    int32_t oldGraphicsFbId = -1;
                    int64_t elapsedRealtimeNanos = -1;
                    const bool callbackReceived = listener->waitForCallback(oldGraphicsFbId, elapsedRealtimeNanos);
                    UT_ASSERT_TRUE(callbackReceived)
                        << "onGraphicsFbReleased callback not received after double-buffer commit on plane: "
                        << index << " pair=" << pair;
                    if (pair == 0)
                    {
                        UT_ASSERT_TRUE(oldGraphicsFbId == -1 || oldGraphicsFbId == firstId)
                            << "Unexpected oldGraphicsFbId: expected -1 or " << firstId
                            << " got " << oldGraphicsFbId << " on plane: " << index;
                    }
                    else
                    {
                        UT_ASSERT_EQUAL(oldGraphicsFbId, firstId)
                            << "Unexpected released fb id on plane: " << index << " pair=" << pair;
                    }
                    UT_ASSERT_TRUE(elapsedRealtimeNanos >= 0)
                        << "Invalid elapsedRealtimeNanos in callback on plane: " << index;
                    UT_LOG_INFO("Double-buffer flip callback: oldGraphicsFbId=%d elapsedNs=%" PRId64 " (plane %d pair %d)",
                                oldGraphicsFbId, elapsedRealtimeNanos, index, pair);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));

                if (!autoMode && !clientMode)
                {
                    char prompt[256];
                    snprintf(prompt, sizeof(prompt),
                             "Validate %s (plane %d): Were both triangles displayed in sequence (orange then cyan)?",
                             mPlaneConfig.testName.c_str(), index);
                    bool result = askUserYesNo(prompt);
                    UT_ASSERT_TRUE(result) << "Double-buffer triangle flip not confirmed on plane " << index;
                }

                PLANE_CONTROL_GRAPHICS_FB_PROVIDER_DESTROY_FB_POS(provider, fbInfo1.graphicsFbId, index);
                PLANE_CONTROL_GRAPHICS_FB_PROVIDER_DESTROY_FB_POS(provider, fbInfo2.graphicsFbId, index);
            }

            // -----------------------------------------------------------------------
            // Step 4: Validate getNativeDisplayHandle and getEGLPlatformType API
            //   These are EGL compositor setup APIs; validated as "call returns OK".
            //   No EGL rendering is performed here — the DMA-BUF path above is the
            //   primary graphics plane test.
            // -----------------------------------------------------------------------
            if (!clientMode)
            {
                int64_t nativeDisplayHandle = 0;
                int32_t eglPlatformType = 0;
                ::android::binder::Status status = provider->getNativeDisplayHandle(&nativeDisplayHandle);
                UT_ASSERT_TRUE(status.isOk())
                    << "getNativeDisplayHandle failed for plane index: " << index;
                status = provider->getEGLPlatformType(&eglPlatformType);
                UT_ASSERT_TRUE(status.isOk())
                    << "getEGLPlatformType failed for plane index: " << index;
                UT_LOG_INFO("EGL info for plane %d: nativeDisplayHandle=%" PRId64 " eglPlatformType=0x%x",
                            index, nativeDisplayHandle, eglPlatformType);
            }
#endif
        }
    }

    if (mPlaneConfig.testName == "Planecontrol_Vsync")
    {
    //Vsync ToDo
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
    bool mPlaneListenerRegistered = false;
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
                                      decoderControlListener, decoderId, testConfig, sinkConfig, false);

                    sinkThread = std::make_unique<std::thread>(&SinkTest::run, &sinkTest, true);
                    bool isSinkAttached = (sinkHandle != nullptr);
                    videoDecoderTest decodeTest(decoderId, decoderHandle, decoderControlListener,
                                                decoderEventListener, mAVBufferManager, false, false, testConfig, mAVbufferHelper, isSinkAttached);
                    decoderThread= std::make_unique<std::thread>(&videoDecoderTest::run, &decodeTest, true);

                    std::this_thread::sleep_for(std::chrono::milliseconds(500));

                }
                PlaneControlTest *planeControlTest = new PlaneControlTest(planeConfig);
                planeControlTest->run();
                delete planeControlTest;
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
            UT_LOG_ERROR("Exception in runPlaneControlTest: %s", ex.what());
            UT_ASSERT_TRUE(false);
        }
        catch (...)
        {
            UT_LOG_ERROR("Unknown exception in runPlaneControlTest");
            UT_ASSERT_TRUE(false);
        }
    }

    void runSingleProcessTest(const char *testName)
    {
        vector<videoDecoderTestConfiguration_t> decoderConfigs;
        vector<videoTestConfiguration_t> sinkConfigs;
        vector<planeControlTestConfig_t> planeConfigs;
        const bool graphicsOnlyTest = isGraphicsPlaneTestName(testName != nullptr ? testName : "");

        UT_status_t planeConfigStatus =
            vts_planecontrol_parse_testconfig(gPlaneControlTestConfiguration, gModuleName, gTestGroup, testName, planeConfigs);
        if (planeConfigStatus != UT_STATUS_OK)
        {
            planeConfigs.clear();
            planeConfigStatus =
                vts_planecontrol_parse_testconfig(gPlaneControlTestConfiguration, PLANE_CONTROL_MGR, gTestGroup, testName, planeConfigs);
        }
        UT_ASSERT_EQUAL(UT_STATUS_OK, planeConfigStatus) << "Failed to parse plane control test config";
        if (planeConfigStatus != UT_STATUS_OK)
        {
            return;
        }

        if (graphicsOnlyTest)
        {
            if (gNoGraphicsTypePlanes == 0)
            {
                UT_LOG_INFO("Skipping %s because no GRAPHICS planes are advertised in the HFP/capabilities", testName);
                return;
            }
            for (auto& planeConfig : planeConfigs)
            {
                PlaneControlTest planeControlTest(planeConfig);
                planeControlTest.run();
            }
            return;
        }

        vts_videodecoder_parse_testconfig(gPlaneControlTestConfiguration, gVideoDecoderModuleName, gTestGroup, testName, decoderConfigs);

        vts_videosink_parse_testconfig(gPlaneControlTestConfiguration, VIDEODECODERMGR, gTestGroup, "Sink_QueueDepth", sinkConfigs);


        for (size_t i = 0; i < decoderConfigs.size(); ++i)
        {
            auto& decoderConfig = decoderConfigs[i];
            if(sinkConfigs.size() == 0)
            {
                UT_LOG_INFO("No Sink Configurations defined in the test configurations");
                break;
            }
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
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_Graphicsplane)
{
    const char *testName = kPlanecontrolGraphicsplane;

    runSingleProcessTest(testName);
}
/**
 * @brief Test the controller's ability to render graphics plane in non-interactive mode
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 014@n
 */
UT_ADD_TEST(VTS_L3_PlaneControl, Planecontrol_Graphicsplane_Auto)
{
    const char *testName = kPlanecontrolGraphicsplaneAuto;

    runSingleProcessTest(testName);
}
/**
 * @brief Test the controller's graphics plane client-style double-buffer flow
 *
 * **Test Group ID:** 003@n
 * **Test Case ID:** 015@n
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
