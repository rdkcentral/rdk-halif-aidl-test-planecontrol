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
#include <stdexcept>

using namespace std;

// Module-level plane index vectors, populated by PlaneControlTest constructor
// and consumed by PlaneControlTest::run().
std::vector<int> videoPlaneIndex;
std::vector<int> graphicsPlaneIndex;
std::vector<com::rdk::hal::planecontrol::SourcePlaneMapping> planeMapping;

static const planeControl_Cap* findPlaneProfile(int32_t planeIndex)
{
    for (const auto& cap : gPlaneControl_hfp)
    {
        if (cap.planeIndex == planeIndex)
        {
            return &cap;
        }
    }
    return nullptr;
}

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
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> videoCapabilities;
    videoPlaneIndex.clear();
    graphicsPlaneIndex.clear();
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);
    for (const auto& plane : capabilities)
    {
        if (plane.type == com::rdk::hal::planecontrol::PlaneType::VIDEO)
        {
            videoPlaneIndex.push_back(plane.planeIndex);
            videoCapabilities.push_back(plane);
        }
        if (plane.type == com::rdk::hal::planecontrol::PlaneType::GRAPHICS)
            graphicsPlaneIndex.push_back(plane.planeIndex);
    }

    if (!isGraphicsPlaneTestName(mPlaneConfig.testName))
    {
        if (mPlaneConfig.planeMappings.size() > videoCapabilities.size())
        {
            const std::string errorMessage =
                "Test requests " + std::to_string(mPlaneConfig.planeMappings.size()) +
                " mappings but only " + std::to_string(videoCapabilities.size()) +
                " VIDEO planes are advertised";
            throw std::runtime_error(errorMessage);
        }

        for (size_t mappingIndex = 0; mappingIndex < mPlaneConfig.planeMappings.size(); ++mappingIndex)
        {
            const auto& capability = videoCapabilities[mappingIndex];
            const auto& sourceTypes = capability.sourceTypes;
            const auto& configuredMapping = mPlaneConfig.planeMappings[mappingIndex];
            const bool sourceSupported = std::find(sourceTypes.begin(), sourceTypes.end(),
                                                   configuredMapping.sourceType) != sourceTypes.end();
            if (!sourceSupported)
            {
                const std::string errorMessage =
                    "VIDEO plane " + std::to_string(capability.planeIndex) +
                    " does not support configured source type " +
                    std::to_string(static_cast<int>(configuredMapping.sourceType));
                throw std::runtime_error(errorMessage);
            }

            mPlaneConfig.planeMappings[mappingIndex].destinationPlaneIndex = capability.planeIndex;
            UT_LOG_INFO("Using advertised VIDEO planeIndex=%d for mapping %zu",
                        capability.planeIndex, mappingIndex);
        }
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
    const bool skipPreMapping = isGraphicsPlaneClientTest(mPlaneConfig.testName);
    if (!skipPreMapping)
    {
        UT_ASSERT_TRUE(!mPlaneConfig.planeMappings.empty())
            << "Test requires at least one source-to-destination mapping: "
            << mPlaneConfig.testName;
        UT_LOG_INFO("Applying %zu source-to-destination plane mapping(s) for %s",
                    mPlaneConfig.planeMappings.size(), mPlaneConfig.testName.c_str());
        for (const auto& mapping : mPlaneConfig.planeMappings)
        {
            UT_LOG_INFO("SourcePlaneMapping: sourceType=%d, sourceIndex=%d, destinationPlaneIndex=%d",
                        static_cast<int>(mapping.sourceType), mapping.sourceIndex,
                        mapping.destinationPlaneIndex);
        }
        PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_POS(mPlaneConfig.planeMappings, aidl_return);
        checkPlaneMappings(mPlaneConfig.planeMappings);
        if (mPlaneConfig.testName != "Planecontrol_Mapping" &&
            mPlaneConfig.testName != "Planecontrol_Alpha")
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.delay_between_steps));
        }
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
        std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.delay_between_steps));
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
        UT_ASSERT_TRUE(!mPlaneConfig.aspectRatios.empty())
            << "Planecontrol_Aspectratio requires Inputs.aspectRatios";
        aidl_return = false;
        for (const auto& map : mPlaneConfig.planeMappings)
        {
            for (const auto& aspectRatio : mPlaneConfig.aspectRatios)
            {
                const char* aspectRatioName = "UNKNOWN";
                switch (aspectRatio)
                {
                case ::com::rdk::hal::planecontrol::AspectRatio::ZOOM_WITH_ASPECT:
                    aspectRatioName = "ZOOM_WITH_ASPECT";
                    break;
                case ::com::rdk::hal::planecontrol::AspectRatio::FULL_STRETCH:
                    aspectRatioName = "FULL_STRETCH";
                    break;
                case ::com::rdk::hal::planecontrol::AspectRatio::FULL_WITH_ASPECT:
                    aspectRatioName = "FULL_WITH_ASPECT";
                    break;
                }
                ::com::rdk::hal::PropertyValue propertyValue;
                propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                    ::com::rdk::hal::PropertyValue::Value::intValue>(
                    static_cast<uint32_t>(aspectRatio));
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                    ::com::rdk::hal::planecontrol::Property::ASPECT_RATIO, propertyValue, aidl_return);
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.delay_between_steps));
                char prompt[256];
                snprintf(prompt, sizeof(prompt),
                         "Validate %s: is %s displayed correctly?",
                         mPlaneConfig.testName.c_str(), aspectRatioName);
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
            const planeControl_Cap* cap = findPlaneProfile(map.destinationPlaneIndex);
            if (cap == nullptr || !cap->supportsZOrder)
            {
                UT_LOG_INFO("Skipping ZORDER for plane %d: supportsZOrder is false",
                            map.destinationPlaneIndex);
                GTEST_SKIP() << "Skipping ZORDER for plane " << map.destinationPlaneIndex << ": supportsZOrder is false";
                continue;
            }

            ::com::rdk::hal::PropertyValue propertyValue;
            for (uint32_t zorder = 0; zorder < mPlaneConfig.planeMappings.size(); zorder++)
            {
                propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                    ::com::rdk::hal::PropertyValue::Value::intValue>(zorder);
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                    ::com::rdk::hal::planecontrol::Property::ZORDER, propertyValue, aidl_return);
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.delay_between_steps));
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
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.delay_between_steps));
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
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.delay_between_steps));
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
        UT_ASSERT_TRUE(!graphicsPlaneIndex.empty())
            << "Planecontrol_Alpha requires an advertised GRAPHICS plane";
        if (graphicsPlaneIndex.empty())
        {
            return;
        }

        PlaneControlGraphicsController graphicsBackground;
        const int32_t graphicsPlane = graphicsPlaneIndex.front();
        const bool connected = graphicsBackground.connectPlaneControlProvider(graphicsPlane);
        UT_ASSERT_TRUE(connected)
            << "Failed to connect graphics framebuffer provider for plane " << graphicsPlane;
        if (!connected)
        {
            return;
        }

        PlaneControlGraphicsController::GraphicsFbClientBuffer backgroundBuffer;
        const bool backgroundReady = graphicsBackground.createAndCommitSolidColorBuffer(
            1280, 720, 0, 160, 255, backgroundBuffer);
        UT_ASSERT_TRUE(backgroundReady)
            << "Failed to create and commit 1280x720 graphics background for plane "
            << graphicsPlane;
        if (!backgroundReady)
        {
            return;
        }

        UT_LOG_INFO("Committed graphics background on planeIndex=%d, framebuffer=%dx%d",
                    graphicsPlane, backgroundBuffer.width, backgroundBuffer.height);

        const std::vector<uint32_t> alphaLevels = {0, 64, 128, 192, 255};
        for (size_t i = 0; i < mPlaneConfig.planeMappings.size(); ++i)
        {
            const auto& map = mPlaneConfig.planeMappings[i];
            if (i < mPlaneConfig.size.size())
            {
                const auto& [height, width] = mPlaneConfig.size[i];
                UT_LOG_INFO("Sizing Alpha video planeIndex=%d to height=%u, width=%u",
                            map.destinationPlaneIndex, height, width);
                ::com::rdk::hal::PropertyValue sizeValue;
                sizeValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                    ::com::rdk::hal::PropertyValue::Value::intValue>(height);
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                    ::com::rdk::hal::planecontrol::Property::HEIGHT, sizeValue, aidl_return);
                sizeValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                    ::com::rdk::hal::PropertyValue::Value::intValue>(width);
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                    ::com::rdk::hal::planecontrol::Property::WIDTH, sizeValue, aidl_return);
            }

            ::com::rdk::hal::PropertyValue propertyValue;
            const planeControl_Cap* cap = findPlaneProfile(map.destinationPlaneIndex);
            if (cap == nullptr || !cap->supportsAlpha)
            {
                UT_LOG_INFO("Skipping ALPHA for plane %d: supportsAlpha is false",
                            map.destinationPlaneIndex);
                GTEST_SKIP() << "Skipping ALPHA for plane " << map.destinationPlaneIndex << ": supportsAlpha is false";
                continue;
            }

            for (const uint32_t alpha : alphaLevels)
            {
                propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                    ::com::rdk::hal::PropertyValue::Value::intValue>(alpha);
                PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                    ::com::rdk::hal::planecontrol::Property::ALPHA, propertyValue, aidl_return);
                std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.delay_between_steps));
                char prompt[256];
                snprintf(prompt, sizeof(prompt),
                         "Validate %s: video plane alpha is %u over the graphics background. Is the blend correct?",
                         mPlaneConfig.testName.c_str(), alpha);
                bool result = askUserYesNo(prompt);
                UT_ASSERT_TRUE(result);
            }
        }

        UT_ASSERT_TRUE(graphicsBackground.destroyGraphicsFbBuffer(backgroundBuffer))
            << "Failed to destroy graphics background for plane " << graphicsPlane;
        graphicsBackground.disconnectPlaneControlProvider();
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
            std::pair<uint32_t, uint32_t> position = mPlaneConfig.positions[i];
            std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
            PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::X, position.first);
            PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::Y, position.second);
            PLANE_CONTROL_SET_PROPERTY_MULTI_ATOMIC(map.destinationPlaneIndex, propertyKVList);
            std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.delay_between_steps));
            char prompt[256];
            snprintf(prompt, sizeof(prompt),
                     "Validate %s: did plane position change ?",
                     mPlaneConfig.testName.c_str());
            bool result = askUserYesNo(prompt);
            UT_ASSERT_TRUE(result);
        }
    }
    // Size
    if (mPlaneConfig.testName == "Planecontrol_WindowSize")
    {
        aidl_return = false;
        UT_ASSERT_TRUE(!mPlaneConfig.planeMappings.empty())
            << "Planecontrol_WindowSize requires a mapped VIDEO plane";
        if (mPlaneConfig.planeMappings.empty())
        {
            return;
        }

        const auto& map = mPlaneConfig.planeMappings.front();
        for (const auto& size : mPlaneConfig.size)
        {
            ::com::rdk::hal::PropertyValue propertyValue;
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                ::com::rdk::hal::PropertyValue::Value::intValue>(size.first);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                ::com::rdk::hal::planecontrol::Property::HEIGHT, propertyValue, aidl_return);
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<
                ::com::rdk::hal::PropertyValue::Value::intValue>(size.second);
            PLANE_CONTROL_SET_PROPERTY_POS(map.destinationPlaneIndex,
                ::com::rdk::hal::planecontrol::Property::WIDTH, propertyValue, aidl_return);
            std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.delay_between_steps));
            char prompt[256];
            snprintf(prompt, sizeof(prompt),
                     "Validate %s: did plane Size change ?",
                     mPlaneConfig.testName.c_str());
            bool result = askUserYesNo(prompt);
            UT_LOG_INFO("WindowSize validation completed for planeIndex=%d: result=%s",
                        map.destinationPlaneIndex, result ? "yes" : "no");
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
                    std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.delay_between_steps));

                    char prompt[256];
                    snprintf(prompt, sizeof(prompt),
                             "Validate %s (plane %d): Is the %s visible? (yes = next image)",
                             mPlaneConfig.testName.c_str(), index,
                             getGraphicsClientPatternLabel(patternToShow));
                    while (!askUserYesNo(prompt))
                    {
                        UT_LOG_INFO("Waiting for user confirmation. Keeping %s on plane %d...",
                                    getGraphicsClientPatternLabel(patternToShow), index);
                        std::this_thread::sleep_for(std::chrono::milliseconds(mPlaneConfig.delay_between_steps));
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
