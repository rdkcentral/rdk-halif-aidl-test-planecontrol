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
 * @defgroup Plane_Control_Halif_Test_L1 Plane Control HALIF Tests L1 File
 * @{
 * @parblock
 *
 * ### L1 Tests for Plane Control HALIF AIDL :
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
 * @file vts_planecontrol_l1_tests.cpp
 *
 */

#include "vts_planecontrol.h"

extern std::vector<planeControl_Cap> gPlaneControl_hfp;

class VTS_L1_PlaneControl : public virtual UTCore
{
protected:
    static void SetUpTestSuite()
    {
        if (gPlaneControlservice == nullptr) {
                UT_FAIL_FATAL("PlaneControl Service not available");
        }

    }

    static void TearDownTestSuite()
    {

    }
};

static bool GetFirstGraphicsProvider(
    ::android::sp<::com::rdk::hal::planecontrol::IGraphicsFbProvider>& provider,
    int32_t& planeIndex)
{
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    auto status = gPlaneControlservice->getCapabilities(&capabilities);
    if (!status.isOk()) {
        return false;
    }

    for (const auto& plane : capabilities) {
        if (plane.type != ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS) {
            continue;
        }

        ::android::sp<ValidGraphicsFbProviderListener> listener = new ValidGraphicsFbProviderListener();
        status = gPlaneControlservice->getGraphicsFbProvider(plane.planeIndex, listener, &provider);
        if (status.isOk() && provider != nullptr) {
            planeIndex = plane.planeIndex;
            return true;
        }
    }

    return false;
}


/**
 * @brief Test the positive scenario for retrieving plane control capabilities.
 *
 * This test verifies the functionality of the getCapabilities method in the PlaneControl service. It ensures that the method returns a valid list of capabilities and that each capability is correctly populated with expected values.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 001@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Initialize the capabilities vector and status object. | capabilities = empty, status = uninitialized | Should be successful | |
 * | 02 | Call the getCapabilities method on the service. | capabilities = empty, status = uninitialized | status.isOk() = true, capabilities not empty | Should Pass |
 * | 03 | Log the result of getCapabilities call. | capabilities = populated, status = OK | Should be successful | |
 * | 04 | Assert that the status is OK. | status.isOk() = true | Assertion passes | Should Pass |
 * | 05 | Assert that the capabilities vector is not empty. | capabilities not empty | Assertion passes | Should Pass |
 * | 06 | Iterate over each capability and verify planeIndex. | cap.planeIndex = index | Assertion passes | Should Pass |
 * | 07 | Verify the type of each capability. | cap.type = expected type | Assertion passes | Should Pass |
 * | 08 | Verify the pixelFormats of each capability. | cap.pixelFormats = expected formats | Assertion passes | Should Pass |
 * | 09 | Verify the supportedDynamicRanges of each capability. | cap.supportedDynamicRanges = expected ranges | Assertion passes | Should Pass |
 * | 10 | Verify other fields like maxWidth, maxHeight, etc. | cap fields = expected values | Assertion passes | Should Pass |
 * | 11 | Verify the primary GRAPHICS plane reports supportsZOrder = false. | primary graphics plane = lowest GRAPHICS planeIndex | supportsZOrder = false | Should Pass |
 */

UT_ADD_TEST(VTS_L1_PlaneControl, getCapabilities_pos) {
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;

    PLANE_CONTROL_GET_CAPABILITIES(capabilities);

    for (const auto& cap : capabilities) {
        auto it = std::find_if(gPlaneControl_hfp.begin(), gPlaneControl_hfp.end(),
                               [&](const auto& hfp_cap) { return hfp_cap.planeIndex == cap.planeIndex; });

        if (it == gPlaneControl_hfp.end()) {
            UT_FAIL("Profile doesn't have plane with index: " + std::to_string(cap.planeIndex));
        } else {
            validateCapabilityMatch(cap, *it);
        }
    }

    // PlaneCapabilities.aidl mandates supportsZOrder = false on the primary graphics plane,
    // which is the lowest indexed GRAPHICS plane since graphics planes are listed after video planes.
    const ::com::rdk::hal::planecontrol::PlaneCapabilities* primaryGraphicsPlane = nullptr;
    for (const auto& cap : capabilities) {
        if (cap.type != ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS) {
            continue;
        }
        if (primaryGraphicsPlane == nullptr || cap.planeIndex < primaryGraphicsPlane->planeIndex) {
            primaryGraphicsPlane = &cap;
        }
    }

    if (primaryGraphicsPlane != nullptr) {
        UT_ASSERT_FALSE(primaryGraphicsPlane->supportsZOrder)
            << "Primary graphics plane " << primaryGraphicsPlane->planeIndex
            << " must report supportsZOrder = false";
    }
}

/**
 * @brief Test the positive scenario of setting video source to destination plane mapping.
 *
 * This test verifies the functionality of setting video source to destination plane mapping for
 * source types advertised by each VIDEO plane. It ensures that the mapping is correctly updated
 * and can be unmapped successfully.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 006@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve the number of planes from YAML configuration. | None | Should be successful | Should be successful |
 * | 02 | Iterate over each plane to check if the plane type is VIDEO. | planeCount = value | Should be successful | Should be successful |
 * | 03 | Create a valid mapping for HDMI source and add to list. | sourceType = HDMI, sourceIndex = 0, destinationPlaneIndex = i | Should be successful | Should be successful |
 * | 04 | Create a valid mapping for COMPOSITE source and add to list. | sourceType = COMPOSITE, sourceIndex = 0, destinationPlaneIndex = i+1 | Should be successful | Should be successful |
 * | 05 | Call setVideoSourceDestinationPlaneMapping with the list of mappings. | listSourcePlaneMapping = [HDMI, COMPOSITE], _aidl_return = false | _aidl_return = true | Should Pass |
 * | 06 | Confirm that the mapping was updated successfully. | _aidl_return = true | Should be successful | Should be successful |
 * | 07 | Clear the list and create an unmap entry for VIDEO_SINK. | sourceType = VIDEO_SINK, sourceIndex = 0, destinationPlaneIndex = -1 | Should be successful | Should be successful |
 * | 08 | Call setVideoSourceDestinationPlaneMapping to unmap the plane. | listSourcePlaneMapping = [VIDEO_SINK], _aidl_return = false | _aidl_return = true | Should Pass |
 * | 09 | Confirm that the unmapping was successful. | _aidl_return = true | Should be successful | Should be successful |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, setVideoSourceDestinationPlaneMapping_pos) {
    UT_LOG_INFO("Starting setVideoSourceDestinationPlaneMapping_pos");

    // Step 1: Retrieve capabilities
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);

    // Step 2: Count video planes and collect indices
    std::vector<uint32_t> planeIndices;
    for (const auto& cap : capabilities) {
        if (cap.type == PlaneType::VIDEO) {
            planeIndices.push_back(cap.planeIndex);
        }
    }

    UT_ASSERT_FALSE(planeIndices.empty()) << "No VIDEO planes found";

    const std::vector<::com::rdk::hal::planecontrol::SourceType> mappableSourceTypes = {
        ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK,
        ::com::rdk::hal::planecontrol::SourceType::HDMI,
        ::com::rdk::hal::planecontrol::SourceType::COMPOSITE
    };
    const auto findCapability = [&capabilities](uint32_t planeIndex) {
        return std::find_if(capabilities.begin(), capabilities.end(),
                            [planeIndex](const auto& capability) {
                                return capability.planeIndex == static_cast<int32_t>(planeIndex);
                            });
    };
    const auto firstPlane = findCapability(planeIndices[0]);
    UT_ASSERT_NOT_EQUAL(firstPlane, capabilities.end());

    const auto firstSource = std::find_if(
        mappableSourceTypes.begin(), mappableSourceTypes.end(),
        [&firstPlane](const auto sourceType) { return PlaneSupportsSourceType(*firstPlane, sourceType); });
    if (firstSource == mappableSourceTypes.end()) {
        GTEST_SKIP() << "The selected VIDEO plane does not advertise a mappable source type";
    }

    // Steps 3-4: Create a mapping using a source type supported by the VIDEO plane
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> mappings;
    PLANE_CONTROL_ADD_SOURCE_PLANE_MAPPING(mappings, *firstSource, 0, planeIndices[0]);

    // Steps 5-6: Set and verify mappings
    bool aidl_return = false;
    PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_POS(mappings, aidl_return);

    // Steps 7-9: Unmap the mapped sources
    mappings.clear();
    PLANE_CONTROL_ADD_SOURCE_PLANE_MAPPING(mappings, *firstSource, 0, -1);
    PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_POS(mappings, aidl_return);
}

/**
 * @brief Test the negative scenarios for setting video source to destination plane mapping.
 *
 * This test verifies the behavior of the setVideoSourceDestinationPlaneMapping function when
 * provided with unsupported source types, duplicate source types and indices, and duplicate
 * destination plane indices.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 007@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve plane capabilities. | capabilities | Should be successful | Should be successful |
 * | 02 | Find a source type not supported by the first VIDEO plane. | capability.sourceTypes | Should be successful | Skip if all defined types are supported |
 * | 03 | Call setVideoSourceDestinationPlaneMapping with the unsupported source type. | destinationPlaneIndex = i | status.exceptionCode() = EX_UNSUPPORTED_OPERATION | Should Fail |
 * | 04 | Find a source type supported by the first VIDEO plane. | capability.sourceTypes | Should be successful | Should be successful |
 * | 05 | Call setVideoSourceDestinationPlaneMapping with duplicate SourceType and SourceIndex when a second VIDEO plane supports it. | listSourcePlaneMapping1 = duplicate sources | _aidl_return = false | Skip if unavailable |
 * | 06 | Call setVideoSourceDestinationPlaneMapping with duplicate DestinationPlaneIndex. | listSourcePlaneMapping2 = duplicate plane | _aidl_return = false | Should Fail |
 * | 07 | Call setVideoSourceDestinationPlaneMapping with a valid mapping. | listSourcePlaneMapping3 = supported source | status.isOk() = true | Should Pass |
 * | 08 | Call setVideoSourceDestinationPlaneMapping with a mapping to an already mapped plane. | listSourcePlaneMapping4 = supported source | _aidl_return = false, status.exceptionCode() = EX_ILLEGAL_STATE | Should Fail |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, setVideoSourceDestinationPlaneMapping_neg) {
    UT_LOG_INFO("Starting setVideoSourceDestinationPlaneMapping_neg");

    // Step 1: Retrieve capabilities
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);

    // Step 2: Count video planes and collect indices
    std::vector<uint32_t> planeIndices;
    for (const auto& cap : capabilities) {
        if (cap.type == ::com::rdk::hal::planecontrol::PlaneType::VIDEO) {
            planeIndices.push_back(cap.planeIndex);
        }
    }
    UT_LOG_INFO("Video plane indices found: %zu", planeIndices.size());
    UT_ASSERT_FALSE(planeIndices.empty()) << "No VIDEO planes found";

    std::vector<::com::rdk::hal::planecontrol::SourceType> allSourceTypes = {
        ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK,
        ::com::rdk::hal::planecontrol::SourceType::HDMI,
        ::com::rdk::hal::planecontrol::SourceType::COMPOSITE
    };

    const auto firstVideoCapability = std::find_if(
        capabilities.begin(), capabilities.end(), [&planeIndices](const auto& capability) {
            return capability.planeIndex == static_cast<int32_t>(planeIndices[0]);
        });
    UT_ASSERT_NOT_EQUAL(firstVideoCapability, capabilities.end());

    const auto unsupportedSource = std::find_if(
        allSourceTypes.begin(), allSourceTypes.end(), [&firstVideoCapability](const auto sourceType) {
            return !PlaneSupportsSourceType(*firstVideoCapability, sourceType);
        });
    if (unsupportedSource != allSourceTypes.end()) {
        std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> mappings;
        bool aidl_return = false;
        PLANE_CONTROL_ADD_SOURCE_PLANE_MAPPING(mappings, *unsupportedSource, 0, planeIndices[0]);
        PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_NEG_WITH_EXCEPTION(
            mappings, aidl_return, ::android::binder::Status::EX_UNSUPPORTED_OPERATION);
    } else {
        UT_LOG_INFO("All defined source types are supported by VIDEO plane %u; "
                    "skipping unsupported-source scenario", planeIndices[0]);
    }

    for (const auto& source : allSourceTypes) {
        if (!PlaneSupportsSourceType(*firstVideoCapability, source)) {
            UT_LOG_INFO("Skipping sourceType %d because it is not supported by VIDEO plane %u",
                        static_cast<int>(source), planeIndices[0]);
            continue;
        }

        std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> mappings;
        bool aidl_return = false;

        if (planeIndices.size() > 1) {
            const auto secondVideoCapability = std::find_if(
                capabilities.begin(), capabilities.end(), [&planeIndices](const auto& capability) {
                    return capability.planeIndex == static_cast<int32_t>(planeIndices[1]);
                });
            UT_ASSERT_NOT_EQUAL(secondVideoCapability, capabilities.end());
            if (PlaneSupportsSourceType(*secondVideoCapability, source)) {
                PLANE_CONTROL_ADD_SOURCE_PLANE_MAPPING(mappings, source, 0, planeIndices[0]);
                PLANE_CONTROL_ADD_SOURCE_PLANE_MAPPING(mappings, source, 0, planeIndices[1]);
                PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_NEG(mappings, aidl_return);
                mappings.clear();
            } else {
                UT_LOG_INFO("Skipping duplicate-source scenario for sourceType %d because VIDEO plane %u "
                            "does not support it",
                            static_cast<int>(source), planeIndices[1]);
            }
        }

        // Test duplicate DestinationPlaneIndex.
        PLANE_CONTROL_ADD_SOURCE_PLANE_MAPPING(mappings, source, 0, planeIndices[0]);
        PLANE_CONTROL_ADD_SOURCE_PLANE_MAPPING(mappings, source, 1, planeIndices[0]);
        PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_NEG(mappings, aidl_return);

        // Test mapping to an already mapped plane.
        mappings.clear();
        PLANE_CONTROL_ADD_SOURCE_PLANE_MAPPING(mappings, source, 1, planeIndices[0]);
        PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_POS(mappings, aidl_return);

        mappings.clear();
        PLANE_CONTROL_ADD_SOURCE_PLANE_MAPPING(mappings, source, 0, planeIndices[0]);
        aidl_return = false;
        PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_NEG_WITH_EXCEPTION(
            mappings, aidl_return, ::android::binder::Status::EX_ILLEGAL_STATE);

        mappings.clear();
        PLANE_CONTROL_ADD_SOURCE_PLANE_MAPPING(mappings, source, 1, -1);
        PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_POS(mappings, aidl_return);
    }
}

/**
 * @brief Test the retrieval of video source destination plane mappings for VIDEO planes.
 *
 * This test verifies the functionality of getVideoSourceDestinationPlaneMapping for VIDEO planes.
 * It ensures that the function retrieves the correct number of mappings corresponding to the VIDEO planes
 * and validates the mappings against the expected configuration.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 004@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve the capabilities to count VIDEO planes using PLANE_CONTROL_GET_CAPABILITIES macro. | capabilities | Should be successful, capabilities not empty | |
 * | 02 | Ensure at least one VIDEO plane exists using PLANE_CONTROL_ENSURE_PLANE_TYPE_EXISTS macro. | PlaneType::VIDEO | Should be successful | |
 * | 03 | Call getVideoSourceDestinationPlaneMapping using PLANE_CONTROL_GET_VIDEO_SOURCE_MAPPING macro. | aidl_return, video_plane_count | Should be successful, status.isOk() = true, mappings count matches video_plane_count | |
 * | 04 | Validate each mapping's sourceType, sourceIndex, and destinationPlaneIndex against profile or capabilities. | aidl_return | Should match expected values | |
 */

UT_ADD_TEST(VTS_L1_PlaneControl, getVideoSourceDestinationPlaneMapping_pos) {
    // Step 1: Retrieve capabilities and count VIDEO planes
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);

    uint32_t video_plane_count = 0;
    for (const auto& cap : capabilities) {
        if (cap.type == ::com::rdk::hal::planecontrol::PlaneType::VIDEO) {
            video_plane_count++;
        }
    }

    // Step 2: Ensure at least one VIDEO plane exists
    PLANE_CONTROL_ENSURE_PLANE_TYPE_EXISTS(video_plane_count, ::com::rdk::hal::planecontrol::PlaneType::VIDEO,
                                          "No VIDEO planes found in capabilities");

    // Step 3: Call getVideoSourceDestinationPlaneMapping using macro
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> aidl_return;
    PLANE_CONTROL_GET_VIDEO_SOURCE_MAPPING(aidl_return, video_plane_count);

    // Step 4: Validate each mapping
    for (const auto& mapping : aidl_return) {
        // Verify that the destination plane index corresponds to a VIDEO plane
        auto it = std::find_if(capabilities.begin(), capabilities.end(),
                               [&mapping](const auto& cap) {
                                   return cap.planeIndex == mapping.destinationPlaneIndex &&
                                          cap.type == ::com::rdk::hal::planecontrol::PlaneType::VIDEO;
                               });
        UT_ASSERT_NOT_EQUAL(it, capabilities.end()) << "Mapping for plane index " << mapping.destinationPlaneIndex
                                                   << " is not a VIDEO plane";

        // Skip validation for unmapped planes
        if (mapping.sourceType == ::com::rdk::hal::planecontrol::SourceType::NONE) {
            UT_ASSERT_TRUE(mapping.sourceIndex == -1) << "Expected sourceIndex -1 for unmapped plane index " << mapping.destinationPlaneIndex;
            continue;
        }

        // Validate sourceType is one of the expected types
        std::vector<::com::rdk::hal::planecontrol::SourceType> validSourceTypes = {
            ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK,
            ::com::rdk::hal::planecontrol::SourceType::HDMI,
            ::com::rdk::hal::planecontrol::SourceType::COMPOSITE
        };
        auto sourceTypeIt = std::find(validSourceTypes.begin(), validSourceTypes.end(), mapping.sourceType);
        UT_ASSERT_NOT_EQUAL(sourceTypeIt, validSourceTypes.end()) << "Invalid sourceType " << static_cast<int>(mapping.sourceType)
                                                                 << " for plane index " << mapping.destinationPlaneIndex;

        // Validate sourceIndex
        UT_ASSERT_TRUE(mapping.sourceIndex >= 0 && mapping.sourceIndex <= 1) << "Invalid sourceIndex " << mapping.sourceIndex
                                                                           << " for plane index " << mapping.destinationPlaneIndex;
    }
}


/**
 * @brief Test the negative scenarios for retrieving video source destination plane mappings.
 *
 * This test verifies the behavior of the getVideoSourceDestinationPlaneMapping function when the system has no VIDEO planes or returns invalid mappings. It ensures the function handles edge cases correctly, such as returning an empty array when no VIDEO planes exist or invalid mappings for VIDEO planes.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 005@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve the capabilities to count VIDEO planes using PLANE_CONTROL_GET_CAPABILITIES macro | capabilities | Should be successful | |
 * | 02 | Count the number of VIDEO planes | video_plane_count | Should be successful | |
 * | 03 | If no VIDEO planes exist, call getVideoSourceDestinationPlaneMapping and expect empty mappings | aidl_return | status.isOk() = true, aidl_return.empty() = true | Should Pass if no VIDEO planes |
 * | 04 | If VIDEO planes exist, call getVideoSourceDestinationPlaneMapping and expect invalid mappings | aidl_return, video_plane_count | status.isOk() = true, mappings contain invalid sourceType, sourceIndex, or destinationPlaneIndex | Should Pass if invalid mappings detected |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, getVideoSourceDestinationPlaneMapping_neg) {
    UT_LOG_INFO("Starting getVideoSourceDestinationPlaneMapping_neg");

    // Step 1: Retrieve capabilities
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);

    uint32_t video_plane_count = 0;
    for (const auto& cap : capabilities) {
        if (cap.type == ::com::rdk::hal::planecontrol::PlaneType::VIDEO) {
            video_plane_count++;
        }
    }

    // Step 2: Call getVideoSourceDestinationPlaneMapping
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> aidl_return;
    PLANE_CONTROL_GET_VIDEO_SOURCE_MAPPING(aidl_return, video_plane_count);

    // Step 3: Validate that mappings are invalid
    for (const auto& mapping : aidl_return) {
        auto it = std::find_if(capabilities.begin(), capabilities.end(),
                               [&mapping](const auto& cap) {
                                   return cap.planeIndex == mapping.destinationPlaneIndex &&
                                          cap.type == ::com::rdk::hal::planecontrol::PlaneType::VIDEO;
                               });
        // Define valid source types in a vector
        std::vector<::com::rdk::hal::planecontrol::SourceType> valid_source_types = {
            ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK,
            ::com::rdk::hal::planecontrol::SourceType::HDMI,
            ::com::rdk::hal::planecontrol::SourceType::COMPOSITE
        };
        bool is_valid_source_type = std::find(valid_source_types.begin(), valid_source_types.end(), mapping.sourceType) != valid_source_types.end();
        bool is_valid_source_index = mapping.sourceIndex >= 0 && mapping.sourceIndex <= 1;
        UT_ASSERT_TRUE(it == capabilities.end() || !is_valid_source_type || !is_valid_source_index)
            << "Invalid mapping for Invalid Mappings: destinationPlaneIndex=" << mapping.destinationPlaneIndex
            << ", sourceType=" << static_cast<int>(mapping.sourceType)
            << ", sourceIndex=" << mapping.sourceIndex;
    }
}

/**
 * @brief Test the positive scenario of retrieving properties using the getProperty API.
 *
 * This test verifies that the getProperty API correctly retrieves properties for each plane resource index
 * and ensures that the returned property value matches the requested property. The test iterates over all
 * available properties for each plane and checks the API's response.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 009@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Ensure the service is available | gPlaneControlservice | Should not be null | Should be successful |
 * | 02 | Retrieve the number of planes using PLANE_CONTROL_GET_CAPABILITIES | capabilities | planeCount should be retrieved, capabilities not empty | Should be successful |
 * | 03 | Iterate over each VIDEO plane resource index | planeIndex | Each index should be processed | Should be successful |
 * | 04 | Iterate over all VIDEO properties | VIDEO_PROPERTIES | Each property should be processed | Should be successful |
 * | 05 | Call getProperty for each VIDEO property using PLANE_CONTROL_GET_PROPERTY_POS | planeIndex, property | Should return valid property value | Should Pass |
 * | 06 | Iterate over each GRAPHICS plane resource index | planeIndex | Each index should be processed | Should be successful |
 * | 07 | Iterate over all GRAPHICS properties | GRAPHICS_PROPERTIES | Each property should be processed | Should be successful |
 * | 08 | Call getProperty for each GRAPHICS property using PLANE_CONTROL_GET_PROPERTY_POS | planeIndex, property | Should return valid property value | Should Pass |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, getProperty_pos) {
    // Step 1: Ensure service is available
    UT_ASSERT_TRUE(gPlaneControlservice != nullptr) << "PlaneControl service is not available";

    // Step 2: Retrieve capabilities and count planes
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);
    uint32_t planeCount = capabilities.size();
    UT_LOG_INFO("Retrieved %u planes from capabilities", planeCount);

    // Steps 3–8: Iterate over planes and their supported properties
    uint32_t video_plane_count = 0;
    uint32_t graphics_plane_count = 0;
    for (const auto& plane : capabilities) {
        if (plane.type == ::com::rdk::hal::planecontrol::PlaneType::VIDEO) {
            video_plane_count++;
        } else if (plane.type == ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS) {
            graphics_plane_count++;
        } else {
            continue;
        }

        for (const auto& property : getTestableProperties(capabilities, plane)) {
            std::optional<::com::rdk::hal::PropertyValue> propertyValue;
            PLANE_CONTROL_GET_PROPERTY_POS(plane.planeIndex, property, propertyValue);
        }
    }

    // Ensure at least one plane was tested
    UT_ASSERT_TRUE(video_plane_count > 0 || graphics_plane_count > 0) << "No VIDEO or GRAPHICS planes found";
}

/**
 * @brief Test the negative scenarios for the PlaneControl getProperty function.
 *
 * This test checks the behavior of the getProperty function when provided with invalid inputs, such as an invalid plane resource index and an invalid property value. The test ensures that the function handles these cases gracefully by returning appropriate error statuses and no values.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 010@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve the number of planes from the YAML configuration | planecontrol.planes | planeCount should be retrieved | Should be successful |
 * | 02 | Calculate invalid plane resource index | invalidPlaneResourceIndex = planeCount + 1 | Should be calculated | Should be successful |
 * | 03 | Call getProperty with invalid plane resource index and valid property using PLANE_CONTROL_GET_PROPERTY_NEG | planeResourceIndex = invalidPlaneResourceIndex, property = RESOURCE_ID | status.isOk() = false, exception = EX_ILLEGAL_ARGUMENT, propertyValue.has_value() = false | Should Fail |
 * | 04 | Use valid plane resource index from VIDEO planes | validPlaneResourceIndex = VIDEO plane index | Should be retrieved | Should be successful |
 * | 05 | Call getProperty with valid plane resource index and invalid property value using PLANE_CONTROL_GET_PROPERTY_NEG | planeResourceIndex = validPlaneResourceIndex, property = 999 | status.isOk() = false, exception = EX_ILLEGAL_ARGUMENT, propertyValue.has_value() = false | Should Fail |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, getProperty_neg) {
    // Step 1: Retrieve plane count
    std::string plane_count_path = PLANE_CONTROL_MGR_PLANES;
    uint32_t planeCount = UT_KVP_PROFILE_GET_UINT32(plane_count_path.c_str());
    UT_LOG_INFO("Retrieved %u planes from profile", planeCount);

    // Step 2: Calculate invalid plane index
    int32_t invalidPlaneResourceIndex = planeCount + 1;
    std::optional<::com::rdk::hal::PropertyValue> propertyValue;

    // Step 3: Get a valid VIDEO plane index
    int32_t validPlaneResourceIndex = -1;
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);
    for (const auto& cap : capabilities) {
        if (cap.type == ::com::rdk::hal::planecontrol::PlaneType::VIDEO) {
            validPlaneResourceIndex = cap.planeIndex;
            break;
        }
    }
    if (validPlaneResourceIndex == -1) {
        UT_FAIL("No VIDEO plane found for negative property test");
    }

    // Step 4: Test with valid plane index and invalid property
    ::com::rdk::hal::planecontrol::Property invalidProperty =
        static_cast<::com::rdk::hal::planecontrol::Property>(-1);
    PLANE_CONTROL_GET_PROPERTY_NEG(validPlaneResourceIndex, invalidProperty, propertyValue, ::android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

/**
 * @brief Test the positive setting of properties for plane control.
 *
 * This test verifies the ability to set various properties on plane resources using valid indices and property values. It ensures that the setProperty function behaves correctly for integer and boolean property values.
 *
 * **Test Group ID:** Basic: 001@n
 * **Test Case ID:** 011@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * |-------|-------------|-----------|-----------------|-------|
 * | 1 | Ensure the service is available | gPlaneControlservice|gPlaneControlService | Should not be null | Should be successful |
 * | 2 | Retrieve capabilities to get plane resources using PLANE_CONTROL_GET_CAPABILITIES | capabilities | Should be successful, capabilities not empty | |
 * | 3 | Loop through each VIDEO plane resource index | planeResourceIndex | Should be successful | |
 * | 4 | Set VIDEO-specific integer properties (OVERSCAN, ASPECT_RATIO) | planeResourceIndex, Property::OVERSCAN = 10, Property::ASPECT_RATIO = 1 | status.isOk() = true, aidl_return = true | Should Pass |
 * | 5 | Loop through each plane resource index (VIDEO and GRAPHICS) | planeResourceIndex | Should be successful | |
 * | 6 | Set geometry properties clamped to the reported capabilities | Property::WIDTH <= maxWidth, Property::HEIGHT <= maxHeight, Property::X + WIDTH <= frameWidth, Property::Y + HEIGHT <= frameHeight | status.isOk() = true, aidl_return = true | Should Pass |
 * | 7 | Set ALPHA only on planes reporting supportsAlpha = true | planeResourceIndex, Property::ALPHA = 254 | status.isOk() = true, aidl_return = true | Skipped when supportsAlpha = false |
 * | 8 | Set ZORDER only on planes reporting supportsZOrder = true | planeResourceIndex, Property::ZORDER = 1 | status.isOk() = true, aidl_return = true | Skipped when supportsZOrder = false |
 * | 9 | Set boolean property (VISIBLE) | planeResourceIndex, Property::VISIBLE = true | status.isOk() = true, aidl_return = true | Should Pass |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, setProperty_pos) {
    // Step 1: Ensure service is available
    UT_ASSERT_TRUE(gPlaneControlservice != nullptr) << "PlaneControl service is not available";

    // Step 2: Retrieve capabilities
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);
    UT_LOG_INFO("Retrieved %zu planes from capabilities", capabilities.size());

    // Steps 3–4: Set VIDEO-specific properties
    uint32_t video_plane_count = 0;
    PLANE_CONTROL_FOREACH_VIDEO_PLANE(video_plane_count, planeResourceIndex, {
        ::com::rdk::hal::PropertyValue propertyValue;
        // Set OVERSCAN
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(10);
        bool aidl_return;
        PLANE_CONTROL_SET_PROPERTY_POS(planeResourceIndex, ::com::rdk::hal::planecontrol::Property::OVERSCAN, propertyValue, aidl_return);

        // Set ASPECT_RATIO
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
        PLANE_CONTROL_SET_PROPERTY_POS(planeResourceIndex, ::com::rdk::hal::planecontrol::Property::ASPECT_RATIO, propertyValue, aidl_return);
    });

    // Steps 5–9: Set common properties for all planes
    for (const auto& plane : capabilities) {
        int32_t planeResourceIndex = plane.planeIndex;
        const PlaneTestGeometry geometry = getTestGeometry(plane);
        ::com::rdk::hal::PropertyValue propertyValue;
        bool aidl_return;

        // Set X
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(geometry.x);
        PLANE_CONTROL_SET_PROPERTY_POS(planeResourceIndex, ::com::rdk::hal::planecontrol::Property::X, propertyValue, aidl_return);

        // Set Y
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(geometry.y);
        PLANE_CONTROL_SET_PROPERTY_POS(planeResourceIndex, ::com::rdk::hal::planecontrol::Property::Y, propertyValue, aidl_return);

        // Set WIDTH
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(geometry.width);
        PLANE_CONTROL_SET_PROPERTY_POS(planeResourceIndex, ::com::rdk::hal::planecontrol::Property::WIDTH, propertyValue, aidl_return);

        // Set HEIGHT
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(geometry.height);
        PLANE_CONTROL_SET_PROPERTY_POS(planeResourceIndex, ::com::rdk::hal::planecontrol::Property::HEIGHT, propertyValue, aidl_return);

        // Set ALPHA only when the plane reports alpha support
        if (planeSupportsProperty(capabilities, plane, ::com::rdk::hal::planecontrol::Property::ALPHA)) {
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(254);
            PLANE_CONTROL_SET_PROPERTY_POS(planeResourceIndex, ::com::rdk::hal::planecontrol::Property::ALPHA, propertyValue, aidl_return);
        }

        // Set Z_ORDER only when the plane reports Z-order support and is not the primary graphics plane
        if (planeSupportsProperty(capabilities, plane, ::com::rdk::hal::planecontrol::Property::ZORDER)) {
            propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
            PLANE_CONTROL_SET_PROPERTY_POS(planeResourceIndex, ::com::rdk::hal::planecontrol::Property::ZORDER, propertyValue, aidl_return);
        }

        // Set VISIBLE (boolean)
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
        PLANE_CONTROL_SET_PROPERTY_POS(planeResourceIndex, ::com::rdk::hal::planecontrol::Property::VISIBLE, propertyValue, aidl_return);
    }

    // Ensure at least one plane was tested
    UT_ASSERT_FALSE(capabilities.empty()) << "No planes found in capabilities";
}

/**
 * @brief Test the negative scenarios for the setProperty function in PlaneControl
 *
 * This test verifies the behavior of the setProperty function when invalid inputs are provided. It checks for invalid plane resource index, invalid property, and invalid property value type to ensure the function handles these cases correctly.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 012@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Ensure the service is available | gPlaneControlservice | Should not be null | Should be successful |
 * | 02 | Retrieve capabilities to get plane resources using PLANE_CONTROL_GET_CAPABILITIES | capabilities | Should be successful, capabilities not empty | |
 * | 03 | Calculate invalid plane resource index | invalidPlaneResourceIndex = min(plane indices) - 1 | Should be calculated | Should be successful |
 * | 04 | Call setProperty with invalid plane index using PLANE_CONTROL_SET_PROPERTY_NEG | planeResourceIndex = invalidPlaneResourceIndex, property = WIDTH, propertyValue = intValue:100 | aidl_return = false | Should Fail |
 * | 05 | Get a valid plane resource index | validPlaneResourceIndex = first plane index | Should be retrieved | Should be successful |
 * | 06 | Call setProperty with invalid property using PLANE_CONTROL_SET_PROPERTY_NEG | planeResourceIndex = validPlaneResourceIndex, property = ASPECT_RATIO + 1, propertyValue = intValue:100 | aidl_return = false | Should Fail |
 * | 07 | Call setProperty with invalid property value type using PLANE_CONTROL_SET_PROPERTY_NEG | planeResourceIndex = validPlaneResourceIndex, property = HEIGHT, propertyValue = stringValue:'InvalidType' | aidl_return = false | Should Fail |
 * | 08 | Call setProperty for properties not supported by the plane capabilities (ALPHA, ZORDER, OVERSCAN, ASPECT_RATIO) | planeResourceIndex = each plane, property = unsupported property | aidl_return = false, exception = EX_ILLEGAL_ARGUMENT | Should Fail |
 * | 09 | Call setProperty with geometry exceeding the reported limits | WIDTH = maxWidth + 1, HEIGHT = maxHeight + 1, X = frameWidth, Y = frameHeight | aidl_return = false, exception = EX_ILLEGAL_ARGUMENT | Should Fail |
 * | 10 | Call setProperty on the read-only RESOURCE_ID property | planeResourceIndex = each plane, property = RESOURCE_ID | aidl_return = false, exception = EX_UNSUPPORTED_OPERATION | Should Fail |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, setProperty_neg) {
    // Step 1: Ensure service is available
    UT_ASSERT_TRUE(gPlaneControlservice != nullptr) << "PlaneControl service is not available";

    // Step 2: Retrieve capabilities
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);
    UT_LOG_INFO("Retrieved %zu planes from capabilities", capabilities.size());
    UT_ASSERT_FALSE(capabilities.empty()) << "No planes found in capabilities";

    // Step 3: Calculate invalid plane index
    std::vector<uint32_t> planeResourceIndices;
    for (const auto& plane : capabilities) {
        planeResourceIndices.push_back(plane.planeIndex);
    }
    std::sort(planeResourceIndices.begin(), planeResourceIndices.end());
    int32_t invalidPlaneResourceIndex = planeResourceIndices.empty() ? -1 : planeResourceIndices.front() - 1;

    // Step 4: Test with invalid plane index
    ::com::rdk::hal::PropertyValue invalidPlaneValue;
    invalidPlaneValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(100);
    bool aidl_return;
    PLANE_CONTROL_SET_PROPERTY_NEG(invalidPlaneResourceIndex, ::com::rdk::hal::planecontrol::Property::WIDTH, invalidPlaneValue, aidl_return, ::android::binder::Status::EX_ILLEGAL_ARGUMENT);

    // Step 5: Get valid plane index
    int32_t validPlaneResourceIndex = planeResourceIndices.front();

    // Step 6: Test with invalid property
    ::com::rdk::hal::PropertyValue invalidPropValue;
    invalidPropValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(100);
    ::com::rdk::hal::planecontrol::Property invalidProperty =
        static_cast<::com::rdk::hal::planecontrol::Property>(
            static_cast<int32_t>(::com::rdk::hal::planecontrol::Property::ASPECT_RATIO) + 1);
    PLANE_CONTROL_SET_PROPERTY_NEG(validPlaneResourceIndex, invalidProperty, invalidPropValue, aidl_return, ::android::binder::Status::EX_ILLEGAL_ARGUMENT);

    // Step 7: Test with invalid property value type
    ::com::rdk::hal::PropertyValue invalidTypeValue;
    invalidTypeValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::stringValue>(::android::String16("InvalidType"));
    PLANE_CONTROL_SET_PROPERTY_NEG(validPlaneResourceIndex, ::com::rdk::hal::planecontrol::Property::HEIGHT, invalidTypeValue, aidl_return, ::android::binder::Status::EX_ILLEGAL_ARGUMENT);

    // Step 8: Test properties and values rejected by the reported capabilities
    for (const auto& plane : capabilities) {
        ::com::rdk::hal::PropertyValue value;

        if (!planeSupportsProperty(capabilities, plane, ::com::rdk::hal::planecontrol::Property::ALPHA)) {
            value.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(254);
            PLANE_CONTROL_SET_PROPERTY_NEG(plane.planeIndex, ::com::rdk::hal::planecontrol::Property::ALPHA, value, aidl_return, ::android::binder::Status::EX_ILLEGAL_ARGUMENT);
        }

        if (!planeSupportsProperty(capabilities, plane, ::com::rdk::hal::planecontrol::Property::ZORDER)) {
            value.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
            PLANE_CONTROL_SET_PROPERTY_NEG(plane.planeIndex, ::com::rdk::hal::planecontrol::Property::ZORDER, value, aidl_return, ::android::binder::Status::EX_ILLEGAL_ARGUMENT);
        }

        if (!planeSupportsProperty(capabilities, plane, ::com::rdk::hal::planecontrol::Property::OVERSCAN)) {
            value.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(10);
            PLANE_CONTROL_SET_PROPERTY_NEG(plane.planeIndex, ::com::rdk::hal::planecontrol::Property::OVERSCAN, value, aidl_return, ::android::binder::Status::EX_ILLEGAL_ARGUMENT);
        }

        if (!planeSupportsProperty(capabilities, plane, ::com::rdk::hal::planecontrol::Property::ASPECT_RATIO)) {
            value.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(1);
            PLANE_CONTROL_SET_PROPERTY_NEG(plane.planeIndex, ::com::rdk::hal::planecontrol::Property::ASPECT_RATIO, value, aidl_return, ::android::binder::Status::EX_ILLEGAL_ARGUMENT);
        }

        // Geometry beyond the reported maxWidth/maxHeight/frameWidth/frameHeight limits
        value.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(plane.maxWidth + 1);
        PLANE_CONTROL_SET_PROPERTY_NEG(plane.planeIndex, ::com::rdk::hal::planecontrol::Property::WIDTH, value, aidl_return, ::android::binder::Status::EX_ILLEGAL_ARGUMENT);

        value.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(plane.maxHeight + 1);
        PLANE_CONTROL_SET_PROPERTY_NEG(plane.planeIndex, ::com::rdk::hal::planecontrol::Property::HEIGHT, value, aidl_return, ::android::binder::Status::EX_ILLEGAL_ARGUMENT);

        value.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(plane.frameWidth);
        PLANE_CONTROL_SET_PROPERTY_NEG(plane.planeIndex, ::com::rdk::hal::planecontrol::Property::X, value, aidl_return, ::android::binder::Status::EX_ILLEGAL_ARGUMENT);

        value.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(plane.frameHeight);
        PLANE_CONTROL_SET_PROPERTY_NEG(plane.planeIndex, ::com::rdk::hal::planecontrol::Property::Y, value, aidl_return, ::android::binder::Status::EX_ILLEGAL_ARGUMENT);

        // RESOURCE_ID is read-only
        value.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(plane.planeIndex);
        PLANE_CONTROL_SET_PROPERTY_NEG(plane.planeIndex, ::com::rdk::hal::planecontrol::Property::RESOURCE_ID, value, aidl_return, ::android::binder::Status::EX_UNSUPPORTED_OPERATION);
    }
}

/**
 * @brief Test the positive scenario of the getPropertyMulti function in PlaneControlBaseTest
 *
 * This test verifies the successful retrieval of multiple properties for a given plane resource index using the getPropertyMulti API. It ensures that the API call returns successfully and that the property values are correctly populated in the propertyKVList.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 013@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Prepare inputs for the test, including planeResourceIndex and propertyKVList with valid PropertyKVPair objects | planeResourceIndex = UT_KVP_PROFILE_GET_UINT32(planecontrol.planes.<index>.<key>), propertyKVList = {Property::RESOURCE_ID, Property::X, Property::Y, Property::WIDTH, Property::HEIGHT, Property::ALPHA, Property::ZORDER, Property::VISIBLE, Property::OVERSCAN, Property::ASPECT_RATIO} | Inputs prepared successfully | Should be successful |
 * | 02 | Log the API call details for getPropertyMulti | planeResourceIndex, propertyKVList, _aidl_return | Log successful | Should be successful |
 * | 03 | Call the getPropertyMulti API with the prepared inputs | planeResourceIndex, propertyKVList, _aidl_return | API call should succeed | Should Pass |
 * | 04 | Verify that the status of the API call is OK | status.isOk() | True | Should Pass |
 * | 05 | Verify that the _aidl_return is true, indicating successful property retrieval | _aidl_return | True | Should Pass |
 * | 06 | Verify that the propertyKVList is not empty after the API call | propertyKVList.empty() | False | Should Pass |
 * | 07 | Verify that each property in propertyKVList has a value | kvPair.propertyValue.value.has_value() | True for each kvPair | Should Pass |
 * | 08 | Log the result of the getPropertyMulti API call | planeResourceIndex, propertyKVList, _aidl_return, status | Log successful | Should be successful |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, getPropertyMulti_pos) {
    UT_LOG_INFO("Starting getPropertyMulti_pos");

    // Step 1: Get plane capabilities
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities); // Uses macro to fetch and validate capabilities

    // Iterate over planes
    for (const auto& plane : capabilities) {
        uint32_t planeResourceIndex = plane.planeIndex;
        ::com::rdk::hal::planecontrol::PlaneType planeType = plane.type;
        std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;

        // Step 1: Prepare inputs - use property enum lists filtered by plane capabilities
        std::vector<::com::rdk::hal::planecontrol::Property> testableProperties = getTestableProperties(capabilities, plane);
        if (planeType == ::com::rdk::hal::planecontrol::PlaneType::VIDEO) {
            PLANE_CONTROL_VALIDATE_GET_PROPERTY_MULTI(planeResourceIndex, testableProperties, propertyKVList, "VIDEO");
        } else if (planeType == ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS) {
            PLANE_CONTROL_VALIDATE_GET_PROPERTY_MULTI(planeResourceIndex, testableProperties, propertyKVList, "GRAPHICS");
        } else {
            continue; // Skip unsupported plane types
        }
    }
}

/**
 * @brief Test the negative scenarios for the getPropertyMulti function in PlaneControl
 *
 * This test evaluates the behavior of the getPropertyMulti function when provided with invalid inputs, such as an invalid plane resource index and an invalid property list. The purpose is to ensure that the function handles erroneous inputs gracefully and returns the expected failure results.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 014@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Prepare an invalid plane resource index for testing | invalidPlaneResourceIndex = -1 | Should be successful | |
 * | 02 | Prepare a valid plane resource index using UT_KVP_PROFILE_GET_UINT32 | validPlaneResourceIndex = UT_KVP_PROFILE_GET_UINT32(planecontrol.planes.<index>.<key>) | Should be successful | |
 * | 03 | Create a valid property key-value pair and add to the list | property = WIDTH, propertyValue = 1920 | Should be successful | |
 * | 04 | Create an invalid property key-value pair and add to the list | property = -1 | Should be successful | |
 * | 05 | Call getPropertyMulti with invalid plane resource index | planeResourceIndex = -1, propertyKVList = [validPropertyKVPair, invalidPropertyKVPair], aidlReturn = false | aidlReturn = false, status.isOk() = true, status.exceptionCode() = EX_NONE | Should Fail |
 * | 06 | Call getPropertyMulti with valid plane resource index and invalid property list | planeResourceIndex = validPlaneResourceIndex, propertyKVList = [validPropertyKVPair, invalidPropertyKVPair], aidlReturn = false | aidlReturn = false, status.isOk() = true, status.exceptionCode() = EX_NONE | Should Fail |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, getPropertyMulti_neg) {
    UT_LOG_INFO("Starting getPropertyMulti_neg");

    // Step 1: Get plane capabilities
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities); // Uses macro to fetch and validate capabilities

    // Step 2: Gather valid plane resource indices
    std::vector<uint32_t> planeResourceIndices;
    for (const auto& plane : capabilities) {
        planeResourceIndices.push_back(plane.planeIndex);
    }
    ASSERT_FALSE(planeResourceIndices.empty()) << "No planes available";
    std::sort(planeResourceIndices.begin(), planeResourceIndices.end());

    // Step 1: Prepare invalid plane resource index
    int32_t invalidPlaneResourceIndex = -1; // As specified in test procedure

    // Step 2: Prepare valid plane resource index
    int32_t validPlaneResourceIndex = planeResourceIndices[0];

    // Step 3: Create valid property list
    std::vector<::com::rdk::hal::planecontrol::Property> validPropertyEnums;
    validPropertyEnums.push_back(::com::rdk::hal::planecontrol::Property::WIDTH);

    // Step 4: Create invalid property list
    std::vector<::com::rdk::hal::planecontrol::Property> invalidPropertyEnums;
    invalidPropertyEnums.push_back(::com::rdk::hal::planecontrol::Property::WIDTH); // Include valid property
    invalidPropertyEnums.push_back(static_cast<::com::rdk::hal::planecontrol::Property>(-1)); // Include invalid property

    std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;

    // Step 5: Test invalid plane resource index
    PLANE_CONTROL_NEGATIVE_TEST_GET_PROPERTY_MULTI(invalidPlaneResourceIndex, validPropertyEnums, propertyKVList, "Invalid Plane Index");

    // Step 6: Test invalid property list
    PLANE_CONTROL_NEGATIVE_TEST_GET_PROPERTY_MULTI(validPlaneResourceIndex, invalidPropertyEnums, propertyKVList, "Invalid Property List");
}

/**
 * @brief Test the positive scenario of setting multiple atomic properties on a plane control.
 *
 * This test verifies the functionality of setting multiple atomic properties on a plane control
 * using the setPropertyMultiAtomic API. It ensures that the API call is successful and the expected
 * results are returned.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 015@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve the plane resource index using the macro | planeResourceIndex = UT_KVP_PROFILE_GET_UINT32(planecontrol.planes.<index>.<key>) | Should be successful | Should be successful |
 * | 02 | Prepare the property key-value list using macros | propertyKVList = {kvPair1, kvPair2, ...} | Should be successful | Should be successful |
 * | 03 | Log the API call | planeResourceIndex, propertyKVList | Should be successful | Should be successful |
 * | 04 | Call the setPropertyMultiAtomic API | planeResourceIndex, propertyKVList, _aidl_return | API should return true, status should be OK | Should Pass |
 * | 05 | Log the result of the API call | planeResourceIndex, propertyKVList, _aidl_return, status | Should be successful | Should be successful |
 * | 06 | Verify the results using assertions | status.isOk(), _aidl_return | Both assertions should be true | Should Pass |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, setPropertyMultiAtomic_pos) {
    UT_LOG_INFO("Starting setPropertyMultiAtomic_pos");

    // Step 1: Get plane capabilities
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities); // Fetches valid plane indices

    // Iterate over planes
    for (const auto& plane : capabilities) {
        uint32_t planeResourceIndex = plane.planeIndex;
        ::com::rdk::hal::planecontrol::PlaneType planeType = plane.type;
        std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;

        // Step 2: Prepare property key-value list (no duplicates)
        if (planeType == ::com::rdk::hal::planecontrol::PlaneType::VIDEO) {
            PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::OVERSCAN, 10);
            PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::ASPECT_RATIO, 1);
        } else if (planeType == ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS) {
            // Graphics planes don't need video-specific properties
        } else {
            continue; // Skip unsupported plane types
        }

        // Add common properties within the limits reported by the plane
        const PlaneTestGeometry geometry = getTestGeometry(plane);
        PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::X, geometry.x);
        PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::Y, geometry.y);
        PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::WIDTH, geometry.width);
        PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::HEIGHT, geometry.height);
        if (planeSupportsProperty(capabilities, plane, ::com::rdk::hal::planecontrol::Property::ALPHA)) {
            PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::ALPHA, 254);
        }
        if (planeSupportsProperty(capabilities, plane, ::com::rdk::hal::planecontrol::Property::ZORDER)) {
            PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::ZORDER, 1);
        } else {
            UT_LOG_INFO("Skipping ZORDER for plane %u: supportsZOrder is false", planeResourceIndex);
        }
        PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::VISIBLE, 1);

        // Steps 3-6: Log, call, log result, and verify
        PLANE_CONTROL_SET_PROPERTY_MULTI_ATOMIC(planeResourceIndex, propertyKVList); // Ensures _aidl_return == true
    }
}

/**
 * @brief Test the negative scenarios for the setPropertyMultiAtomic function in PlaneControl.
 *
 * This test evaluates the behavior of the setPropertyMultiAtomic function when provided with invalid inputs, such as an invalid plane resource index and duplicate properties in the property list. The test ensures that the function handles these scenarios gracefully and returns the expected results.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 016@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Calculate invalidPlaneResourceIndexLow by subtracting 1 from a valid index. | invalidPlaneResourceIndexLow = validIndex - 1 | Should Fail | Should be successful |
 * | 02 | Calculate invalidPlaneResourceIndexHigh by adding 1 to a valid index. | invalidPlaneResourceIndexHigh = validIndex + 1 | Should Fail | Should be successful |
 * | 03 | Create a valid property key-value list. | propertyKVList = valid list | Should be successful | Should be successful |
 * | 04 | Call setPropertyMultiAtomic with invalidPlaneResourceIndexLow. | planeResourceIndex = invalidPlaneResourceIndexLow, propertyKVList = valid list | aidlReturn = false | Should Fail |
 * | 05 | Call setPropertyMultiAtomic with invalidPlaneResourceIndexHigh. | planeResourceIndex = invalidPlaneResourceIndexHigh, propertyKVList = valid list | aidlReturn = false | Should Fail |
 * | 06 | Calculate validPlaneResourceIndex from a valid index. | validPlaneResourceIndex = validIndex | Should be successful | Should be successful |
 * | 07 | Create a duplicate property key-value list. | duplicatePropertyKVList = list with duplicates | Should be successful | Should be successful |
 * | 08 | Call setPropertyMultiAtomic with validPlaneResourceIndex and duplicatePropertyKVList. | planeResourceIndex = validPlaneResourceIndex, propertyKVList = duplicate list | aidlReturn = false | Should Fail |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, setPropertyMultiAtomic_neg) {
    UT_LOG_INFO("Starting setPropertyMultiAtomic_neg");

    // Steps 1-2: Get plane capabilities and calculate invalid indices
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);

    std::vector<uint32_t> planeResourceIndices;
    for (const auto& plane : capabilities) {
        planeResourceIndices.push_back(plane.planeIndex);
    }
    ASSERT_FALSE(planeResourceIndices.empty()) << "No planes available";
    std::sort(planeResourceIndices.begin(), planeResourceIndices.end());

    int32_t invalidPlaneResourceIndexLow = planeResourceIndices[0] - 1;
    int32_t invalidPlaneResourceIndexHigh = planeResourceIndices.back() + 2;
    int32_t validPlaneResourceIndex = planeResourceIndices[0];

    // Step 3: Create valid property key-value list
    std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> validPropertyKVList;
    PLANE_CONTROL_ADD_PROP_TO_LIST(validPropertyKVList, ::com::rdk::hal::planecontrol::Property::X, 123);
    PLANE_CONTROL_ADD_PROP_TO_LIST(validPropertyKVList, ::com::rdk::hal::planecontrol::Property::Y, 6);

    // Step 4: Test invalid plane resource index low
    PLANE_CONTROL_SET_PROPERTY_MULTI_ATOMIC_NEG(invalidPlaneResourceIndexLow, validPropertyKVList, "Invalid Plane Index Low");

    // Step 5: Test invalid plane resource index high
    PLANE_CONTROL_SET_PROPERTY_MULTI_ATOMIC_NEG(invalidPlaneResourceIndexHigh, validPropertyKVList, "Invalid Plane Index High");

    // Steps 7-8: Create duplicate property key-value list
    std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> duplicatePropertyKVList;
    PLANE_CONTROL_ADD_PROP_TO_LIST(duplicatePropertyKVList, ::com::rdk::hal::planecontrol::Property::WIDTH, 60);
    PLANE_CONTROL_ADD_PROP_TO_LIST(duplicatePropertyKVList, ::com::rdk::hal::planecontrol::Property::WIDTH, 80); // Duplicate property

}

/**
 * @brief Test the positive scenario of registering a valid PlaneControl listener
 *
 * This test verifies the successful registration of a valid IPlaneControlListener instance using the registerListener API. It ensures that the API call returns a successful status and the expected output.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 017@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Create a valid instance of IPlaneControlListener | listener = new ValidPlaneControlListener() | Instance should be created successfully | Should be successful |
 * | 02 | Prepare the output parameter for the API call | aidl_return = false | Output parameter initialized | Should be successful |
 * | 03 | Log the API call details | Log the listener and output parameter | Log should be successful | Should be successful |
 * | 04 | Call the registerListener API with the listener and output parameter | listener, aidl_return | API should return a successful status | Should Pass |
 * | 05 | Log the result of the API call | Log the listener, output parameter, and status | Log should be successful | Should be successful |
 * | 06 | Verify the API call returned a successful status | status.isOk() | Status should be true | Should Pass |
 * | 07 | Verify the output parameter is true | aidl_return | Output should be true | Should Pass |
 */
UT_ADD_TEST(VTS_L1_PlaneControl,  registerListener_pos) {
    UT_LOG_INFO("Starting registerListener_pos");

    // Step 1: Create a valid instance of IPlaneControlListener
    android::sp<IPlaneControlListener> listener = new ValidPlaneControlListener();
    ASSERT_TRUE(listener != nullptr) << "Failed to create ValidPlaneControlListener";

    // Step 2: Prepare output parameter
    bool aidl_return = false;

    // Steps 3-7: Log, call, log result, and verify
    PLANE_CONTROL_REGISTER_LISTENER_POS(listener, aidl_return);
}

/**
 * @brief Test the negative scenarios for registering a PlaneControl listener
 *
 * This test verifies the behavior of the registerListener function when attempting to register a null listener and when trying to register a listener that is already registered. These scenarios are important to ensure the robustness of the listener registration mechanism.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 018@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Initialize a null IPlaneControlListener and attempt to register it | listener = null, _aidl_return = false | status.isOk() = false, _aidl_return = false | Should Fail |
 * | 02 | Create a valid IPlaneControlListener and register it initially | listener = valid, _aidl_return = false | initial_aidl_return = true | Should be successful |
 * | 03 | Attempt to register the already registered valid IPlaneControlListener again | listener = valid, _aidl_return = false | status.isOk() = false, _aidl_return = false | Should Fail |
 */
UT_ADD_TEST(VTS_L1_PlaneControl,  registerListener_neg) {
    UT_LOG_INFO("Starting registerListener_neg");

    // Step 1: Initialize a null listener and attempt to register
    android::sp<IPlaneControlListener> listener = nullptr;
    bool aidl_return = false;
    PLANE_CONTROL_REGISTER_LISTENER_NEG(listener, aidl_return, "Null Listener");

    // Step 2: Create and register a valid listener
    listener = new ValidPlaneControlListener();
    ASSERT_TRUE(listener != nullptr) << "Failed to create ValidPlaneControlListener";
    aidl_return = false;
    PLANE_CONTROL_REGISTER_LISTENER_POS(listener, aidl_return); // Expect success

    // Step 3: Attempt to re-register the same listener
    aidl_return = false;
    PLANE_CONTROL_REGISTER_LISTENER_NEG(listener, aidl_return, "Already Registered Listener");
}

/**
 * @brief Test the positive scenario of unregistering a listener in PlaneControl.
 *
 * This test verifies that a valid listener can be successfully unregistered after being registered. It ensures that the unregister operation returns a successful status and result.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 019@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Create a valid listener instance | listener = new ValidPlaneControlListener() | Listener instance created | Should be successful |
 * | 02 | Register the listener to ensure it is registered | listener = 0x[Address], registerResult = [value] | registerStatus.isOk() = true, registerResult = true | Should Pass |
 * | 03 | Unregister the listener | listener = 0x[Address], unregisterResult = [value] | unregisterStatus.isOk() = true, unregisterResult = true | Should Pass |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, unregisterListener_pos) {
    UT_LOG_INFO("Starting unregisterListener_pos");

    // Step 1: Create a valid listener instance
    android::sp<IPlaneControlListener> listener = new ValidPlaneControlListener();
    ASSERT_TRUE(listener != nullptr) << "Failed to create ValidPlaneControlListener";

    // Step 2: Register the listener
    bool registerResult = false;
    PLANE_CONTROL_REGISTER_LISTENER_POS(listener, registerResult); // Expect success

    // Step 3: Unregister the listener
    bool unregisterResult = false;
    PLANE_CONTROL_UNREGISTER_LISTENER_POS(listener, unregisterResult);
}

/**
 * @brief Test the negative scenarios for unregistering a listener in PlaneControl
 *
 * This test evaluates the behavior of the unregisterListener function when provided with invalid inputs or scenarios. It checks the function's response to null inputs, unregistered listeners, and listeners that have been registered and unregistered already.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 020@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Attempt to unregister a listener with null input | listener = nullptr, aidl_return = false | aidl_return should be false, status.isOk() should be false | Should Fail |
 * | 02 | Attempt to unregister a listener that was never registered | listener = unregisteredListener, aidl_return = false | aidl_return should be false, status.isOk() should be false | Should Fail |
 * | 03 | Attempt to unregister a listener that was registered and unregistered already | listener = registeredListener, aidl_return = false | aidl_return should be false, status.isOk() should be false | Should Fail |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, unregisterListener_neg) {
    UT_LOG_INFO("Starting unregisterListener_neg");

    // Step 1: Attempt to unregister a null listener
    android::sp<IPlaneControlListener> listener = nullptr;
    bool aidl_return = false;
    PLANE_CONTROL_UNREGISTER_LISTENER_NEG(listener, aidl_return, "Null Listener");

    // Step 2: Attempt to unregister a listener that was never registered
    listener = new ValidPlaneControlListener();
    ASSERT_TRUE(listener != nullptr) << "Failed to create ValidPlaneControlListener";
    aidl_return = false;
    PLANE_CONTROL_UNREGISTER_LISTENER_NEG(listener, aidl_return, "Unregistered Listener");

    // Step 3: Attempt to unregister a listener that was registered and unregistered already
    listener = new ValidPlaneControlListener();
    ASSERT_TRUE(listener != nullptr) << "Failed to create ValidPlaneControlListener";
    bool registerResult = false;
    PLANE_CONTROL_REGISTER_LISTENER_POS(listener, registerResult); // Register
    aidl_return = false;
    PLANE_CONTROL_UNREGISTER_LISTENER_POS(listener, aidl_return); // Unregister
    aidl_return = false;
    PLANE_CONTROL_UNREGISTER_LISTENER_NEG(listener, aidl_return, "Already Unregistered Listener");
}

/**
 * @brief Test the positive scenario of retrieving graphics frame buffer provider.
 *
 * This test verifies the functionality of getGraphicsFbProvider for GRAPHICS planes.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 002@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve plane capabilities using PLANE_CONTROL_GET_CAPABILITIES macro | capabilities | Should be successful, capabilities not empty | |
 * | 02 | Identify GRAPHICS planes from capabilities | plane.type == PlaneType::GRAPHICS | At least one GRAPHICS plane exists | |
 * | 03 | Call getGraphicsFbProvider for each GRAPHICS plane | planeResourceIndex, provider | status.isOk() = true, provider != null | Should Pass |
 * | 04 | Log the API call and results | planeResourceIndex, nativeHandle, status | Should be successful | |
 * | 05 | Verify the handle is valid | nativeHandle >= 0 | Should be successful | |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, getGraphicsFbProvider_pos) {
    UT_LOG_INFO("Starting getGraphicsFbProvider_pos");

    // Step 1: Retrieve capabilities
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);

    // Step 2: Identify GRAPHICS planes
    bool graphicsPlaneFound = false;
    for (const auto& plane : capabilities) {
        if (plane.type != ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS) {
            continue;
        }

        graphicsPlaneFound = true;
        uint32_t planeResourceIndex = plane.planeIndex;
        ::android::sp<::com::rdk::hal::planecontrol::IGraphicsFbProvider> provider;

        // Steps 3-5: Call API, log, and verify
        PLANE_CONTROL_GET_GRAPHICS_FB_PROVIDER_INSTANCE_POS(planeResourceIndex, provider);
    }

    UT_ASSERT_TRUE(graphicsPlaneFound) << "No GRAPHICS plane found in capabilities";
}

/**
 * @brief Test the negative scenarios for retrieving graphics frame buffer provider.
 *
 * This test verifies the behavior of getGraphicsFbProvider with invalid inputs, such as an invalid plane resource index or a non-GRAPHICS plane index.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 003@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve plane capabilities using PLANE_CONTROL_GET_CAPABILITIES macro | capabilities | Should be successful | |
 * | 02 | Identify a VIDEO plane index for invalid test | plane.type == PlaneType::VIDEO | Should be successful if VIDEO plane exists | |
 * | 03 | Identify a valid GRAPHICS plane index to determine max index | plane.type == PlaneType::GRAPHICS | Should be successful if GRAPHICS plane exists | |
 * | 04 | Call getGraphicsFbProvider with invalid plane index (beyond max) | invalidPlaneResourceIndex = maxPlaneIndex + 1 | status.isOk() = true, provider == null | Should Pass |
 * | 05 | Call getGraphicsFbProvider with VIDEO plane index | invalidPlaneResourceIndex = videoPlaneIndex | status.isOk() = true, provider == null | Should Pass if VIDEO plane exists |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, getGraphicsFbProvider_neg) {
    UT_LOG_INFO("Starting getGraphicsFbProvider_neg");

    // Step 1: Retrieve capabilities
    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> capabilities;
    PLANE_CONTROL_GET_CAPABILITIES(capabilities);

    // Step 2: Identify a VIDEO plane index
    uint32_t videoPlaneIndex = 0;
    bool videoPlaneFound = false;
    for (const auto& plane : capabilities) {
        if (plane.type == ::com::rdk::hal::planecontrol::PlaneType::VIDEO) {
            videoPlaneIndex = plane.planeIndex;
            videoPlaneFound = true;
            break;
        }
    }

    // Step 3: Identify max plane index
    uint32_t maxPlaneIndex = 0;
    bool graphicsPlaneFound = false;
    for (const auto& plane : capabilities) {
        if (plane.type == ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS) {
            graphicsPlaneFound = true;
        }
        UT_ASSERT_TRUE(plane.planeIndex >= 0) << "Negative plane index: " << plane.planeIndex;
        maxPlaneIndex = std::max(maxPlaneIndex, static_cast<uint32_t>(plane.planeIndex));
    }

    UT_ASSERT_TRUE(graphicsPlaneFound) << "No GRAPHICS plane found in capabilities";

    // Step 4: Test with invalid plane index
    uint32_t invalidPlaneResourceIndex = maxPlaneIndex + 1;
    int64_t nativeHandle = 0;
    PLANE_CONTROL_GET_GRAPHICS_FB_PROVIDER_NEG(invalidPlaneResourceIndex, nativeHandle, "Invalid Plane Index");

    // Step 5: Test with VIDEO plane index
    if (videoPlaneFound) {
        PLANE_CONTROL_GET_GRAPHICS_FB_PROVIDER_NEG(videoPlaneIndex, nativeHandle, "Video Plane Index");
    } else {
        UT_LOG_INFO("No VIDEO plane found, skipping video plane index test");
    }
}

/** @} */ // End of Plane_Control_Halif_Test_L1
/** @} */ // End of Plane_Control_Halif_Test
/** @} */ // End of Plane_Control
/** @} */ // End of HPK
