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
 * @defgroup Plane_Control_Halif_Test_TestClass Plane Control Test Class
 * @{
 * @parblock
 *
 * ### PlaneControlTest class implementation for Plane Control VTS HALIF AIDL tests :
 *
 * **Pre-Conditions:**  None@n
 * **Dependencies:** IPlaneControl service must be initialized via VTS_PlaneControl_init()@n
 *
 * @endparblock
 */

/**
 * @file vts_planecontrol_test.cpp
 */

#include "vts_planecontrol_test.h"
#include "vts_videosink_helper_functions.h"
#include "vts_videosink_parse_hfp.h"
#include "vts_videosink_test.h"
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

using namespace std;

// Module-level plane index vectors, populated by PlaneControlTest constructor
// and consumed by PlaneControlTest::run().
std::vector<int> videoPlaneIndex;
std::vector<int> graphicsPlaneIndex;
std::vector<com::rdk::hal::planecontrol::SourcePlaneMapping> planeMapping;

// ---------------------------------------------------------------------------
// checkPlaneMappings
// ---------------------------------------------------------------------------

void checkPlaneMappings(vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> expectedMapping)
{
    std::vector<SourcePlaneMapping> retrievedMappings;
    UT_LOG_INFO("Calling getVideoSourceDestinationPlaneMapping(OUT:retrievedMappings[])");
    auto status = gPlaneControlservice->getVideoSourceDestinationPlaneMapping(&retrievedMappings);
    UT_LOG_INFO("Result getVideoSourceDestinationPlaneMapping(OUT:retrievedMappings[], status:[isOK:%s, exception:%s])",
                UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_EQUAL(retrievedMappings.size(), expectedMapping.size());
    for (size_t i = 0; i < expectedMapping.size(); ++i)
    {
        const auto& expected = expectedMapping[i];
        const auto& actual   = retrievedMappings[i];
        UT_ASSERT_EQUAL(expected.sourceIndex,            actual.sourceIndex);
        UT_ASSERT_EQUAL(expected.destinationPlaneIndex,  actual.destinationPlaneIndex);
    }
}

// ---------------------------------------------------------------------------
// PlaneControlTest constructor
// ---------------------------------------------------------------------------

PlaneControlTest::PlaneControlTest(planeControlTestConfig_t planeConfig)
    : mPlaneConfig(planeConfig)
{
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    videoPlaneIndex.clear();
    graphicsPlaneIndex.clear();
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);
    for (const auto& plane : capabilities)
    {
        if (plane.type == com::rdk::hal::planecontrol::PlaneType::VIDEO)
            videoPlaneIndex.push_back(plane.planeIndex);
        if (plane.type == com::rdk::hal::planecontrol::PlaneType::GRAPHICS)
            graphicsPlaneIndex.push_back(plane.planeIndex);
    }
}

// ---------------------------------------------------------------------------
// PlaneControlTest::run
// ---------------------------------------------------------------------------

void PlaneControlTest::run()
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
            for (size_t i = 0; i < mPlaneConfig.planeMappings.size() - 1; i++)
            {
                SourcePlaneMapping temp = mPlaneConfig.planeMappings[i];
                mPlaneConfig.planeMappings[i]     = mPlaneConfig.planeMappings[i + 1];
                mPlaneConfig.planeMappings[i + 1] = temp;
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
    // Aspect Ratio
    if (mPlaneConfig.testName == "Planecontrol_Aspectratio")
    {
        std::vector<::com::rdk::hal::planecontrol::AspectRatio> aspectRatios = {
            ::com::rdk::hal::planecontrol::AspectRatio::FULL_WITH_ASPECT,
            ::com::rdk::hal::planecontrol::AspectRatio::FULL_STRETCH,
            ::com::rdk::hal::planecontrol::AspectRatio::ZOOM_WITH_ASPECT
        };
        aidl_return = false;
        for (const auto& map : mPlaneConfig.planeMappings)
        {
            for (const auto& aspectRatio : aspectRatios)
            {
                ::com::rdk::hal::PropertyValue propertyValue;
                propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                    ::com::rdk::hal::PropertyValue::Value::intValue>(
                    static_cast<uint32_t>(aspectRatio));
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                    ::com::rdk::hal::planecontrol::Property::ASPECT_RATIO, propertyValue, aidl_return);
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
    // Z Order
    if (mPlaneConfig.testName == "Planecontrol_Zorder")
    {
        aidl_return = false;
        for (const auto& map : mPlaneConfig.planeMappings)
        {
            ::com::rdk::hal::PropertyValue propertyValue;
            for (uint32_t zorder = 0; zorder < mPlaneConfig.planeMappings.size(); zorder++)
            {
                propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                    ::com::rdk::hal::PropertyValue::Value::intValue>(zorder);
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                    ::com::rdk::hal::planecontrol::Property::ZORDER, propertyValue, aidl_return);
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
                char prompt[256];
                snprintf(prompt, sizeof(prompt),
                         "Validate %s: did Zorder of the plane changed?",
                         mPlaneConfig.testName.c_str());
                bool result = askUserYesNo(prompt);
                UT_ASSERT_TRUE(result);
            }
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                ::com::rdk::hal::PropertyValue::Value::intValue>(0);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                ::com::rdk::hal::planecontrol::Property::ZORDER, propertyValue, aidl_return);
        }
    }
    // Visibility
    if (mPlaneConfig.testName == "Planecontrol_Visibility")
    {
        aidl_return = false;
        for (const auto& map : mPlaneConfig.planeMappings)
        {
            ::com::rdk::hal::PropertyValue propertyValue;
            for (uint32_t visibility = 0; visibility < 2; visibility++)
            {
                propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                    ::com::rdk::hal::PropertyValue::Value::intValue>(visibility);
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                    ::com::rdk::hal::planecontrol::Property::VISIBLE, propertyValue, aidl_return);
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
                char prompt[256];
                snprintf(prompt, sizeof(prompt),
                         "Validate %s: did plane visibility changed?",
                         mPlaneConfig.testName.c_str());
                bool result = askUserYesNo(prompt);
                UT_ASSERT_EQUAL(result, (bool)visibility);
            }
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                ::com::rdk::hal::PropertyValue::Value::intValue>(1);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                ::com::rdk::hal::planecontrol::Property::VISIBLE, propertyValue, aidl_return);
        }
    }
    // Overscan
    if (mPlaneConfig.testName == "Planecontrol_Overscan")
    {
        aidl_return = false;
        for (const auto& map : mPlaneConfig.planeMappings)
        {
            ::com::rdk::hal::PropertyValue propertyValue;
            for (uint32_t overscan = 0; overscan <= 100; overscan += 10)
            {
                propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                    ::com::rdk::hal::PropertyValue::Value::intValue>(overscan);
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                    ::com::rdk::hal::planecontrol::Property::OVERSCAN, propertyValue, aidl_return);
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
                char prompt[256];
                snprintf(prompt, sizeof(prompt),
                         "Validate %s: did plane overscan changed ?",
                         mPlaneConfig.testName.c_str());
                bool result = askUserYesNo(prompt);
                UT_ASSERT_TRUE(result);
            }
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                ::com::rdk::hal::PropertyValue::Value::intValue>(0);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                ::com::rdk::hal::planecontrol::Property::OVERSCAN, propertyValue, aidl_return);
        }
    }
    // Alpha
    if (mPlaneConfig.testName == "Planecontrol_Alpha")
    {
        aidl_return = false;
        for (const auto& map : mPlaneConfig.planeMappings)
        {
            ::com::rdk::hal::PropertyValue propertyValue;
            for (uint32_t alpha = 1; alpha <= 256; alpha <<= 1)
            {
                propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                    ::com::rdk::hal::PropertyValue::Value::intValue>((alpha - 1));
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                    ::com::rdk::hal::planecontrol::Property::ALPHA, propertyValue, aidl_return);
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
                char prompt[256];
                snprintf(prompt, sizeof(prompt),
                         "Validate %s: did plane alpha changed ?",
                         mPlaneConfig.testName.c_str());
                bool result = askUserYesNo(prompt);
                UT_ASSERT_TRUE(result);
            }
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                ::com::rdk::hal::PropertyValue::Value::intValue>(255);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                ::com::rdk::hal::planecontrol::Property::ALPHA, propertyValue, aidl_return);
        }
    }
    // Dynamic Range
    if (mPlaneConfig.testName == "Planecontrol_Dynamicrange")
    {
        char prompt[256];
        snprintf(prompt, sizeof(prompt),
                 "Validate %s: is plane displaying video properly ?",
                 mPlaneConfig.testName.c_str());
        bool result = askUserYesNo(prompt);
        UT_ASSERT_TRUE(result);
        // play different stream with dynamic range in test config.
    }
    // Pixel Format
    if (mPlaneConfig.testName == "Planecontrol_Pixelformat")
    {
        char prompt[256];
        snprintf(prompt, sizeof(prompt),
                 "Validate %s: is plane displaying video properly ?",
                 mPlaneConfig.testName.c_str());
        bool result = askUserYesNo(prompt);
        UT_ASSERT_TRUE(result);
        // play different stream with different resolution in test config.
    }
    // Position
    if (mPlaneConfig.testName == "Planecontrol_Position")
    {
        aidl_return = false;
        for (size_t i = 0; i < mPlaneConfig.planeMappings.size(); i++)
        {
            ::com::rdk::hal::planecontrol::SourcePlaneMapping map = mPlaneConfig.planeMappings[i];
            if (i >= mPlaneConfig.positions.size())
            {
                continue;
            }
            std::pair position = mPlaneConfig.positions[i];
            ::com::rdk::hal::PropertyValue propertyValue;
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                ::com::rdk::hal::PropertyValue::Value::intValue>(position.first);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                ::com::rdk::hal::planecontrol::Property::X, propertyValue, aidl_return);
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                ::com::rdk::hal::PropertyValue::Value::intValue>(position.second);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                ::com::rdk::hal::planecontrol::Property::Y, propertyValue, aidl_return);
            std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
            char prompt[256];
            snprintf(prompt, sizeof(prompt),
                     "Validate %s: did plane position change ?",
                     mPlaneConfig.testName.c_str());
            bool result = askUserYesNo(prompt);
            UT_ASSERT_TRUE(result);
        }
    }
    // Size
    if (mPlaneConfig.testName == "Planecontrol_Size")
    {
        aidl_return = false;
        for (size_t i = 0; i < mPlaneConfig.planeMappings.size(); i++)
        {
            ::com::rdk::hal::planecontrol::SourcePlaneMapping map = mPlaneConfig.planeMappings[i];
            if (i >= mPlaneConfig.size.size())
            {
                continue;
            }
            std::pair size = mPlaneConfig.size[i];
            ::com::rdk::hal::PropertyValue propertyValue;
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                ::com::rdk::hal::PropertyValue::Value::intValue>(size.first);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                ::com::rdk::hal::planecontrol::Property::HEIGHT, propertyValue, aidl_return);
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                ::com::rdk::hal::PropertyValue::Value::intValue>(size.second);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                ::com::rdk::hal::planecontrol::Property::WIDTH, propertyValue, aidl_return);
            std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));
            char prompt[256];
            snprintf(prompt, sizeof(prompt),
                     "Validate %s: did plane Size change ?",
                     mPlaneConfig.testName.c_str());
            bool result = askUserYesNo(prompt);
            UT_ASSERT_TRUE(result);
        }
    }
    // Graphics plane tests (Client and Auto modes)
    if (isGraphicsPlaneTestName(mPlaneConfig.testName))
    {
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
                    auto& firstFb  = (pair % 2 == 0) ? fb1 : fb2;
                    auto& secondFb = (pair % 2 == 0) ? fb2 : fb1;

                    bool accepted = false;
                    UT_ASSERT_TRUE(graphicsClient.commitGraphicsFbBuffer(firstFb.graphicsFbId, accepted));
                    UT_ASSERT_TRUE(accepted) << "commitGraphicsFbBuffer first failed for plane: " << index;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

                    for (;;)
                    {
                        int32_t tmpReleasedId  = -1;
                        int64_t tmpElapsedNs   = -1;
                        if (!graphicsClient.waitForReleasedGraphicsFb(tmpReleasedId, tmpElapsedNs, 0))
                        {
                            break;
                        }
                    }

                    accepted = false;
                    UT_ASSERT_TRUE(graphicsClient.commitGraphicsFbBuffer(secondFb.graphicsFbId, accepted));
                    UT_ASSERT_TRUE(accepted) << "commitGraphicsFbBuffer second failed for plane: " << index;

                    int32_t releasedId  = -1;
                    int64_t elapsedNs   = -1;
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
            ::android::sp<RecordingGraphicsFbProviderListener> listener = new RecordingGraphicsFbProviderListener();
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
            // -----------------------------------------------------------------------
            {
                ::com::rdk::hal::planecontrol::GraphicsFbInfo fbInfo;
                ::android::os::ParcelFileDescriptor fd;
                PLANE_CONTROL_GRAPHICS_FB_PROVIDER_CREATE_FB_POS(provider, fbWidth, fbHeight, fbInfo, fd, index);
                UT_ASSERT_TRUE(fbInfo.graphicsFbId >= 0)
                    << "createGraphicsFb returned invalid id for plane index: " << index;

                const size_t mapLength = getGraphicsFbMapLength(fbInfo);
                void* mapped = mmap(nullptr, mapLength, PROT_READ | PROT_WRITE, MAP_SHARED, fd.get(), 0);
                if (mapped != MAP_FAILED)
                {
                    drawTriangleToDmaBuf(static_cast<uint8_t*>(mapped),
                                         fbInfo.stride, fbInfo.pixelWidth, fbInfo.pixelHeight,
                                         fbInfo.offset, 255, 165, 0);
                    verifyTriangleSpotPixels(static_cast<uint8_t*>(mapped),
                                             fbInfo.stride, fbInfo.pixelWidth, fbInfo.pixelHeight,
                                             fbInfo.offset, 255, 165, 0);
                    munmap(mapped, mapLength);
                }

                bool commitResult = false;
                PLANE_CONTROL_GRAPHICS_FB_PROVIDER_COMMIT_FB_POS(provider, fbInfo.graphicsFbId, commitResult, index);
                UT_ASSERT_TRUE(commitResult)
                    << "commitGraphicsFb (orange triangle) returned false for plane: " << index;
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.stepDelay));

                if (!isGraphicsPlaneAutoTest(mPlaneConfig.testName))
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
            // Step 3: Double-buffered frame flip
            // -----------------------------------------------------------------------
            {
                ::com::rdk::hal::planecontrol::GraphicsFbInfo fbInfo1, fbInfo2;
                ::android::os::ParcelFileDescriptor fd1, fd2;

                PLANE_CONTROL_GRAPHICS_FB_PROVIDER_CREATE_FB_POS(provider, fbWidth, fbHeight, fbInfo1, fd1, index);
                PLANE_CONTROL_GRAPHICS_FB_PROVIDER_CREATE_FB_POS(provider, fbWidth, fbHeight, fbInfo2, fd2, index);
                UT_ASSERT_NOT_EQUAL(fbInfo1.graphicsFbId, fbInfo2.graphicsFbId)
                    << "FB1 and FB2 must have unique IDs on plane: " << index;

                const size_t mapLen1 = getGraphicsFbMapLength(fbInfo1);
                void* map1 = mmap(nullptr, mapLen1, PROT_READ | PROT_WRITE, MAP_SHARED, fd1.get(), 0);
                if (map1 != MAP_FAILED)
                {
                    drawTriangleToDmaBuf(static_cast<uint8_t*>(map1),
                                         fbInfo1.stride, fbInfo1.pixelWidth, fbInfo1.pixelHeight,
                                         fbInfo1.offset, 255, 165, 0);
                    verifyTriangleSpotPixels(static_cast<uint8_t*>(map1),
                                             fbInfo1.stride, fbInfo1.pixelWidth, fbInfo1.pixelHeight,
                                             fbInfo1.offset, 255, 165, 0);
                    munmap(map1, mapLen1);
                }

                const size_t mapLen2 = getGraphicsFbMapLength(fbInfo2);
                void* map2 = mmap(nullptr, mapLen2, PROT_READ | PROT_WRITE, MAP_SHARED, fd2.get(), 0);
                if (map2 != MAP_FAILED)
                {
                    drawTriangleToDmaBuf(static_cast<uint8_t*>(map2),
                                         fbInfo2.stride, fbInfo2.pixelWidth, fbInfo2.pixelHeight,
                                         fbInfo2.offset, 0, 255, 255);
                    verifyTriangleSpotPixels(static_cast<uint8_t*>(map2),
                                             fbInfo2.stride, fbInfo2.pixelWidth, fbInfo2.pixelHeight,
                                             fbInfo2.offset, 0, 255, 255);
                    munmap(map2, mapLen2);
                }

                const int32_t pairCount = 1;
                for (int32_t pair = 0; pair < pairCount; ++pair)
                {
                    const int32_t firstId  = (pair % 2 == 0) ? fbInfo1.graphicsFbId : fbInfo2.graphicsFbId;
                    const int32_t secondId = (pair % 2 == 0) ? fbInfo2.graphicsFbId : fbInfo1.graphicsFbId;

                    bool commitResult = false;
                    PLANE_CONTROL_GRAPHICS_FB_PROVIDER_COMMIT_FB_POS(provider, firstId, commitResult, index);
                    UT_ASSERT_TRUE(commitResult);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

                    listener->reset();
                    commitResult = false;
                    PLANE_CONTROL_GRAPHICS_FB_PROVIDER_COMMIT_FB_POS(provider, secondId, commitResult, index);
                    UT_ASSERT_TRUE(commitResult);

                    int32_t oldGraphicsFbId      = -1;
                    int64_t elapsedRealtimeNanos = -1;
                    const bool callbackReceived = listener->waitForCallback(oldGraphicsFbId, elapsedRealtimeNanos);
                    UT_ASSERT_TRUE(callbackReceived)
                        << "onGraphicsFbReleased callback not received on plane: " << index;
                    UT_ASSERT_TRUE(elapsedRealtimeNanos >= 0)
                        << "Invalid elapsedRealtimeNanos on plane: " << index;
                    UT_LOG_INFO("Double-buffer flip callback: oldFbId=%d elapsedNs=%" PRId64 " (plane %d pair %d)",
                                oldGraphicsFbId, elapsedRealtimeNanos, index, pair);
                }

                PLANE_CONTROL_GRAPHICS_FB_PROVIDER_DESTROY_FB_POS(provider, fbInfo1.graphicsFbId, index);
                PLANE_CONTROL_GRAPHICS_FB_PROVIDER_DESTROY_FB_POS(provider, fbInfo2.graphicsFbId, index);
            }

            // -----------------------------------------------------------------------
            // Step 4: getNativeDisplayHandle / getEGLPlatformType
            // -----------------------------------------------------------------------
            {
                int64_t nativeDisplayHandle = 0;
                int32_t eglPlatformType     = 0;
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
        } // for each graphicsPlaneIndex
    } // isGraphicsPlaneTestName

    if (mPlaneConfig.testName == "Planecontrol_Vsync")
    {
        // Vsync ToDo
    }
}

/** @} */ // End of Plane_Control_Halif_Test_TestClass
/** @} */ // End of Plane_Control_Halif_Test
/** @} */ // End of Plane_Control
