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
 * @addtogroup HPK Hardware Porting Kit
 * @{
 * @par The Hardware Porting Kit
 * HPK is the next evolution of the well-defined Hardware Abstraction Layer
 * (HAL), but augmented with more comprehensive documentation and test suites
 * that OEM or SOC vendors can use to self-certify their ports before taking
 * them to RDKM for validation or to an operator for final integration and
 * deployment. The Hardware Porting Kit effectively enables an OEM and/or SOC
 * vendor to self-certify their own Video Accelerator devices, with minimal RDKM
 * assistance.
 *
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
 * @defgroup Plane_Control_Halif_Test_L2 Plane Control HALIF Tests L2 File
 * @{
 * @parblock
 *
 * ### L2 Tests for Plane Control HALIF AIDL :
 *
 * Level 1 test cases for all APIs of Plane Control HALIF AIDL Interface
 *
 * **Pre-Conditions:**  None@n
 * **Dependencies:** None@n
 *
 * Ref to API Definition specification documentation :
 *
 * @endparblock
 */


/**
 * @file vts_planecontrol_l2_tests.cpp
 *
 */

#include "vts_planecontrol.h"

/**
 * @file vts_planecontrol_l2_tests.cpp
 *
 */

 #include "vts_planecontrol.h"

class VTS_L2_PlaneControl : public virtual UTCore
{
protected:
    static std::vector<::com::rdk::hal::planecontrol::Capabilities> sCapabilities;
    static std::vector<uint32_t> sVideoPlaneIndices;
    static std::vector<::com::rdk::hal::planecontrol::SourceType> sAllSourceTypes;

    static void SetUpTestSuite() {
        UT_LOG_INFO("Setting up VTS_L2_PlaneControl test suite");

        // Initialize source types
        sAllSourceTypes = {
            ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK,
            ::com::rdk::hal::planecontrol::SourceType::HDMI,
            ::com::rdk::hal::planecontrol::SourceType::COMPOSITE
        };

        // Step 1: Get platform capabilities
        UT_LOG_INFO("Calling getCapabilities(OUT:capabilities[])");
        auto status = gPlaneControlservice->getCapabilities(&sCapabilities);
        UT_LOG_INFO("Result getCapabilities(OUT:capabilities[], status:[isOK:%s, exception:%s])",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        if (!status.isOk()) {
            UT_FAIL("Failed to get platform capabilities");
        }

        // Step 2: Populate video plane indices
        for (const auto& capability : sCapabilities) {
            if (capability.type == PlaneType::VIDEO) {
                sVideoPlaneIndices.push_back(capability.planeIndex);
            }
        }
        if (sVideoPlaneIndices.size() < 2) {
            UT_LOG_WARNING("Only %zu video planes found, some tests may be limited", sVideoPlaneIndices.size());
        }
    }

    static void TearDownTestSuite() {
        UT_LOG_INFO("Tearing down VTS_L2_PlaneControl test suite");

        // Step 1: Cleanup - Unmap all mappings
        std::vector<SourcePlaneMapping> cleanupMappings;
        for (const auto& sourceType : sAllSourceTypes) {
            for (int sourceIndex = 0; sourceIndex <= 1; ++sourceIndex) {
                SourcePlaneMapping unmap;
                unmap.sourceType = sourceType;
                unmap.sourceIndex = sourceIndex;
                unmap.destinationPlaneIndex = -1;
                cleanupMappings.push_back(unmap);
            }
        }
        UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping for final cleanup");
        bool mappingResult;
        auto status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(cleanupMappings, &mappingResult);
        UT_LOG_INFO("Result setVideoSourceDestinationPlaneMapping(cleanup, OUT:mappingResult:[%d], status:[isOK:%s, exception:%s])",
                    mappingResult,
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        if (!status.isOk()) {
            UT_LOG_ERROR("Failed to cleanup mappings");
        }
    }

    void SetUp() override {
        // Optional: Per-test setup if needed
    }

    void TearDown() override {
        // Optional: Per-test cleanup if needed
    }
};

// Static member initialization
std::vector<::com::rdk::hal::planecontrol::Capabilities> VTS_L2_PlaneControl::sCapabilities;
std::vector<uint32_t> VTS_L2_PlaneControl::sVideoPlaneIndices;
std::vector<::com::rdk::hal::planecontrol::SourceType> VTS_L2_PlaneControl::sAllSourceTypes;

/**
 * @brief Test the functionality of getting and setting properties for plane control
 *
 * **Test Group ID:** 02@n
 * **Test Case ID:** 001@n
 */
UT_ADD_TEST(VTS_L2_PlaneControl, TestGetSetProperty) {
    UT_LOG_INFO("Starting TestGetSetProperty");

    for (const auto& capability : sCapabilities) {
        int32_t planeIndex = capability.planeIndex;

        // Set and get WIDTH
        PropertyValue propertyValue;
        std::string frameWidthPath = "IPlaneControl.planes." + std::to_string(planeIndex) + ".frameWidth";
        uint32_t frame_width = UT_KVP_PROFILE_GET_UINT32(frameWidthPath.c_str());
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(frame_width);
        bool setResult;
        UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[WIDTH], IN:propertyValue:[%d], OUT:setResult)",
                    planeIndex, frame_width);
        auto status = gPlaneControlservice->setProperty(planeIndex, Property::WIDTH, propertyValue, &setResult);
        UT_LOG_INFO("Result setProperty(IN:planeIndex:[%d], IN:property:[WIDTH], IN:propertyValue:[%d], OUT:setResult:[%s], status:[isOK:%s, exception:%s])",
                    planeIndex, frame_width, UT_Control_GetMapString(bool_mapTable, setResult),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(setResult);

        std::optional<PropertyValue> getPropertyValue;
        UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[WIDTH], OUT:getPropertyValue)",
                    planeIndex);
        status = gPlaneControlservice->getProperty(planeIndex, Property::WIDTH, &getPropertyValue);
        UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[WIDTH], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])",
                    planeIndex, getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(getPropertyValue.has_value());
        UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), propertyValue.value);

        // Set and get HEIGHT
        std::string frameHeightPath = "IPlaneControl.planes." + std::to_string(planeIndex) + ".frameHeight";
        uint32_t frame_height = UT_KVP_PROFILE_GET_UINT32(frameHeightPath.c_str());
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(frame_height);
        UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[HEIGHT], IN:propertyValue:[%d], OUT:setResult)",
                    planeIndex, frame_height);
        status = gPlaneControlservice->setProperty(planeIndex, Property::HEIGHT, propertyValue, &setResult);
        UT_LOG_INFO("Result setProperty(IN:planeIndex:[%d], IN:property:[HEIGHT], IN:propertyValue:[%d], OUT:setResult:[%s], status:[isOK:%s, exception:%s])",
                    planeIndex, frame_height, UT_Control_GetMapString(bool_mapTable, setResult),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(setResult);

        UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[HEIGHT], OUT:getPropertyValue)",
                    planeIndex);
        status = gPlaneControlservice->getProperty(planeIndex, Property::HEIGHT, &getPropertyValue);
        UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[HEIGHT], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])",
                    planeIndex, getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(getPropertyValue.has_value());
        UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), propertyValue.value);

        // Set and get X
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
        UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[X], IN:propertyValue:[intValue:0])", planeIndex);
        status = gPlaneControlservice->setProperty(planeIndex, Property::X, propertyValue, &setResult);
        UT_LOG_INFO("Result setProperty(IN:planeIndex:[%d], IN:property:[X], IN:propertyValue:[intValue:0], status:[isOK:%s, exception:%s], setResult:[%s])",
                    planeIndex, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()), setResult ? "true" : "false");
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(setResult);

        UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[X], OUT:getPropertyValue)",
                    planeIndex);
        status = gPlaneControlservice->getProperty(planeIndex, Property::X, &getPropertyValue);
        UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[X], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])",
                    planeIndex, getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(getPropertyValue.has_value());
        UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), propertyValue.value);

        // Set and get Y
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
        UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[Y], IN:propertyValue:[intValue:0])", planeIndex);
        status = gPlaneControlservice->setProperty(planeIndex, Property::Y, propertyValue, &setResult);
        UT_LOG_INFO("Result setProperty(IN:planeIndex:[%d], IN:property:[Y], IN:propertyValue:[intValue:0], status:[isOK:%s, exception:%s], setResult:[%s])",
                    planeIndex, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()), setResult ? "true" : "false");
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(setResult);

        UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[Y], OUT:getPropertyValue)",
                    planeIndex);
        status = gPlaneControlservice->getProperty(planeIndex, Property::Y, &getPropertyValue);
        UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[Y], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])",
                    planeIndex, getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(getPropertyValue.has_value());
        UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), propertyValue.value);

        // Set and get ALPHA (if supported)
        std::string supportsAlphaPath = "IPlaneControl.planes." + std::to_string(planeIndex) + ".supportsAlpha";
        bool supportsAlpha = UT_KVP_PROFILE_GET_BOOL(supportsAlphaPath.c_str());
        if (supportsAlpha) {
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
            UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[ALPHA], IN:propertyValue:[intValue:0])", planeIndex);
            status = gPlaneControlservice->setProperty(planeIndex, Property::ALPHA, propertyValue, &setResult);
            UT_LOG_INFO("Result setProperty(IN:planeIndex:[%d], IN:property:[ALPHA], IN:propertyValue:[intValue:0], status:[isOK:%s, exception:%s], setResult:[%s])",
                        planeIndex, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()), setResult ? "true" : "false");
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(setResult);

            UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[ALPHA], OUT:getPropertyValue)",
                        planeIndex);
            status = gPlaneControlservice->getProperty(planeIndex, Property::ALPHA, &getPropertyValue);
            UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[ALPHA], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])",
                        planeIndex, getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(getPropertyValue.has_value());
            UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), propertyValue.value);
        }

        // Set and get ZORDER (if supported)
        std::string supportsZOrderPath = "IPlaneControl.planes." + std::to_string(planeIndex) + ".supportsZOrder";
        bool supportsZOrder = UT_KVP_PROFILE_GET_BOOL(supportsZOrderPath.c_str());
        if (supportsZOrder) {
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
            UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[ZORDER], IN:propertyValue:[intValue:1])", planeIndex);
            status = gPlaneControlservice->setProperty(planeIndex, Property::ZORDER, propertyValue, &setResult);
            UT_LOG_INFO("Result setProperty(IN:planeIndex:[%d], IN:property:[ZORDER], IN:propertyValue:[intValue:1], status:[isOK:%s, exception:%s], setResult:[%s])",
                        planeIndex, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()), setResult ? "true" : "false");
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(setResult);

            UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[ZORDER], OUT:getPropertyValue)",
                        planeIndex);
            status = gPlaneControlservice->getProperty(planeIndex, Property::ZORDER, &getPropertyValue);
            UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[ZORDER], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])",
                        planeIndex, getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(getPropertyValue.has_value());
            UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), propertyValue.value);
        }

        // Set and get VISIBLE
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
        UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[VISIBLE], IN:propertyValue:[intValue:1])", planeIndex);
        status = gPlaneControlservice->setProperty(planeIndex, Property::VISIBLE, propertyValue, &setResult);
        UT_LOG_INFO("Result setProperty(IN:planeIndex:[%d], IN:property:[VISIBLE], IN:propertyValue:[intValue:1], status:[isOK:%s, exception:%s], setResult:[%s])",
                    planeIndex, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()), setResult ? "true" : "false");
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(setResult);

        UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[VISIBLE], OUT:getPropertyValue)",
                    planeIndex);
        status = gPlaneControlservice->getProperty(planeIndex, Property::VISIBLE, &getPropertyValue);
        UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[VISIBLE], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])",
                    planeIndex, getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(getPropertyValue.has_value());
        UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), propertyValue.value);

        // Set and get video plane properties (if VIDEO plane)
        std::string planeTypePath = "IPlaneControl.planes." + std::to_string(planeIndex) + ".type";
        char planeTypeString[256];
        UT_KVP_PROFILE_GET_STRING(planeTypePath.c_str(), planeTypeString);
        if (std::string(planeTypeString) == "VIDEO") {
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
            UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[OVERSCAN], IN:propertyValue:[intValue:0])", planeIndex);
            status = gPlaneControlservice->setProperty(planeIndex, Property::OVERSCAN, propertyValue, &setResult);
            UT_LOG_INFO("Result setProperty(IN:planeIndex:[%d], IN:property:[OVERSCAN], IN:propertyValue:[intValue:0], status:[isOK:%s, exception:%s], setResult:[%s])",
                        planeIndex, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()), setResult ? "true" : "false");
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(setResult);

            UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[OVERSCAN], OUT:getPropertyValue)",
                        planeIndex);
            status = gPlaneControlservice->getProperty(planeIndex, Property::OVERSCAN, &getPropertyValue);
            UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[OVERSCAN], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])",
                        planeIndex, getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(getPropertyValue.has_value());
            UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), propertyValue.value);

            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
            UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[ASPECT_RATIO], IN:propertyValue:[intValue:1])", planeIndex);
            status = gPlaneControlservice->setProperty(planeIndex, Property::ASPECT_RATIO, propertyValue, &setResult);
            UT_LOG_INFO("Result setProperty(IN:planeIndex:[%d], IN:property:[ASPECT_RATIO], IN:propertyValue:[intValue:1], status:[isOK:%s, exception:%s], setResult:[%s])",
                        planeIndex, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()), setResult ? "true" : "false");
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(setResult);

            UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[ASPECT_RATIO], OUT:getPropertyValue)",
                        planeIndex);
            status = gPlaneControlservice->getProperty(planeIndex, Property::ASPECT_RATIO, &getPropertyValue);
            UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[ASPECT_RATIO], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])",
                        planeIndex, getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(getPropertyValue.has_value());
            UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), propertyValue.value);
        }
    }
}

/**
 * @brief Test the functionality of setting and getting multiple properties atomically
 *
 * **Test Group ID:** 02@n
 * **Test Case ID:** 002@n
 */
UT_ADD_TEST(VTS_L2_PlaneControl, TestGetSetPropertyMultiAtomic) {
    for (const auto& capability : sCapabilities) {
        int32_t planeIndex = capability.planeIndex;

        // Prepare Key value pair of the available properties
        std::vector<PropertyKVPair> propertyKVList;
        PropertyKVPair kvPair;

        kvPair.property = Property::WIDTH;
        std::string frameWidthPath = "IPlaneControl.planes." + std::to_string(planeIndex) + ".frameWidth";
        uint32_t frame_width = UT_KVP_PROFILE_GET_UINT32(frameWidthPath.c_str());
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(frame_width);
        propertyKVList.push_back(kvPair);

        kvPair.property = Property::HEIGHT;
        std::string frameHeightPath = "IPlaneControl.planes." + std::to_string(planeIndex) + ".frameHeight";
        uint32_t frame_height = UT_KVP_PROFILE_GET_UINT32(frameHeightPath.c_str());
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(frame_height);
        propertyKVList.push_back(kvPair);

        kvPair.property = Property::X;
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
        propertyKVList.push_back(kvPair);

        kvPair.property = Property::Y;
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
        propertyKVList.push_back(kvPair);

        std::string supportsAlphaPath = "IPlaneControl.planes." + std::to_string(planeIndex) + ".supportsAlpha";
        bool supportsAlpha = UT_KVP_PROFILE_GET_BOOL(supportsAlphaPath.c_str());
        if (supportsAlpha) {
            kvPair.property = Property::ALPHA;
            kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
            propertyKVList.push_back(kvPair);
        }

        std::string supportsZOrderPath = "IPlaneControl.planes." + std::to_string(planeIndex) + ".supportsZOrder";
        bool supportsZOrder = UT_KVP_PROFILE_GET_BOOL(supportsZOrderPath.c_str());
        if (supportsZOrder) {
            kvPair.property = Property::ZORDER;
            kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
            propertyKVList.push_back(kvPair);
        }

        kvPair.property = Property::VISIBLE;
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
        propertyKVList.push_back(kvPair);

        std::string planeTypePath = "IPlaneControl.planes." + std::to_string(planeIndex) + ".type";
        char planeTypeString[256];
        UT_KVP_PROFILE_GET_STRING(planeTypePath.c_str(), planeTypeString);
        if (std::string(planeTypeString) == "VIDEO") {
            kvPair.property = Property::OVERSCAN;
            kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
            propertyKVList.push_back(kvPair);

            kvPair.property = Property::ASPECT_RATIO;
            kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
            propertyKVList.push_back(kvPair);
        }

        // Use setPropertyMultiAtomic method
        bool setResult;
        UT_LOG_INFO("Calling setPropertyMultiAtomic(IN:planeIndex:[%d], IN:propertyKVList[])", planeIndex);
        auto status = gPlaneControlservice->setPropertyMultiAtomic(planeIndex, propertyKVList, &setResult);
        UT_LOG_INFO("Result setPropertyMultiAtomic(IN:planeIndex:[%d], IN:propertyKVList[], OUT:setResult:[%d], status:[isOK:%s, exception:%s])",
                    planeIndex, setResult,
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(setResult);

        // Get the property value using getPropertyMulti
        std::vector<PropertyKVPair> getPropertyKVList;
        for (const auto& kv : propertyKVList) {
            PropertyKVPair kvGet;
            kvGet.property = kv.property;
            getPropertyKVList.push_back(kvGet);
        }

        bool getResult;
        UT_LOG_INFO("Calling getPropertyMulti(IN:planeIndex:[%d], INOUT:propertyKVList[])", planeIndex);
        status = gPlaneControlservice->getPropertyMulti(planeIndex, &getPropertyKVList, &getResult);
        UT_LOG_INFO("Result getPropertyMulti(IN:planeIndex:[%d], INOUT:propertyKVList[], OUT:getResult:[%d], status:[isOK:%s, exception:%s])",
                    planeIndex, getResult,
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(getResult);

        for (const auto& kv : getPropertyKVList) {
            if (kv.property == Property::WIDTH && kv.propertyValue.value.has_value()) {
                int32_t width = kv.propertyValue.value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>();
                UT_ASSERT_EQUAL(width, frame_width);
            } else if (kv.property == Property::HEIGHT && kv.propertyValue.value.has_value()) {
                int32_t height = kv.propertyValue.value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>();
                UT_ASSERT_EQUAL(height, frame_height);
            } else if (kv.property == Property::X && kv.propertyValue.value.has_value()) {
                int32_t val = kv.propertyValue.value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>();
                UT_ASSERT_EQUAL(val, 0);
            } else if (kv.property == Property::Y && kv.propertyValue.value.has_value()) {
                int32_t val = kv.propertyValue.value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>();
                UT_ASSERT_EQUAL(val, 0);
            } else if (kv.property == Property::ALPHA && kv.propertyValue.value.has_value()) {
                int32_t val = kv.propertyValue.value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>();
                UT_ASSERT_EQUAL(val, 0);
            } else if (kv.property == Property::ZORDER && kv.propertyValue.value.has_value()) {
                int32_t val = kv.propertyValue.value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>();
                UT_ASSERT_EQUAL(val, 1);
            } else if (kv.property == Property::VISIBLE && kv.propertyValue.value.has_value()) {
                int32_t val = kv.propertyValue.value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>();
                UT_ASSERT_EQUAL(val, 1);
            } else if (kv.property == Property::OVERSCAN && kv.propertyValue.value.has_value()) {
                int32_t val = kv.propertyValue.value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>();
                UT_ASSERT_EQUAL(val, 0);
            } else if (kv.property == Property::ASPECT_RATIO && kv.propertyValue.value.has_value()) {
                int32_t val = kv.propertyValue.value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>();
                UT_ASSERT_EQUAL(val, 1);
            }
        }
    }
}

/**
 * @brief Test the functionality of setting and retrieving video source to destination plane mappings
 *
 * **Test Group ID:** 02@n
 * **Test Case ID:** 003@n
 */
UT_ADD_TEST(VTS_L2_PlaneControl, TestGetSetVideoSourceDestinationPlaneMapping) {
    UT_ASSERT_GREATER_EQUAL(sVideoPlaneIndices.size(), 2) << "Need at least 2 video planes, found " << sVideoPlaneIndices.size();

    std::vector<SourcePlaneMapping> sourcePlaneMappings;
    std::vector<SourcePlaneMapping> retrievedMappings;
    bool mappingResult;

    for (const auto& source : sAllSourceTypes) {
        // Unmap all existing mappings for target planes
        std::vector<SourcePlaneMapping> unmapMappings;
        for (const auto& unmapSource : sAllSourceTypes) {
            for (int sourceIndex = 0; sourceIndex <= 1; ++sourceIndex) {
                SourcePlaneMapping unmap;
                unmap.sourceType = unmapSource;
                unmap.sourceIndex = sourceIndex;
                unmap.destinationPlaneIndex = -1;
                unmapMappings.push_back(unmap);
            }
        }
        UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping to unmap all sources for planes:[%d,%d]",
                    sVideoPlaneIndices[0], sVideoPlaneIndices[1]);
        auto status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(unmapMappings, &mappingResult);
        UT_LOG_INFO("Result setVideoSourceDestinationPlaneMapping(unmap, OUT:mappingResult:[%d], status:[isOK:%s, exception:%s])",
                    mappingResult,
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk()) << "Failed to unmap all sources for sourceType " << static_cast<int>(source);
        UT_ASSERT_TRUE(mappingResult) << "Unmapping failed for sourceType " << static_cast<int>(source);

        // Set new mappings
        sourcePlaneMappings.clear();
        SourcePlaneMapping mapdata_1;
        mapdata_1.sourceType = source;
        mapdata_1.sourceIndex = 0;
        mapdata_1.destinationPlaneIndex = sVideoPlaneIndices[0];

        SourcePlaneMapping mapdata_2;
        mapdata_2.sourceType = source;
        mapdata_2.sourceIndex = 1;
        mapdata_2.destinationPlaneIndex = sVideoPlaneIndices[1];

        sourcePlaneMappings = {mapdata_1, mapdata_2};

        UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping(IN:sourceType:%d, planes:[%d,%d], OUT:mappingResult)",
                    static_cast<int>(source), sVideoPlaneIndices[0], sVideoPlaneIndices[1]);
        status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(sourcePlaneMappings, &mappingResult);
        UT_LOG_INFO("Result setVideoSourceDestinationPlaneMapping(IN:sourceType:%d, OUT:mappingResult:[%d], status:[isOK:%s, exception:%s])",
                    static_cast<int>(source), mappingResult,
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk()) << "Failed to set mappings for sourceType " << static_cast<int>(source);
        UT_ASSERT_TRUE(mappingResult) << "Mapping failed for sourceType " << static_cast<int>(source);

        // Verify the mappings
        UT_LOG_INFO("Calling getVideoSourceDestinationPlaneMapping(OUT:retrievedMappings[])");
        status = gPlaneControlservice->getVideoSourceDestinationPlaneMapping(&retrievedMappings);
        UT_LOG_INFO("Result getVideoSourceDestinationPlaneMapping(OUT:retrievedMappings[], status:[isOK:%s, exception:%s])",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk()) << "Failed to get mappings for sourceType " << static_cast<int>(source);

        for (const auto& mapping : sourcePlaneMappings) {
            auto it = std::find_if(retrievedMappings.begin(), retrievedMappings.end(),
                                [&mapping](const SourcePlaneMapping& retrieved) {
                                    return retrieved.destinationPlaneIndex == mapping.destinationPlaneIndex &&
                                            retrieved.sourceType == mapping.sourceType &&
                                            retrieved.sourceIndex == mapping.sourceIndex;
                                });
            UT_ASSERT_NOT_EQUAL(it, retrievedMappings.end()) << "Mapping not found for sourceType " << static_cast<int>(source)
                                                            << ", sourceIndex " << mapping.sourceIndex
                                                            << ", plane " << mapping.destinationPlaneIndex;
            UT_ASSERT_EQUAL(it->sourceType, mapping.sourceType) << "SourceType mismatch for plane " << mapping.destinationPlaneIndex;
            UT_ASSERT_EQUAL(it->sourceIndex, mapping.sourceIndex) << "SourceIndex mismatch for plane " << mapping.destinationPlaneIndex;
        }
    }
}

/** @} */ // End of Plane_Control_Halif_Test_L2
/** @} */ // End of Plane_Control_Halif_Test
/** @} */ // End of Plane_Control
/** @} */ // End of HPK