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
    static std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> sCapabilities;
    static std::vector<uint32_t> sVideoPlaneIndices;
    static std::vector<uint32_t> sGraphicsPlaneIndices;
    static std::vector<::com::rdk::hal::planecontrol::SourceType> sAllSourceTypes;

    static void SetUpTestSuite() {

        if (gPlaneControlservice == nullptr) {
                UT_FAIL_FATAL("PlaneControl Service not available");
        }

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
            } else if (capability.type == PlaneType::GRAPHICS) {
                sGraphicsPlaneIndices.push_back(capability.planeIndex);
            }
        }
        if (sVideoPlaneIndices.size() < 2) {
            UT_LOG_WARNING("Only %zu video planes found, some tests may be limited", sVideoPlaneIndices.size());
        }
        if (sGraphicsPlaneIndices.empty()) {
            UT_LOG_WARNING("No graphics planes found, GraphicsFb lifecycle tests will be skipped");
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

class RecordingGraphicsFbProviderListener : public BnGraphicsFbProviderListener {
public:
    ::android::binder::Status onGraphicsFbReleased(int32_t oldGraphicsFbId, int64_t elapsedRealtimeNanos) override {
        std::lock_guard<std::mutex> lock(mMutex);
        mCallbackReceived = true;
        mOldGraphicsFbId = oldGraphicsFbId;
        mElapsedRealtimeNanos = elapsedRealtimeNanos;
        mCondition.notify_all();
        return ::android::binder::Status::ok();
    }

    bool waitForCallback(int32_t& oldGraphicsFbId, int64_t& elapsedRealtimeNanos) {
        std::unique_lock<std::mutex> lock(mMutex);
        const bool ready = mCondition.wait_for(lock, std::chrono::milliseconds(500), [this]() {
            return mCallbackReceived;
        });
        if (!ready) {
            return false;
        }

        oldGraphicsFbId = mOldGraphicsFbId;
        elapsedRealtimeNanos = mElapsedRealtimeNanos;
        return true;
    }

private:
    std::mutex mMutex;
    std::condition_variable mCondition;
    bool mCallbackReceived = false;
    int32_t mOldGraphicsFbId = -1;
    int64_t mElapsedRealtimeNanos = -1;
};

// Static member initialization
std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> VTS_L2_PlaneControl::sCapabilities;
std::vector<uint32_t> VTS_L2_PlaneControl::sVideoPlaneIndices;
std::vector<uint32_t> VTS_L2_PlaneControl::sGraphicsPlaneIndices;
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

        std::string propPath_width = PLANE_CONTROL_MGR_PLANES + "."  + std::to_string(planeIndex) + ".frameWidth";
        uint32_t value_width = UT_KVP_PROFILE_GET_UINT32(propPath_width.c_str());
        PLANE_CONTROL_SET_CONFIG_UINT32_PROPERTY(planeIndex, WIDTH, WIDTH, frameWidth);
        PLANE_CONTROL_GET_CONFIG_UINT32_PROPERTY(planeIndex, WIDTH, WIDTH, value_width);

        std::string propPath_height = PLANE_CONTROL_MGR_PLANES + "."  + std::to_string(planeIndex) + ".frameHeight";
        uint32_t value_height = UT_KVP_PROFILE_GET_UINT32(propPath_height.c_str());
        PLANE_CONTROL_SET_CONFIG_UINT32_PROPERTY(planeIndex, HEIGHT, HEIGHT, frameHeight);
        PLANE_CONTROL_GET_CONFIG_UINT32_PROPERTY(planeIndex, HEIGHT, HEIGHT, value_height);

        PLANE_CONTROL_SET_INT_ZERO_PROPERTY(planeIndex, X, X);
        PLANE_CONTROL_GET_INT_ZERO_PROPERTY(planeIndex, X, X);

        PLANE_CONTROL_SET_INT_ZERO_PROPERTY(planeIndex, Y, Y);
        PLANE_CONTROL_GET_INT_ZERO_PROPERTY(planeIndex, Y, Y);

        PLANE_CONTROL_SET_GET_CONFIG_BOOL_PROPERTY(planeIndex, ALPHA, ALPHA, supportsAlpha);

        PLANE_CONTROL_SET_GET_CONFIG_BOOL_PROPERTY(planeIndex, ZORDER, ZORDER, supportsZOrder);

        // VISIBLE
        PLANE_CONTROL_SET_INT_ONE_PROPERTY(planeIndex, VISIBLE, VISIBLE);
        PLANE_CONTROL_GET_INT_ONE_PROPERTY(planeIndex, VISIBLE, VISIBLE);


        std::string planeTypePath = PLANE_CONTROL_MGR_PLANES + "."  + std::to_string(planeIndex) + ".type";
        char planeTypeString[256];
        UT_KVP_PROFILE_GET_STRING(planeTypePath.c_str(), planeTypeString);
        if (std::string(planeTypeString) == "VIDEO") {

            PLANE_CONTROL_SET_INT_ZERO_PROPERTY(planeIndex, OVERSCAN, OVERSCAN);
            PLANE_CONTROL_GET_INT_ZERO_PROPERTY(planeIndex, OVERSCAN, OVERSCAN);

            PLANE_CONTROL_SET_INT_ONE_PROPERTY(planeIndex, ASPECT_RATIO, ASPECT_RATIO);
            PLANE_CONTROL_GET_INT_ONE_PROPERTY(planeIndex, ASPECT_RATIO, ASPECT_RATIO);
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
        std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
        ::com::rdk::hal::planecontrol::PropertyKVPair kvPair;

        kvPair.property = com::rdk::hal::planecontrol::Property::WIDTH;
        std::string frameWidthPath = PLANE_CONTROL_MGR_PLANES + "."  + std::to_string(planeIndex) + ".frameWidth";
        uint32_t frame_width = UT_KVP_PROFILE_GET_UINT32(frameWidthPath.c_str());
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(frame_width);
        propertyKVList.push_back(kvPair);

        kvPair.property = com::rdk::hal::planecontrol::Property::HEIGHT;
        std::string frameHeightPath = PLANE_CONTROL_MGR_PLANES + "."  + std::to_string(planeIndex) + ".frameHeight";
        uint32_t frame_height = UT_KVP_PROFILE_GET_UINT32(frameHeightPath.c_str());
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(frame_height);
        propertyKVList.push_back(kvPair);

        kvPair.property = ::com::rdk::hal::planecontrol::Property::X;
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
        propertyKVList.push_back(kvPair);

        kvPair.property = ::com::rdk::hal::planecontrol::Property::Y;
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
        propertyKVList.push_back(kvPair);

        std::string supportsAlphaPath = PLANE_CONTROL_MGR_PLANES + "."  + std::to_string(planeIndex) + ".supportsAlpha";
        bool supportsAlpha = UT_KVP_PROFILE_GET_BOOL(supportsAlphaPath.c_str());
        if (supportsAlpha) {
            kvPair.property = ::com::rdk::hal::planecontrol::Property::ALPHA;
            kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
            propertyKVList.push_back(kvPair);
        }

        std::string supportsZOrderPath = PLANE_CONTROL_MGR_PLANES + "."  + std::to_string(planeIndex) + ".supportsZOrder";
        bool supportsZOrder = UT_KVP_PROFILE_GET_BOOL(supportsZOrderPath.c_str());
        if (supportsZOrder) {
            kvPair.property = ::com::rdk::hal::planecontrol::Property::ZORDER;
            kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
            propertyKVList.push_back(kvPair);
        }

        kvPair.property = ::com::rdk::hal::planecontrol::Property::VISIBLE;
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
        propertyKVList.push_back(kvPair);

        std::string planeTypePath = PLANE_CONTROL_MGR_PLANES + "."  + std::to_string(planeIndex) + ".type";
        char planeTypeString[256];
        UT_KVP_PROFILE_GET_STRING(planeTypePath.c_str(), planeTypeString);
        if (std::string(planeTypeString) == "VIDEO") {
            kvPair.property = ::com::rdk::hal::planecontrol::Property::OVERSCAN;
            kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
            propertyKVList.push_back(kvPair);

            kvPair.property = ::com::rdk::hal::planecontrol::Property::ASPECT_RATIO;
            kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
            propertyKVList.push_back(kvPair);
        }

        PLANE_CONTROL_SET_PROPERTY_MULTI_ATOMIC(planeIndex, propertyKVList);

        // Get the property value using getPropertyMulti
        std::vector<com::rdk::hal::planecontrol::Property> getProperties;
        for (const auto& kv : propertyKVList) {
            getProperties.push_back(kv.property);
        }

        std::vector<com::rdk::hal::planecontrol::PropertyKVPair> getPropertyKVList;
        PLANE_CONTROL_VALIDATE_GET_PROPERTY_MULTI(planeIndex, getProperties, getPropertyKVList, "all");

        for (const auto& setKv : propertyKVList) {
            bool foundMatchingProperty = false;
            for (const auto& getKv : getPropertyKVList) {
                if (getKv.property != setKv.property) {
                    continue;
                }

                foundMatchingProperty = true;
                UT_ASSERT_TRUE(setKv.propertyValue.value.has_value())
                    << "Set value missing for property " << static_cast<int32_t>(setKv.property)
                    << " on plane " << planeIndex;
                UT_ASSERT_TRUE(getKv.propertyValue.value.has_value())
                    << "Get value missing for property " << static_cast<int32_t>(setKv.property)
                    << " on plane " << planeIndex;

                const int32_t expectedVal = setKv.propertyValue.value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>();
                const int32_t actualVal = getKv.propertyValue.value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>();
                UT_ASSERT_EQUAL(actualVal, expectedVal)
                    << "Round-trip mismatch for property " << static_cast<int32_t>(setKv.property)
                    << " on plane " << planeIndex;
                break;
            }

            UT_ASSERT_TRUE(foundMatchingProperty)
                << "Property missing in getPropertyMulti response for plane " << planeIndex
                << ", property " << static_cast<int32_t>(setKv.property);
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
    UT_LOG_INFO("Starting TestGetSetVideoSourceDestinationPlaneMapping");
    UT_ASSERT_GREATER_EQUAL(sVideoPlaneIndices.size(), 2) << "Need at least 2 video planes, found " << sVideoPlaneIndices.size();

    for (const auto& sourceType : sAllSourceTypes) {
        PLANE_CONTROL_SET_MAPPING(sourceType, sVideoPlaneIndices[0], sVideoPlaneIndices[1]);
        PLANE_CONTROL_GET_MAPPING(sourceType, sVideoPlaneIndices[0], sVideoPlaneIndices[1]);
    }
}


/** @} */ // End of Plane_Control_Halif_Test_L2
/** @} */ // End of Plane_Control_Halif_Test
/** @} */ // End of Plane_Control
/** @} */ // End of HPK
