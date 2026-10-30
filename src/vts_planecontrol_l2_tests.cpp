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

    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);

    for (const auto& capability : capabilities) {
        int32_t planeIndex = capability.planeIndex;

        // WIDTH/HEIGHT are bounded by maxWidth/maxHeight, which are <= frameWidth/frameHeight
        setAndVerifyIntProperty(planeIndex, ::com::rdk::hal::planecontrol::Property::WIDTH, capability.maxWidth);
        setAndVerifyIntProperty(planeIndex, ::com::rdk::hal::planecontrol::Property::HEIGHT, capability.maxHeight);

        setAndVerifyIntProperty(planeIndex, ::com::rdk::hal::planecontrol::Property::X, 0);
        setAndVerifyIntProperty(planeIndex, ::com::rdk::hal::planecontrol::Property::Y, 0);

        // ALPHA requires supportsAlpha (Property.aidl)
        if (planeSupportsProperty(capabilities, capability, ::com::rdk::hal::planecontrol::Property::ALPHA)) {
            setAndVerifyIntProperty(planeIndex, ::com::rdk::hal::planecontrol::Property::ALPHA, 255);
        } else {
            UT_LOG_INFO("Skipping ALPHA for plane %d: supportsAlpha is false", planeIndex);
        }

        // ZORDER is optional and is always false for the primary graphics plane (PlaneCapabilities.aidl)
        if (planeSupportsProperty(capabilities, capability, ::com::rdk::hal::planecontrol::Property::ZORDER)) {
            setAndVerifyIntProperty(planeIndex, ::com::rdk::hal::planecontrol::Property::ZORDER, planeIndex);
        } else {
            UT_LOG_INFO("Skipping ZORDER for plane %d: supportsZOrder is false", planeIndex);
        }

        setAndVerifyIntProperty(planeIndex, ::com::rdk::hal::planecontrol::Property::VISIBLE, 1);

        // OVERSCAN/ASPECT_RATIO only apply to VIDEO planes
        if (planeSupportsProperty(capabilities, capability, ::com::rdk::hal::planecontrol::Property::OVERSCAN)) {
            setAndVerifyIntProperty(planeIndex, ::com::rdk::hal::planecontrol::Property::OVERSCAN, 0);
            setAndVerifyIntProperty(planeIndex, ::com::rdk::hal::planecontrol::Property::ASPECT_RATIO, 1);
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
    UT_LOG_INFO("Starting TestGetSetPropertyMultiAtomic");

    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);

    for (const auto& capability : capabilities) {
        int32_t planeIndex = capability.planeIndex;

        // Prepare Key value pair of the available properties
        std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
        ::com::rdk::hal::planecontrol::PropertyKVPair kvPair;

        kvPair.property = com::rdk::hal::planecontrol::Property::WIDTH;
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(capability.maxWidth);
        propertyKVList.push_back(kvPair);

        kvPair.property = com::rdk::hal::planecontrol::Property::HEIGHT;
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(capability.maxHeight);
        propertyKVList.push_back(kvPair);

        kvPair.property = ::com::rdk::hal::planecontrol::Property::X;
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
        propertyKVList.push_back(kvPair);

        kvPair.property = ::com::rdk::hal::planecontrol::Property::Y;
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
        propertyKVList.push_back(kvPair);

        if (planeSupportsProperty(capabilities, capability, ::com::rdk::hal::planecontrol::Property::ALPHA)) {
            kvPair.property = ::com::rdk::hal::planecontrol::Property::ALPHA;
            kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(0);
            propertyKVList.push_back(kvPair);
        }

        if (planeSupportsProperty(capabilities, capability, ::com::rdk::hal::planecontrol::Property::ZORDER)) {
            kvPair.property = ::com::rdk::hal::planecontrol::Property::ZORDER;
            kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
            propertyKVList.push_back(kvPair);
        }

        kvPair.property = ::com::rdk::hal::planecontrol::Property::VISIBLE;
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
        propertyKVList.push_back(kvPair);

        if (planeSupportsProperty(capabilities, capability, ::com::rdk::hal::planecontrol::Property::OVERSCAN)) {
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
    UT_ASSERT_FALSE(sVideoPlaneIndices.empty()) << "No VIDEO planes found";

    const auto firstVideoCapability = std::find_if(
        sCapabilities.begin(), sCapabilities.end(), [](const auto& capability) {
            return capability.planeIndex == static_cast<int32_t>(sVideoPlaneIndices[0]);
        });
    UT_ASSERT_NOT_EQUAL(firstVideoCapability, sCapabilities.end());

    std::vector<SourcePlaneMapping> existingMappings;
    auto cleanupStatus =
        gPlaneControlservice->getVideoSourceDestinationPlaneMapping(&existingMappings);
    UT_ASSERT_TRUE(cleanupStatus.isOk())
        << "Failed to retrieve mappings before cleanup";

    std::vector<SourcePlaneMapping> cleanupMappings;
    for (const auto& existing : existingMappings) {
        if (existing.sourceType != SourceType::NONE) {
            auto unmap = existing;
            unmap.destinationPlaneIndex = -1;
            cleanupMappings.push_back(unmap);
        }
    }
    if (!cleanupMappings.empty()) {
        bool cleanupResult = false;
        cleanupStatus = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(
            cleanupMappings, &cleanupResult);
        PLANE_CONTROL_ASSERT_STATUS(
            setVideoSourceDestinationPlaneMapping, cleanupStatus, cleanupResult, -1,
            cleanupMappings);
    }

    bool testedSupportedSourceType = false;
    for (const auto& sourceType : sAllSourceTypes) {
        if (!PlaneSupportsSourceType(*firstVideoCapability, sourceType)) {
            UT_LOG_INFO("Skipping sourceType %d because it is not supported by VIDEO plane %u",
                        static_cast<int>(sourceType), sVideoPlaneIndices[0]);
            continue;
        }

        std::vector<SourcePlaneMapping> mappings;
        SourcePlaneMapping mapping;
        mapping.sourceType = sourceType;
        mapping.sourceIndex = 0;
        mapping.destinationPlaneIndex = sVideoPlaneIndices[0];
        mappings.push_back(mapping);

        bool mappingResult = false;
        auto status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(mappings, &mappingResult);
        PLANE_CONTROL_ASSERT_STATUS(setVideoSourceDestinationPlaneMapping, status, mappingResult, -1, sourceType);

        std::vector<SourcePlaneMapping> retrievedMappings;
        status = gPlaneControlservice->getVideoSourceDestinationPlaneMapping(&retrievedMappings);
        UT_ASSERT_TRUE(status.isOk()) << "Failed to get mappings for sourceType "
                                      << static_cast<int>(sourceType);
        const auto retrievedMapping = std::find_if(
            retrievedMappings.begin(), retrievedMappings.end(), [&mapping](const auto& retrieved) {
                return retrieved.destinationPlaneIndex == mapping.destinationPlaneIndex;
            });
        UT_ASSERT_NOT_EQUAL(retrievedMapping, retrievedMappings.end());
        UT_ASSERT_EQUAL(retrievedMapping->sourceType, mapping.sourceType);
        UT_ASSERT_EQUAL(retrievedMapping->sourceIndex, mapping.sourceIndex);

        SourcePlaneMapping unmap;
        unmap.sourceType = sourceType;
        unmap.sourceIndex = 0;
        unmap.destinationPlaneIndex = -1;
        mappings.assign(1, unmap);
        status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(mappings, &mappingResult);
        PLANE_CONTROL_ASSERT_STATUS(setVideoSourceDestinationPlaneMapping, status, mappingResult, -1, unmap);

        testedSupportedSourceType = true;
    }
    if (!testedSupportedSourceType) {
        GTEST_SKIP() << "The selected VIDEO plane does not advertise a mappable source type";
    }
}


/** @} */ // End of Plane_Control_Halif_Test_L2
/** @} */ // End of Plane_Control_Halif_Test
/** @} */ // End of Plane_Control
/** @} */ // End of HPK
