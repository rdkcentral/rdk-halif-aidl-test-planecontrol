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

// Common property lists for VIDEO and GRAPHICS planes
const std::vector<::com::rdk::hal::planecontrol::Property> VIDEO_PROPERTIES = {
    ::com::rdk::hal::planecontrol::Property::RESOURCE_ID,
    ::com::rdk::hal::planecontrol::Property::X,
    ::com::rdk::hal::planecontrol::Property::Y,
    ::com::rdk::hal::planecontrol::Property::WIDTH,
    ::com::rdk::hal::planecontrol::Property::HEIGHT,
    ::com::rdk::hal::planecontrol::Property::ALPHA,
    ::com::rdk::hal::planecontrol::Property::ZORDER,
    ::com::rdk::hal::planecontrol::Property::VISIBLE,
    ::com::rdk::hal::planecontrol::Property::OVERSCAN,
    ::com::rdk::hal::planecontrol::Property::ASPECT_RATIO
};

const std::vector<::com::rdk::hal::planecontrol::Property> GRAPHICS_PROPERTIES = {
    ::com::rdk::hal::planecontrol::Property::RESOURCE_ID,
    ::com::rdk::hal::planecontrol::Property::X,
    ::com::rdk::hal::planecontrol::Property::Y,
    ::com::rdk::hal::planecontrol::Property::WIDTH,
    ::com::rdk::hal::planecontrol::Property::HEIGHT,
    ::com::rdk::hal::planecontrol::Property::ALPHA,
    ::com::rdk::hal::planecontrol::Property::ZORDER,
    ::com::rdk::hal::planecontrol::Property::VISIBLE
};


class ValidPlaneControlListener : public BnPlaneControlListener {
public:

    ValidPlaneControlListener() {};
    virtual ~ValidPlaneControlListener() = default;
};
extern std::vector<planeControl_Cap> gPlaneControl_hfp;

class VTS_L1_PlaneControl : public virtual UTCore
{
protected:
    static void SetUpTestSuite()
    {

    }

    static void TearDownTestSuite()
    {

    }
};


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
 */

UT_ADD_TEST(VTS_L1_PlaneControl, getCapabilities_pos) {
    std::vector<::com::rdk::hal::planecontrol::Capabilities> capabilities;

    UT_LOG_INFO("Calling getCapabilities(OUT:capabilities[])");
    ::android::binder::Status status = gPlaneControlservice->getCapabilities(&capabilities);

    UT_LOG_INFO("Result getCapabilities(OUT:capabilities[size=%zu]), status:[isOK:%s, exception:%s]",
                capabilities.size(),
                UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

    UT_ASSERT_TRUE(status.isOk()) << "getCapabilities status not OK";
    UT_ASSERT_FALSE(capabilities.empty()) << "Capabilities list is empty";
    UT_ASSERT_EQUAL(capabilities.size(), gPlaneControl_hfp.size()) << "Capabilities size mismatch with HFP";

    for (const auto& cap : capabilities) {
        auto it = std::find_if(gPlaneControl_hfp.begin(), gPlaneControl_hfp.end(),
                               [&](const auto& hfp_cap) { return hfp_cap.planeIndex == cap.planeIndex; });

        if (it == gPlaneControl_hfp.end()) {
            UT_FAIL("Profile doesn't have plane with index: " + std::to_string(cap.planeIndex));
        } else {
            validateCapabilityMatch(cap, *it);
        }
    }

}


/**
 * @brief Test the retrieval of native graphics window handle for a valid GRAPHICS type plane.
 *
 * This test verifies the functionality of retrieving the native graphics window handle for a plane of type GRAPHICS. It ensures that the function correctly identifies a plane of the required type and retrieves the handle without errors.
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
 * | 01 | Retrieve the count of plane resources from YAML using the macro. | planeResourceCount = UT_KVP_PROFILE_GET_LIST_COUNT(IPlaneControl.planes) | Should be successful |  |
 * | 02 | Iterate through the plane resources to find a valid GRAPHICS type. | i = 0 to planeResourceCount | Should be successful |  |
 * | 03 | Check if the plane type at index i is GRAPHICS using getPlaneType(i). | planeType = getPlaneType(i) | Should be successful |  |
 * | 04 | Call getNativeGraphicsWindowHandle for the first valid GRAPHICS type plane. | planeResourceIndex = i, nativeGraphicsWindowHandle = 0 | Should Pass |  |
 * | 05 | Verify the status is OK and no exception occurred. | status.isOk() = true, status.exceptionCode() = EX_NONE | Should Pass |  |
 * | 06 | Verify the nativeGraphicsWindowHandle is not zero. | nativeGraphicsWindowHandle != 0 | Should Pass |  |
 */

 UT_ADD_TEST(VTS_L1_PlaneControl, getNativeGraphicsWindowHandle_pos) {
    int64_t nativeHandle = 0;
    ::android::binder::Status status;
    bool graphicsPlaneFound = false;

    // Loop through all planes from HFP
    for (const auto& cap : gPlaneControl_hfp) {
        if (cap.type == PlaneType::GRAPHICS) {
            graphicsPlaneFound = true;
            uint32_t planeIndex = cap.planeIndex;

            UT_LOG_INFO("Testing getNativeGraphicsWindowHandle for GRAPHICS plane index: %u", planeIndex);

            std::string inputParams = "IN:planeResourceIndex:[" + std::to_string(planeIndex) + "], OUT:nativeGraphicsWindowHandle[]";
            status = gPlaneControlservice->getNativeGraphicsWindowHandle(planeIndex, &nativeHandle);
            std::string outputParams = "IN:planeResourceIndex:[" + std::to_string(planeIndex) + "], OUT:nativeGraphicsWindowHandle[0x" + std::to_string(nativeHandle) + "]";

            UT_LOG_INFO("getNativeGraphicsWindowHandle", inputParams, outputParams, status, status.isOk());
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(nativeHandle != 0) << "Handle is zero for plane index: " << planeIndex;

            // Verify handle consistency
            int64_t handle2 = 0;
            status = gPlaneControlservice->getNativeGraphicsWindowHandle(planeIndex, &handle2);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_EQUAL(nativeHandle, handle2) << "Handles differ for plane index: " << planeIndex;
        }
    }

    ASSERT_TRUE(graphicsPlaneFound) << "No GRAPHICS plane found in gPlaneControl_hfp";
}


/**
 * @brief Test the behavior of getNativeGraphicsWindowHandle with an invalid plane resource index.
 *
 * This test checks the response of the getNativeGraphicsWindowHandle function when provided with an invalid plane resource index. The purpose is to ensure that the function handles invalid inputs gracefully and returns the appropriate error status.
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
 * | 01 | Initialize nativeGraphicsWindowHandle to 0 and declare status variable. | nativeGraphicsWindowHandle = 0 | Should be successful | |
 * | 02 | Set invalidPlaneResourceIndex to video plane index. | invalidPlaneResourceIndex = video plane index | Should be successful | |
 * | 03 | Call getNativeGraphicsWindowHandle with invalidPlaneResourceIndex and capture the status. | planeResourceIndex = -1, nativeGraphicsWindowHandle = 0 | status.exceptionCode() = EX_ILLEGAL_ARGUMENT, nativeGraphicsWindowHandle = 0 | Should Fail |
 * | 04 | Verify that nativeGraphicsWindowHandle remains 0. | nativeGraphicsWindowHandle = 0 | Should be successful | |
 * | 05 | Verify that the status exception code is EX_ILLEGAL_ARGUMENT. | status.exceptionCode() = EX_ILLEGAL_ARGUMENT | Should be successful | |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, getNativeGraphicsWindowHandle_neg) {
    int64_t nativeGraphicsWindowHandle = 0;
    ::android::binder::Status status;

    // Retrieve the count of plane resources from YAML using the macro
    std::string plane_count_path = "IPlaneControl.planes";
    uint32_t planeResourceCount = UT_KVP_PROFILE_GET_UINT32(plane_count_path.c_str());

    // Iterate through the plane resources to find a non-GRAPHICS type (negatively test non-graphics planes)
    for (int32_t i = 0; i < planeResourceCount; ++i) {
        char planeTypeString[256];
        std::string plane_type_path = "IPlaneControl.planes."+std::to_string(i)+".type";
        UT_KVP_PROFILE_GET_STRING(plane_type_path.c_str(), planeTypeString);

        // Test only for non-GRAPHICS planes (negatively test VIDEO planes)
        if (planeTypeString == "VIDEO") {
            // Corrected YAML path for plane index (if index is explicitly defined in profile)
            std::string plane_index_path = "IPlaneControl.planes." + std::to_string(i);
            uint32_t plane_indexfrom_profile = UT_KVP_PROFILE_GET_UINT32(plane_index_path.c_str());

            UT_LOG_INFO("Calling getNativeGraphicsWindowHandle(IN:planeResourceIndex:[%u], OUT:nativeGraphicsWindowHandle[])", plane_indexfrom_profile);

            // Call the method with a VIDEO plane resource
            status = gPlaneControlservice->getNativeGraphicsWindowHandle(plane_indexfrom_profile, &nativeGraphicsWindowHandle);

            UT_LOG_INFO("Result getNativeGraphicsWindowHandle(IN:planeResourceIndex:[%u], OUT:nativeGraphicsWindowHandle[0x%llX]), status:[isOK:%s, exception:%s]",
                        plane_indexfrom_profile,
                        nativeGraphicsWindowHandle,
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

            // Verification Steps
            // Since VIDEO planes are not graphics planes, this should return an ILLEGAL_ARGUMENT exception
            UT_ASSERT_EQUAL(status.exceptionCode(), ::android::binder::Status::EX_ILLEGAL_ARGUMENT);
        }
    }
}


/**
 * @brief Test the positive scenario of flipping a graphics buffer using a valid plane resource index.
 *
 * This test verifies the functionality of the flipGraphicsBuffer method by ensuring it successfully flips a graphics buffer when provided with a valid plane resource index. The test checks the return status and the boolean result to confirm the operation's success.
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
 * | 01 | Retrieve valid plane resource indices from YAML configuration. | None | Valid plane indices should be retrieved. | Should be successful |
 * | 02 | Ensure there is at least one valid GRAPHICS plane index. | validPlaneIndices | validPlaneIndices should not be empty. | Should be successful |
 * | 03 | Test with a valid plane resource index by calling flipGraphicsBuffer. | planeResourceIndex = validPlaneResourceIndex, aidl_return = false | status.isOk() should be true, aidl_return should be true. | Should Pass |
 */

 UT_ADD_TEST(VTS_L1_PlaneControl, flipGraphicsBuffer_pos) {
    uint32_t planeCount = 0;
    bool graphicsPlaneFound = false;

    PLANE_CONTROL_FOREACH_GRAPHICS_PLANE(planeCount, planeIndex, {
        graphicsPlaneFound = true;
        bool aidl_return = false;

        std::string inputParams = "IN:planeResourceIndex:[" + std::to_string(planeIndex) + "]";
        auto status = gPlaneControlservice->flipGraphicsBuffer(planeIndex, &aidl_return);
        std::string outputParams = "IN:planeResourceIndex:[" + std::to_string(planeIndex) + "], OUT:_aidl_return:[" + (aidl_return ? "true" : "false") + "]";

        UT_LOG_INFO("flipGraphicsBuffer", inputParams, outputParams, status, aidl_return);
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidl_return);
    });

    if (!graphicsPlaneFound) {
        GTEST_SKIP() << "Skipping test: No GRAPHICS plane found via getCapabilities()";
    }
}


/**
 * @brief Test the negative scenario of flipping the graphics buffer.
 *
 * This test verifies the behavior of the flipGraphicsBuffer function when provided with an invalid plane resource index. The test ensures that the function returns a failure status and the output parameter is false.
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
 * | 01 | Retrieve plane resource index using YAML macro | planeResourceIndex = UT_KVP_PROFILE_GET_UINT32(IPlaneControl.planes.<index>) | Should be successful | |
 * | 02 | Log before calling the API | planeResourceIndex = [value] | Should be successful | |
 * | 03 | Call the flipGraphicsBuffer API with invalid index | planeResourceIndex = [value], _aidl_return = false | status.isOk() = false, _aidl_return = false | Should Fail |
 * | 04 | Log the result of the API call | planeResourceIndex = [value], _aidl_return = [false], status = [isOK:false, exception:...] | Should be successful | |
 * | 05 | Verify the return value and output parameter | status.isOk() = false, _aidl_return = false | Should be successful | |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, flipGraphicsBuffer_neg) {
    bool _aidl_return = false;
    ::android::binder::Status status;
    // Retrieve the count of plane resources from YAML using the macro
    std::string plane_count_path = "IPlaneControl.planes";
    uint32_t planeResourceCount = UT_KVP_PROFILE_GET_UINT32(plane_count_path.c_str());
    // Iterate through the plane resources to find a  VIDEO type
    for (int32_t i = 0; i < planeResourceCount; ++i)
    {
        char planeTypeString[256];
        std::string plane_type_path = "IPlaneControl.planes."+std::to_string(i)+".type";
        UT_KVP_PROFILE_GET_STRING(plane_type_path.c_str(), planeTypeString);
        if (planeTypeString == "VIDEO") {
            std::string plane_index_path = "IPlaneControl.planes." + std::to_string(i);
            uint32_t plane_indexfrom_profile = UT_KVP_PROFILE_GET_UINT32(plane_index_path.c_str());

            UT_LOG_INFO("Calling flipGraphicsBuffer with video plane index (IN:planeResourceIndex:[%d], OUT:_aidl_return)", plane_indexfrom_profile);
            status = gPlaneControlservice->flipGraphicsBuffer(plane_indexfrom_profile, &_aidl_return);
            UT_LOG_INFO("Result flipGraphicsBuffer(IN:planeResourceIndex:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                        plane_indexfrom_profile, _aidl_return ? "true" : "false",
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_FALSE(status.isOk());
            UT_ASSERT_FALSE(_aidl_return);
        }
    }
}


/**
 * @brief Test the positive scenario of setting video source to destination plane mapping.
 *
 * This test verifies the functionality of setting video source to destination plane mapping for HDMI and COMPOSITE sources. It ensures that the mapping is correctly updated and can be unmapped successfully.
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

    // Step 1: Ensure HFP is parsed
    if (gPlaneControl_hfp.empty()) {
        UT_LOG_INFO("Parsing HFP data");
        if (vts_planecontrol_parsePlaneControlHFP() != UT_STATUS_OK) {
            UT_LOG_WARNING("Failed to parse HFP, relying on mock service");
        }
    }

    // Step 2: Collect video plane indices
    std::vector<uint32_t> planeIndices;
    for (const auto& cap : gPlaneControl_hfp) {
        if (cap.type == ::com::rdk::hal::planecontrol::PlaneType::VIDEO) {
            planeIndices.push_back(cap.planeIndex);
        }
    }
    UT_LOG_INFO("Video plane indices found: %zu", planeIndices.size());
    if (planeIndices.size() < 2) {
        UT_LOG_WARNING("Need at least 2 video planes, found %zu, checking gNoVideoTypePlanes", planeIndices.size());
    }
    UT_ASSERT_GREATER_EQUAL(gNoVideoTypePlanes, 2) << "Need at least 2 video planes, found " << gNoVideoTypePlanes;

    // Steps 3-4: Create mappings for HDMI and COMPOSITE
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> mappings;
    ::com::rdk::hal::planecontrol::SourcePlaneMapping mapping1;
    mapping1.sourceType = ::com::rdk::hal::planecontrol::SourceType::HDMI;
    mapping1.sourceIndex = 0;
    mapping1.destinationPlaneIndex = planeIndices[0];
    mappings.push_back(mapping1);

    ::com::rdk::hal::planecontrol::SourcePlaneMapping mapping2;
    mapping2.sourceType = ::com::rdk::hal::planecontrol::SourceType::COMPOSITE;
    mapping2.sourceIndex = 0;
    mapping2.destinationPlaneIndex = planeIndices[1];
    mappings.push_back(mapping2);

    // Steps 5-6: Set mappings
    bool aidl_return = false;
    ::android::binder::Status status;
    UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping(IN:mappings, OUT:_aidl_return)");
    status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(mappings, &aidl_return);
    UT_LOG_INFO("Result setVideoSourceDestinationPlaneMapping(IN:mappings, OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                aidl_return ? "true" : "false",
                UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_TRUE(aidl_return);

    // Verify mappings
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> retrievedMappings;
    UT_LOG_INFO("Calling getVideoSourceDestinationPlaneMapping(OUT:retrievedMappings[])");
    status = gPlaneControlservice->getVideoSourceDestinationPlaneMapping(&retrievedMappings);
    UT_LOG_INFO("Result getVideoSourceDestinationPlaneMapping(OUT:retrievedMappings[], status:[isOK:%s, exception:%s]",
                UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    UT_ASSERT_TRUE(status.isOk());

    // Check HDMI mapping
    bool foundHDMI = false;
    for (const auto& mapping : retrievedMappings) {
        if (mapping.destinationPlaneIndex == planeIndices[0] &&
            mapping.sourceType == ::com::rdk::hal::planecontrol::SourceType::HDMI &&
            mapping.sourceIndex == 0) {
            foundHDMI = true;
            break;
        }
    }
    UT_ASSERT_TRUE(foundHDMI) << "HDMI mapping not found for plane " << planeIndices[0];

    // Check COMPOSITE mapping
    bool foundComposite = false;
    for (const auto& mapping : retrievedMappings) {
        if (mapping.destinationPlaneIndex == planeIndices[1] &&
            mapping.sourceType == ::com::rdk::hal::planecontrol::SourceType::COMPOSITE &&
            mapping.sourceIndex == 0) {
            foundComposite = true;
            break;
        }
    }
    UT_ASSERT_TRUE(foundComposite) << "COMPOSITE mapping not found for plane " << planeIndices[1];

    // Steps 7-9: Unmap VIDEO_SINK
    mappings.clear();
    mapping1.sourceType = ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
    mapping1.sourceIndex = 0;
    mapping1.destinationPlaneIndex = -1;
    mappings.push_back(mapping1);

    UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping to unmap VIDEO_SINK");
    status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(mappings, &aidl_return);
    UT_LOG_INFO("Result setVideoSourceDestinationPlaneMapping(IN:mappings, OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                aidl_return ? "true" : "false",
                UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_TRUE(aidl_return);

    // Comprehensive unmapping for cleanup
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> unmapMappings;
    std::vector<::com::rdk::hal::planecontrol::SourceType> allSourceTypes = {
        ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK,
        ::com::rdk::hal::planecontrol::SourceType::HDMI,
        ::com::rdk::hal::planecontrol::SourceType::COMPOSITE
    };
    for (const auto& source : allSourceTypes) {
        for (int sourceIndex = 0; sourceIndex <= 1; ++sourceIndex) {
            ::com::rdk::hal::planecontrol::SourcePlaneMapping unmap;
            unmap.sourceType = source;
            unmap.sourceIndex = sourceIndex;
            unmap.destinationPlaneIndex = -1;
            unmapMappings.push_back(unmap);
        }
    }
    UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping to unmap all sources");
    status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(unmapMappings, &aidl_return);
    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_TRUE(aidl_return);
}

/**
 * @brief Test the negative scenarios for setting video source to destination plane mapping.
 *
 * This test verifies the behavior of the setVideoSourceDestinationPlaneMapping function when provided with invalid mappings, such as duplicate source types and indices, and duplicate destination plane indices. The test ensures that the function correctly handles these invalid inputs by returning false or throwing exceptions as expected.
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
 * | 01 | Retrieve the plane count from YAML configuration. | None | Should be successful | Should be successful |
 * | 02 | Iterate over available planes and prepare test data for duplicate SourceType and SourceIndex. | planeCount = value | Should be successful | Should be successful |
 * | 03 | Call setVideoSourceDestinationPlaneMapping with duplicate SourceType and SourceIndex. | listSourcePlaneMapping1 = {VIDEO_SINK, 0, i}, {VIDEO_SINK, 0, i+1} | _aidl_return = false | Should Fail |
 * | 04 | Prepare test data for duplicate DestinationPlaneIndex. | None | Should be successful | Should be successful |
 * | 05 | Call setVideoSourceDestinationPlaneMapping with duplicate DestinationPlaneIndex. | listSourcePlaneMapping2 = {VIDEO_SINK, 0, i}, {VIDEO_SINK, 1, i} | _aidl_return = false | Should Fail |
 * | 06 | Prepare test data for mapping to an already mapped plane. | None | Should be successful | Should be successful |
 * | 07 | Call setVideoSourceDestinationPlaneMapping with valid mapping. | listSourcePlaneMapping3 = {VIDEO_SINK, 1, 1} | status.isOk() = true | Should Pass |
 * | 08 | Call setVideoSourceDestinationPlaneMapping with mapping to an already mapped plane. | listSourcePlaneMapping4 = {VIDEO_SINK, 0, 1} | _aidl_return = false, status.exceptionCode() = EX_ILLEGAL_STATE | Should Fail |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, setVideoSourceDestinationPlaneMapping_neg) {
    UT_LOG_INFO("Starting setVideoSourceDestinationPlaneMapping_neg");

    // Ensure HFP is parsed
    if (gPlaneControl_hfp.empty()) {
        UT_LOG_INFO("Parsing HFP data");
        if (vts_planecontrol_parsePlaneControlHFP() != UT_STATUS_OK) {
            UT_LOG_WARNING("Failed to parse HFP, relying on mock service");
        }
    }

    // Step 1: Retrieve video plane indices
    std::vector<uint32_t> plane_indexfrom_profile;
    for (const auto& cap : gPlaneControl_hfp) {
        if (cap.type == ::com::rdk::hal::planecontrol::PlaneType::VIDEO) {
            plane_indexfrom_profile.push_back(cap.planeIndex);
        }
    }
    UT_LOG_INFO("Video plane indices found: %zu", plane_indexfrom_profile.size());
    if (plane_indexfrom_profile.size() < 2) {
        UT_LOG_WARNING("Need at least 2 video planes, found %zu, checking gNoVideoTypePlanes", plane_indexfrom_profile.size());
    }
    UT_ASSERT_GREATER_EQUAL(gNoVideoTypePlanes, 2) << "Need at least 2 video planes, found " << gNoVideoTypePlanes;

    // Prepare test data
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping1;
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping2;
    std::vector<::com::rdk::hal::planecontrol::SourceType> allSourceTypes = {
        ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK,
        ::com::rdk::hal::planecontrol::SourceType::HDMI,
        ::com::rdk::hal::planecontrol::SourceType::COMPOSITE
    };

    for (const auto& source : allSourceTypes) {
        // Clear mapping lists
        listSourcePlaneMapping1.clear();
        listSourcePlaneMapping2.clear();

        // Step 3: Test duplicate SourceType and SourceIndex
        ::com::rdk::hal::planecontrol::SourcePlaneMapping element1;
        element1.sourceType = source;
        element1.sourceIndex = 0;
        element1.destinationPlaneIndex = plane_indexfrom_profile[0];
        listSourcePlaneMapping1.push_back(element1);
        element1.destinationPlaneIndex = plane_indexfrom_profile[1];
        listSourcePlaneMapping1.push_back(element1);

        bool _aidl_return = false;
        ::android::binder::Status status;
        UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping(IN:listSourcePlaneMapping1, OUT:_aidl_return)");
        status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping1, &_aidl_return);
        UT_LOG_INFO("Result setVideoSourceDestinationPlaneMapping(IN:listSourcePlaneMapping1, OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]",
                    _aidl_return, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(_aidl_return);

        // Step 5: Test duplicate DestinationPlaneIndex
        element1.sourceType = source;
        element1.sourceIndex = 0;
        element1.destinationPlaneIndex = plane_indexfrom_profile[0];
        listSourcePlaneMapping2.push_back(element1);
        element1.sourceIndex = 1;
        listSourcePlaneMapping2.push_back(element1);

        UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping(IN:listSourcePlaneMapping2, OUT:_aidl_return)");
        status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping2, &_aidl_return);
        UT_LOG_INFO("Result setVideoSourceDestinationPlaneMapping(IN:listSourcePlaneMapping2, OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]",
                    _aidl_return, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(_aidl_return);

        // Steps 7-8: Test mapping to already mapped plane
        std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping3;
        element1.sourceType = source;
        element1.sourceIndex = 1;
        element1.destinationPlaneIndex = plane_indexfrom_profile[0];
        listSourcePlaneMapping3.push_back(element1);
        UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping(IN:listSourcePlaneMapping3, OUT:_aidl_return)");
        status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping3, &_aidl_return);
        UT_LOG_INFO("Result setVideoSourceDestinationPlaneMapping(IN:listSourcePlaneMapping3, OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]",
                    _aidl_return, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(_aidl_return);

        std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping4;
        element1.sourceType = source;
        element1.sourceIndex = 0;
        element1.destinationPlaneIndex = plane_indexfrom_profile[0];
        listSourcePlaneMapping4.push_back(element1);
        UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping(IN:listSourcePlaneMapping4, OUT:_aidl_return)");
        status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping4, &_aidl_return);
        UT_LOG_INFO("Result setVideoSourceDestinationPlaneMapping(IN:listSourcePlaneMapping4, OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]",
                    _aidl_return, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(_aidl_return);
        UT_ASSERT_EQUAL(status.exceptionCode(), ::android::binder::Status::EX_ILLEGAL_STATE);

        // Unmap all sources to reset state
        std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> unmapMappings;
        for (const auto& unmapSource : allSourceTypes) {
            for (int sourceIndex = 0; sourceIndex <= 1; ++sourceIndex) {
                ::com::rdk::hal::planecontrol::SourcePlaneMapping unmap;
                unmap.sourceType = unmapSource;
                unmap.sourceIndex = sourceIndex;
                unmap.destinationPlaneIndex = -1;
                unmapMappings.push_back(unmap);
            }
        }
        UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping to unmap all sources");
        status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(unmapMappings, &_aidl_return);
        UT_LOG_INFO("Result setVideoSourceDestinationPlaneMapping(IN:unmapMappings, OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]",
                    _aidl_return, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(_aidl_return);
    }
}

/**
 * @brief Test the positive scenario for retrieving video source to destination plane mapping.
 *
 * This test verifies the functionality of the getVideoSourceDestinationPlaneMapping API to ensure it correctly retrieves the mapping between video sources and destination planes. The test checks the status of the API call, the non-emptiness of the returned mapping, and validates the mapping details based on predefined conditions.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 008@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Call the getVideoSourceDestinationPlaneMapping API to retrieve the mapping. | aidl_return = std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> | Status should be OK, aidl_return should not be empty | Should Pass |
 * | 02 | Check if the status of the API call is OK. | status = gPlaneControlservice->getVideoSourceDestinationPlaneMapping(&aidl_return) | Status should be OK | Should be successful |
 * | 03 | Verify that the returned mapping is not empty. | aidl_return | aidl_return should not be empty | Should be successful |
 * | 04 | Validate the size of the returned mapping against the expected plane count. | expectedPlaneCount = 3 | aidl_return.size() should equal expectedPlaneCount | Should be successful |
 * | 05 | Iterate through each mapping and validate the source and destination indices. | mapping.sourceType, mapping.sourceIndex, mapping.destinationPlaneIndex | Correct mapping based on sourceType | Should be successful |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, getVideoSourceDestinationPlaneMapping_pos) {
    std::vector<::com::rdk::hal::planecontrol::Capabilities> capabilities;
    auto status = gPlaneControlservice->getCapabilities(&capabilities);
    UT_ASSERT_TRUE(status.isOk());

    // Count VIDEO planes using capabilities
    uint32_t video_plane_count = 0;
    for (const auto& cap : capabilities) {
        if (cap.type == ::com::rdk::hal::planecontrol::PlaneType::VIDEO) {
            video_plane_count++;
        }
    }
    //UT_ASSERT_GREATER_EQUAL(video_plane_count, 2);// do we need this check?

    // Call the method under test
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> aidl_return;
    status = gPlaneControlservice->getVideoSourceDestinationPlaneMapping(&aidl_return);

    UT_LOG_INFO("Result getVideoSourceDestinationPlaneMapping, status:[isOK:%s, exception:%s]",
                UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_EQUAL(aidl_return.size(), video_plane_count);
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
 * | 01 | Ensure the service is available | service = nullptr | Service should not be null | Should be successful |
 * | 02 | Retrieve the number of planes from the YAML configuration | IPlaneControl.planes | planeCount should be retrieved | Should be successful |
 * | 03 | Iterate over each plane resource index | planeResourceIndex = 0 to planeCount-1 | Each index should be processed | Should be successful |
 * | 04 | Iterate over all properties for each plane | property = RESOURCE_ID to ASPECT_RATIO | Each property should be processed | Should be successful |
 * | 05 | Log the API call for getProperty | planeResourceIndex, property | Log should be generated | Should be successful |
 * | 06 | Prepare the return value for getProperty | _aidl_return | _aidl_return should be initialized | Should be successful |
 * | 07 | Call the getProperty API | planeResourceIndex, property, _aidl_return | API should return status | Should Pass |
 * | 08 | Log the result of getProperty | planeResourceIndex, property, _aidl_return, status | Log should be generated | Should be successful |
 * | 09 | Verify the status is OK | status | status.isOk() should be true | Should Pass |
 * | 10 | Verify the return value is not null | _aidl_return | _aidl_return.has_value() should be true | Should Pass |
 * | 11 | Verify the return value matches the requested property | _aidl_return->value->getTag(), property | Tags should match | Should Pass |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, getProperty_pos) {
    uint32_t planeCount;

    PLANE_CONTROL_FOREACH_VIDEO_PLANE(planeCount, planeIndex, {
        ValidatePropertyList(planeIndex, VIDEO_PROPERTIES, "VIDEO");
    });

    PLANE_CONTROL_FOREACH_GRAPHICS_PLANE(planeCount, planeIndex, {
        ValidatePropertyList(planeIndex, GRAPHICS_PROPERTIES, "GRAPHICS");
    });
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
 * | 01 | Calculate invalid plane resource index by adding 1 to the count of planes | invalidPlaneResourceIndex = UT_KVP_PROFILE_GET_LIST_COUNT(IPlaneControl.planes) + 1 | Should return status.isOk() = false, aidlReturn.has_value() = false | Should be successful |
 * | 02 | Call getProperty with invalid plane resource index and valid property | planeResourceIndex = invalidPlaneResourceIndex, property = validProperty | Should return status.isOk() = false, aidlReturn.has_value() = false | Should Fail |
 * | 03 | Use valid plane resource index and invalid property value | validPlaneResourceIndex = 0, invalidPropertyValue = 999 | Should return status.isOk() = false, aidlReturn.has_value() = false | Should be successful |
 * | 04 | Call getProperty with valid plane resource index and invalid property value | planeResourceIndex = validPlaneResourceIndex, property = invalidPropertyValue | Should return status.isOk() = false, aidlReturn.has_value() = false | Should Fail |
 */

 UT_ADD_TEST(VTS_L1_PlaneControl, getProperty_neg) {
    ::com::rdk::hal::planecontrol::Property property = ::com::rdk::hal::planecontrol::Property::X;
    ::std::optional<::com::rdk::hal::PropertyValue> aidlReturn;
    int32_t invalidIndex = -1;

    std::string inputParams = "IN:planeResourceIndex:[" + std::to_string(invalidIndex) + "], IN:property:[X], OUT:propertyValue";
    auto status = gPlaneControlservice->getProperty(invalidIndex, property, &aidlReturn);
    std::string outputParams = "IN:planeResourceIndex:[" + std::to_string(invalidIndex) + "], IN:property:[X], OUT:propertyValue";
    UT_LOG_INFO("getProperty", inputParams, outputParams, status, status.isOk());
    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_FALSE(aidlReturn.has_value()); // Should return null
}


/**
 * @brief Test the positive setting of properties for plane control.
 *
 * This test verifies the ability to set various properties on plane resources using valid indices and property values. It ensures that the setProperty function behaves correctly for integer, boolean, and string property values.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 011@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve the number of plane resources from YAML configuration using the macro | IPlaneControl.planes | Should be successful | |
 * | 02 | Loop through each plane resource index | planeResourceIndex = i | Should be successful | |
 * | 03 | Set integer property value for the plane resource | planeResourceIndex = i, property = Property::X, propertyValue = intValue:123 | status.isOk() = true, aidl_return = true | Should Pass |
 * | 04 | Set boolean property value for the plane resource | planeResourceIndex = i, property = Property::VISIBLE, propertyValue = booleanValue:true | status.isOk() = true, aidl_return = true | Should Pass |
 * | 05 | Set string property value for the plane resource | planeResourceIndex = i, property = Property::RESOURCE_ID, propertyValue = stringValue:ResourceString | status.isOk() = true, aidl_return = true | Should Pass |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, setProperty_pos) {
    bool aidl_return = false;
    ::android::binder::Status status;

    std::vector<::com::rdk::hal::planecontrol::Capabilities> capabilities;
    status = gPlaneControlservice->getCapabilities(&capabilities);
    UT_ASSERT_TRUE(status.isOk());

    for (const auto& plane : capabilities) {
        uint32_t planeResourceIndex = plane.planeIndex;
        ::com::rdk::hal::planecontrol::PlaneType planeType = plane.type;

        if (planeType == ::com::rdk::hal::planecontrol::PlaneType::VIDEO) {
            PLANE_CONTROL_SET_AND_ASSERT_PROPERTY(planeResourceIndex, OVERSCAN, 10);
            PLANE_CONTROL_SET_AND_ASSERT_PROPERTY(planeResourceIndex, ASPECT_RATIO, 1);
        }

        PLANE_CONTROL_SET_AND_ASSERT_PROPERTY(planeResourceIndex, X, 123);
        PLANE_CONTROL_SET_AND_ASSERT_PROPERTY(planeResourceIndex, VISIBLE, 1);
        PLANE_CONTROL_SET_AND_ASSERT_PROPERTY(planeResourceIndex, Y, 5);
        PLANE_CONTROL_SET_AND_ASSERT_PROPERTY(planeResourceIndex, WIDTH, 50);
        PLANE_CONTROL_SET_AND_ASSERT_PROPERTY(planeResourceIndex, HEIGHT, 60);
        PLANE_CONTROL_SET_AND_ASSERT_PROPERTY(planeResourceIndex, ALPHA, 254);
        PLANE_CONTROL_SET_AND_ASSERT_PROPERTY(planeResourceIndex, ZORDER, 1);
    }
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
 * | 01 | Attempt to set a property with an invalid plane resource index | planeResourceIndex = invalidPlaneResourceIndex, property = WIDTH, propertyValue = intValue:100, aidl_return = false | aidl_return should be false | Should Fail |
 * | 02 | Attempt to set a property with an invalid property | planeResourceIndex = validPlaneResourceIndex, property = invalidProperty, propertyValue = intValue:100, aidl_return = false | aidl_return should be false | Should Fail |
 * | 03 | Attempt to set a property with an invalid property value type | planeResourceIndex = validPlaneResourceIndex, property = HEIGHT, propertyValue = stringValue:'InvalidType', aidl_return = false | aidl_return should be false | Should Fail |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, setProperty_neg) {
    bool aidl_return = false;
    ::android::binder::Status status;

    std::vector<::com::rdk::hal::planecontrol::Capabilities> capabilities;
    status = gPlaneControlservice->getCapabilities(&capabilities);
    UT_ASSERT_TRUE(status.isOk());

    std::vector<uint32_t> planeResourceIndices;
    for (const auto& plane : capabilities) {
        planeResourceIndices.push_back(plane.planeIndex);
    }

    std::sort(planeResourceIndices.begin(), planeResourceIndices.end());

    // Negative Test 1: Invalid Plane Resource Index
    int32_t invalidPlaneResourceIndex = planeResourceIndices.front() - 1;  // Invalid: before first valid index
    ::com::rdk::hal::planecontrol::Property propertyWidth = ::com::rdk::hal::planecontrol::Property::WIDTH;

    ::com::rdk::hal::PropertyValue invalidPlaneValue;
    invalidPlaneValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(100);

    UT_LOG_INFO("Calling setProperty(IN:planeResourceIndex:[%d], IN:property:[WIDTH], IN:propertyValue:[intValue:100])", invalidPlaneResourceIndex);
    status = gPlaneControlservice->setProperty(invalidPlaneResourceIndex, propertyWidth, invalidPlaneValue, &aidl_return);
    UT_LOG_INFO("Result setProperty => aidl_return:[%s], status:[isOK:%s, exception:%s]",
        aidl_return ? "true" : "false",
        UT_Control_GetMapString(bool_mapTable, status.isOk()),
        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    UT_ASSERT_FALSE(aidl_return);

    // Negative Test 2: Invalid Property Enum
    int32_t validPlaneResourceIndex = planeResourceIndices.front();  // Use a valid index
    int32_t invalidPropertyEnum = 999;  // Invalid enum not defined

    ::com::rdk::hal::PropertyValue invalidPropValue;
    invalidPropValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(100);

    UT_LOG_INFO("Calling setProperty(IN:planeResourceIndex:[%d], IN:property:[999], IN:propertyValue:[intValue:100])", validPlaneResourceIndex);
    status = gPlaneControlservice->setProperty(
        validPlaneResourceIndex,
        static_cast<::com::rdk::hal::planecontrol::Property>(invalidPropertyEnum),
        invalidPropValue,
        &aidl_return
    );
    UT_LOG_INFO("Result setProperty => aidl_return:[%s], status:[isOK:%s, exception:%s]",
        aidl_return ? "true" : "false",
        UT_Control_GetMapString(bool_mapTable, status.isOk()),
        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    UT_ASSERT_FALSE(aidl_return);

    // Negative Test 3: Invalid Property Value Type
    ::com::rdk::hal::planecontrol::Property propertyHeight = ::com::rdk::hal::planecontrol::Property::HEIGHT;
    ::com::rdk::hal::PropertyValue invalidTypeValue;
    invalidTypeValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::stringValue>(::android::String16("InvalidType"));

    UT_LOG_INFO("Calling setProperty(IN:planeResourceIndex:[%d], IN:property:[HEIGHT], IN:propertyValue:[stringValue:'InvalidType'])", validPlaneResourceIndex);
    status = gPlaneControlservice->setProperty(validPlaneResourceIndex, propertyHeight, invalidTypeValue, &aidl_return);
    UT_LOG_INFO("Result setProperty => aidl_return:[%s], status:[isOK:%s, exception:%s]",
        aidl_return ? "true" : "false",
        UT_Control_GetMapString(bool_mapTable, status.isOk()),
        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    UT_ASSERT_FALSE(aidl_return);
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
* | 01 | Prepare inputs for the test, including planeResourceIndex and propertyKVList with valid PropertyKVPair objects | planeResourceIndex = UT_KVP_PROFILE_GET_UINT32(IPlaneControl.planes.<index>.<key>), propertyKVList = {Property::RESOURCE_ID, Property::X, Property::Y, Property::WIDTH, Property::HEIGHT, Property::ALPHA, Property::ZORDER, Property::VISIBLE, Property::OVERSCAN, Property::ASPECT_RATIO} | Inputs prepared successfully | Should be successful |
* | 02 | Log the API call details for getPropertyMulti | planeResourceIndex, propertyKVList, _aidl_return | Log successful | Should be successful |
* | 03 | Call the getPropertyMulti API with the prepared inputs | planeResourceIndex, propertyKVList, _aidl_return | API call should succeed | Should Pass |
* | 04 | Verify that the status of the API call is OK | status.isOk() | True | Should Pass |
* | 05 | Verify that the _aidl_return is true, indicating successful property retrieval | _aidl_return | True | Should Pass |
* | 06 | Verify that the propertyKVList is not empty after the API call | propertyKVList.empty() | False | Should Pass |
* | 07 | Verify that each property in propertyKVList has a value | kvPair.propertyValue.value.has_value() | True for each kvPair | Should Pass |
* | 08 | Log the result of the getPropertyMulti API call | planeResourceIndex, propertyKVList, _aidl_return, status | Log successful | Should be successful |
*/
UT_ADD_TEST(VTS_L1_PlaneControl, getPropertyMulti_pos) {
    std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
    ::android::binder::Status status;

    // Properties to test
    const std::vector<::com::rdk::hal::planecontrol::Property> videoProperties = {
        ::com::rdk::hal::planecontrol::Property::RESOURCE_ID,
        ::com::rdk::hal::planecontrol::Property::X,
        ::com::rdk::hal::planecontrol::Property::Y,
        ::com::rdk::hal::planecontrol::Property::WIDTH,
        ::com::rdk::hal::planecontrol::Property::HEIGHT,
        ::com::rdk::hal::planecontrol::Property::ALPHA,
        ::com::rdk::hal::planecontrol::Property::ZORDER,
        ::com::rdk::hal::planecontrol::Property::VISIBLE,
        ::com::rdk::hal::planecontrol::Property::OVERSCAN,
        ::com::rdk::hal::planecontrol::Property::ASPECT_RATIO
    };

    const std::vector<::com::rdk::hal::planecontrol::Property> graphicProperties = {
        ::com::rdk::hal::planecontrol::Property::RESOURCE_ID,
        ::com::rdk::hal::planecontrol::Property::X,
        ::com::rdk::hal::planecontrol::Property::Y,
        ::com::rdk::hal::planecontrol::Property::WIDTH,
        ::com::rdk::hal::planecontrol::Property::HEIGHT,
        ::com::rdk::hal::planecontrol::Property::ALPHA,
        ::com::rdk::hal::planecontrol::Property::ZORDER,
        ::com::rdk::hal::planecontrol::Property::VISIBLE
    };

    // Get plane capabilities
    std::vector<::com::rdk::hal::planecontrol::Capabilities> capabilities;
    status = gPlaneControlservice->getCapabilities(&capabilities);
    UT_ASSERT_TRUE(status.isOk());

    for (const auto& plane : capabilities) {
        uint32_t planeResourceIndex = plane.planeIndex;
        ::com::rdk::hal::planecontrol::PlaneType planeType = plane.type;

        if (planeType == PlaneType::VIDEO) {
            PLANE_CONTROL_BUILD_PROPERTY_KV_LIST(videoProperties, propertyKVList);
            PLANE_CONTROL_VALIDATE_GET_PROPERTY_MULTI(planeResourceIndex, propertyKVList, "VIDEO");
        } else if (planeType == PlaneType::GRAPHICS) {
            PLANE_CONTROL_BUILD_PROPERTY_KV_LIST(graphicProperties, propertyKVList);
            PLANE_CONTROL_VALIDATE_GET_PROPERTY_MULTI(planeResourceIndex, propertyKVList, "GRAPHICS");
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
* | 02 | Prepare a valid plane resource index using UT_KVP_PROFILE_GET_UINT32 | validPlaneResourceIndex = UT_KVP_PROFILE_GET_UINT32(IPlaneControl.planes.<index>.<key>) | Should be successful | |
* | 03 | Create a valid property key-value pair and add to the list | property = WIDTH, propertyValue = 1920 | Should be successful | |
* | 04 | Create an invalid property key-value pair and add to the list | property = -1 | Should be successful | |
* | 05 | Call getPropertyMulti with invalid plane resource index | planeResourceIndex = -1, propertyKVList = [validPropertyKVPair, invalidPropertyKVPair], aidlReturn = false | aidlReturn = false, status.isOk() = true, status.exceptionCode() = EX_NONE | Should Fail |
* | 06 | Call getPropertyMulti with valid plane resource index and invalid property list | planeResourceIndex = validPlaneResourceIndex, propertyKVList = [validPropertyKVPair, invalidPropertyKVPair], aidlReturn = false | aidlReturn = false, status.isOk() = true, status.exceptionCode() = EX_NONE | Should Fail |
*/
UT_ADD_TEST(VTS_L1_PlaneControl, getPropertyMulti_neg) {
    ::android::binder::Status status;

    std::vector<::com::rdk::hal::planecontrol::Capabilities> capabilities;
    status = gPlaneControlservice->getCapabilities(&capabilities);
    UT_ASSERT_TRUE(status.isOk());

    // Gather sorted list of valid planeResourceIndexes
    std::vector<uint32_t> planeResourceIndex;
    for (const auto& plane : capabilities) {
        planeResourceIndex.push_back(plane.planeIndex);
    }
    std::sort(planeResourceIndex.begin(), planeResourceIndex.end());

    // Prepare test data
    int32_t invalidPlaneResourceIndex = static_cast<int32_t>(planeResourceIndex[0]) - 1;
    int32_t validPlaneResourceIndex = static_cast<int32_t>(planeResourceIndex[0]);

    std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> validpropertyKVList;
    std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> invalidpropertyKVList;

    // Valid property list with valid property
    ::com::rdk::hal::planecontrol::PropertyKVPair validKV;
    validKV.property = ::com::rdk::hal::planecontrol::Property::WIDTH;
    validpropertyKVList.push_back(validKV);

    // Invalid property list with an invalid enum value
    ::com::rdk::hal::planecontrol::PropertyKVPair invalidKV;
    invalidKV.property = static_cast<::com::rdk::hal::planecontrol::Property>(-1); // Clearly invalid
    invalidpropertyKVList.push_back(invalidKV);

    // Test Case 1: Invalid Plane Resource Index
    PLANE_CONTROL_NEGATIVE_TEST_GET_PROPERTY_MULTI(invalidPlaneResourceIndex, validpropertyKVList, "Invalid Plane Index");

    // Test Case 2: Invalid Property List
    PLANE_CONTROL_NEGATIVE_TEST_GET_PROPERTY_MULTI(validPlaneResourceIndex, invalidpropertyKVList, "Invalid Property");
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
 * | 01 | Retrieve the plane resource index using the macro | planeResourceIndex = UT_KVP_PROFILE_GET_UINT32(IPlaneControl.planes.<index>.<key>) | Should be successful | Should be successful |
 * | 02 | Prepare the property key-value list using macros | propertyKVList = {kvPair1, kvPair2, ...} | Should be successful | Should be successful |
 * | 03 | Log the API call | planeResourceIndex, propertyKVList | Should be successful | Should be successful |
 * | 04 | Call the setPropertyMultiAtomic API | planeResourceIndex, propertyKVList, _aidl_return | API should return true, status should be OK | Should Pass |
 * | 05 | Log the result of the API call | planeResourceIndex, propertyKVList, _aidl_return, status | Should be successful | Should be successful |
 * | 06 | Verify the results using assertions | status.isOk(), _aidl_return | Both assertions should be true | Should Pass |
 */

UT_ADD_TEST(VTS_L1_PlaneControl, setPropertyMultiAtomic_pos)
{
    bool _aidl_return = false;
    ::android::binder::Status status;
    std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
    // Get plane capabilities
    std::vector<::com::rdk::hal::planecontrol::Capabilities> capabilities;
    status = gPlaneControlservice->getCapabilities(&capabilities);
    UT_ASSERT_TRUE(status.isOk());

    for (const auto& plane : capabilities) {
        uint32_t planeResourceIndex = plane.planeIndex;
        ::com::rdk::hal::planecontrol::PlaneType planeType = plane.type;
        UT_LOG_INFO("Plane Index: %d, Plane Type: %d", plane.planeIndex, plane.type);

        if (planeType == PlaneType::VIDEO) {
            PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::OVERSCAN, 10);
            PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::ASPECT_RATIO, 1);
        }

        // Add common properties
        PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::X, 123);
        PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::Y, 6);
        PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::WIDTH, 40);
        PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::HEIGHT, 70);
        PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::ALPHA, 254);
        PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::ZORDER, 1);
        PLANE_CONTROL_ADD_PROP_TO_LIST(propertyKVList, ::com::rdk::hal::planecontrol::Property::VISIBLE, 1);

        // Call setPropertyMultiAtomic
        UT_LOG_INFO("Calling setPropertyMultiAtomic(IN:planeResourceIndex:[%d], IN:propertyKVList[], OUT:_aidl_return)", planeResourceIndex);
        ::android::binder::Status status = gPlaneControlservice->setPropertyMultiAtomic(planeResourceIndex, propertyKVList, &_aidl_return);
        UT_LOG_INFO("Result setPropertyMultiAtomic(IN:planeResourceIndex:[%d], IN:propertyKVList[], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    planeResourceIndex, UT_Control_GetMapString(bool_mapTable, _aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(_aidl_return);
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
 * | 08 | Add a duplicate property to the list. | duplicatePropertyKVList = list with duplicates | Should be successful | Should be successful |
 * | 09 | Call setPropertyMultiAtomic with validPlaneResourceIndex and duplicatePropertyKVList. | planeResourceIndex = validPlaneResourceIndex, propertyKVList = duplicate list | aidlReturn = false | Should Fail |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, setPropertyMultiAtomic_neg)
{
    bool aidlReturn = false;
    ::android::binder::Status status;
    std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> validPropertyKVList;
    std::vector<uint32_t> planeResourceIndex;

    // Get plane capabilities
    std::vector<::com::rdk::hal::planecontrol::Capabilities> capabilities;
    status = gPlaneControlservice->getCapabilities(&capabilities);
    UT_ASSERT_TRUE(status.isOk());

    // Gather available plane resource indexes
    for (const auto& plane : capabilities)
    {
        planeResourceIndex.push_back(plane.planeIndex);
    }

    std::sort(planeResourceIndex.begin(), planeResourceIndex.end());

    // Set invalid indexes
    int32_t invalidPlaneResourceIndexLow = planeResourceIndex[0] - 1;
    int32_t invalidPlaneResourceIndexHigh = planeResourceIndex.back() + 2;

    // Prepare valid PropertyKVPair list
    ::com::rdk::hal::planecontrol::PropertyKVPair kvPair;
    kvPair.property = ::com::rdk::hal::planecontrol::Property::X;
    kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(123);
    validPropertyKVList.push_back(kvPair);

    kvPair.property = ::com::rdk::hal::planecontrol::Property::Y;
    kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(6);
    validPropertyKVList.push_back(kvPair);

    // Test Case 1: Invalid Plane Resource Index (Low)
    UT_LOG_INFO("Calling setPropertyMultiAtomic(IN:planeResourceIndex:[%d], IN:propertyKVList[], OUT:_aidl_return)", invalidPlaneResourceIndexLow);
    status = gPlaneControlservice->setPropertyMultiAtomic(invalidPlaneResourceIndexLow, validPropertyKVList, &aidlReturn);
    UT_LOG_INFO("Result setPropertyMultiAtomic(IN:planeResourceIndex:[%d], IN:propertyKVList[], OUT:_aidl_return[%d]), status:[isOK:%s, exception:%s]",
                invalidPlaneResourceIndexLow, aidlReturn, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    UT_ASSERT_FALSE(aidlReturn);  // Expect failure due to invalid index

    // Test Case 2: Invalid Plane Resource Index (High)
    aidlReturn = false;
    UT_LOG_INFO("Calling setPropertyMultiAtomic(IN:planeResourceIndex:[%d], IN:propertyKVList[], OUT:_aidl_return)", invalidPlaneResourceIndexHigh);
    status = gPlaneControlservice->setPropertyMultiAtomic(invalidPlaneResourceIndexHigh, validPropertyKVList, &aidlReturn);
    UT_LOG_INFO("Result setPropertyMultiAtomic(IN:planeResourceIndex:[%d], IN:propertyKVList[], OUT:_aidl_return[%d]), status:[isOK:%s, exception:%s]",
                invalidPlaneResourceIndexHigh, aidlReturn, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    UT_ASSERT_FALSE(aidlReturn);  // Expect failure due to invalid index

    // Test Case 3: Duplicate Properties in Property List
    int32_t validPlaneResourceIndex = planeResourceIndex[0];
    std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> duplicatePropertyKVList;

    kvPair.property = ::com::rdk::hal::planecontrol::Property::WIDTH;
    kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(60);
    duplicatePropertyKVList.push_back(kvPair);

    // Add a duplicate property
    kvPair.property = ::com::rdk::hal::planecontrol::Property::WIDTH;
    kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(80);
    duplicatePropertyKVList.push_back(kvPair);

    aidlReturn = false;
    UT_LOG_INFO("Calling setPropertyMultiAtomic(IN:planeResourceIndex:[%d], IN:propertyKVList[], OUT:_aidl_return)", validPlaneResourceIndex);
    status = gPlaneControlservice->setPropertyMultiAtomic(validPlaneResourceIndex, duplicatePropertyKVList, &aidlReturn);
    UT_LOG_INFO("Result setPropertyMultiAtomic(IN:planeResourceIndex:[%d], IN:propertyKVList[], OUT:_aidl_return[%d]), status:[isOK:%s, exception:%s]",
                validPlaneResourceIndex, aidlReturn, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    UT_ASSERT_FALSE(aidlReturn);  // Expect failure due to duplicate properties
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
UT_ADD_TEST(VTS_L1_PlaneControl, registerListener_pos)
{
    // Create a valid instance of IPlaneControlListener
    android::sp<IPlaneControlListener> listener = new ValidPlaneControlListener();
    bool aidl_return = false;

    UT_LOG_INFO("Calling registerListener(IN:listener:[0x%X], OUT:_aidl_return)", listener.get());
    android::binder::Status status = gPlaneControlservice->registerListener(listener, &aidl_return);
    UT_LOG_INFO("Result registerListener(IN:listener:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                listener.get(), aidl_return ? "true" : "false",
                UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_TRUE(aidl_return);

    // Second registration attempt (should pass)
    UT_LOG_INFO("Calling Second registerListener(IN:listener:[0x%X], OUT:_aidl_return)", listener.get());
    status = gPlaneControlservice->registerListener(listener, &aidl_return);
    UT_LOG_INFO("Result registerListener(IN:listener:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                listener.get(), aidl_return ? "true" : "false",
                UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_FALSE(aidl_return);
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
UT_ADD_TEST(VTS_L1_PlaneControl, registerListener_neg)
{
    // Create a null listener (invalid)
    android::sp<IPlaneControlListener> listener = nullptr;
    bool aidl_return = false;

    // Attempt to register a null listener (should fail)
    android::binder::Status status = gPlaneControlservice->registerListener(listener, &aidl_return);
    UT_LOG_INFO("Attempt to register null listener(IN:listener:[nullptr], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                aidl_return ? "true" : "false",
                UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

    UT_ASSERT_FALSE(status.isOk());
    UT_ASSERT_FALSE(aidl_return);

    // Create a valid instance of IPlaneControlListener
    listener = new ValidPlaneControlListener();

    // First registration attempt (should succeed)
    status = gPlaneControlservice->registerListener(listener, &aidl_return);
    UT_LOG_INFO("First attempt to registerListener(IN:listener:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                listener.get(), aidl_return ? "true" : "false",
                UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_TRUE(aidl_return);

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
    // Create a valid listener instance
    android::sp<IPlaneControlListener> listener = new ValidPlaneControlListener();

    // Register the listener first to ensure it is registered
    bool registerResult;
    ::android::binder::Status registerStatus = gPlaneControlservice->registerListener(listener, &registerResult);
    UT_LOG_INFO("Calling registerListener(IN:listener[0x%X], OUT:registerResult[%d])", listener.get(), registerResult);
    UT_LOG_INFO("Result registerListener(IN:listener[0x%X], OUT:registerResult[%d]), status:[isOK:%s, exception:%s]",
                listener.get(), registerResult, UT_Control_GetMapString(bool_mapTable, registerStatus.isOk()),
                UT_Control_GetMapString(statusException_mapTable, registerStatus.exceptionCode()));

    UT_ASSERT_TRUE(registerStatus.isOk());
    UT_ASSERT_TRUE(registerResult);

    // Unregister the listener
    bool unregisterResult;
    ::android::binder::Status unregisterStatus = gPlaneControlservice->unregisterListener(listener, &unregisterResult);
    UT_LOG_INFO("Calling unregisterListener(IN:listener[0x%X], OUT:unregisterResult[%d])", listener.get(), unregisterResult);
    UT_LOG_INFO("Result unregisterListener(IN:listener[0x%X], OUT:unregisterResult[%d]), status:[isOK:%s, exception:%s]",
                listener.get(), unregisterResult, UT_Control_GetMapString(bool_mapTable, unregisterStatus.isOk()),
                UT_Control_GetMapString(statusException_mapTable, unregisterStatus.exceptionCode()));
    UT_ASSERT_TRUE(unregisterStatus.isOk());
    UT_ASSERT_TRUE(unregisterResult);
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
    // Test Case 1: Unregister Listener with Null Input
    {
        android::sp<IPlaneControlListener> nullListener = nullptr;
        bool aidl_return = false;
        UT_LOG_INFO("Calling unregisterListener(IN:listener:[nullptr], OUT:_aidl_return)");
        ::android::binder::Status status = gPlaneControlservice->unregisterListener(nullListener, &aidl_return);
        UT_LOG_INFO("Result unregisterListener(IN:listener:[nullptr], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidl_return);
        UT_ASSERT_FALSE(status.isOk());
    }

    // Test Case 2: Attempt to Unregister a Listener Not Registered
    {
        android::sp<IPlaneControlListener> unregisteredListener = new ValidPlaneControlListener();
        bool aidl_return = false;
        UT_LOG_INFO("Calling unregisterListener(IN:listener:[unregisteredListener], OUT:_aidl_return)");
        ::android::binder::Status status = gPlaneControlservice->unregisterListener(unregisteredListener, &aidl_return);
        UT_LOG_INFO("Result unregisterListener(IN:listener:[unregisteredListener], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidl_return);
        UT_ASSERT_TRUE(status.isOk());
    }

    // Test Case 3: Attempt to Unregister a Listener that is Registered and unregistered already
    {
        android::sp<IPlaneControlListener> registeredListener = new ValidPlaneControlListener();
        bool aidl_return = false;
        bool registerResult = false;

        // Simulate registration and unregistration
        ::android::binder::Status registerStatus = gPlaneControlservice->registerListener(registeredListener, &registerResult);
        UT_ASSERT_TRUE(registerStatus.isOk());
        UT_ASSERT_TRUE(registerResult);
        ::android::binder::Status status = gPlaneControlservice->unregisterListener(registeredListener, &aidl_return);
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidl_return);

        UT_LOG_INFO("Calling unregisterListener after register and unregister(IN:listener:[registeredListener], OUT:_aidl_return)");
        status = gPlaneControlservice->unregisterListener(registeredListener, &aidl_return);
        UT_LOG_INFO("Result unregisterListener(IN:listener:[registeredListener], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidl_return);
        UT_ASSERT_TRUE(status.isOk());
    }
}

/**
 * @brief Test the positive scenario of releasing a native graphics window handle.
 *
 * This test verifies the functionality of the releaseNativeGraphicsWindowHandle API to ensure it correctly releases a native graphics window handle without errors.@n
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 021@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve planeResourceIndex from YAML using macro | planeResourceIndex = UT_KVP_PROFILE_GET_LIST_COUNT(IPlaneControl.planes) | Should be successful | Should be successful |
 * | 02 | Assume nativeWindowHandle is a valid handle obtained from some previous operation | nativeWindowHandle = 123456789 | Should be successful | Should be successful |
 * | 03 | Log the API call | planeResourceIndex = 0, nativeWindowHandle = 123456789 | Should be successful | Should be successful |
 * | 04 | Call the API releaseNativeGraphicsWindowHandle | planeResourceIndex = 0, nativeWindowHandle = 123456789 | Status is OK, exception code is EX_NONE | Should Pass |
 * | 05 | Log the result of the API call | planeResourceIndex = 0, nativeWindowHandle = 123456789, status.isOk() = true, status.exceptionCode() = EX_NONE | Should be successful | Should be successful |
 * | 06 | Verify the API returns a successful status | status.isOk() = true, status.exceptionCode() = EX_NONE | Should be successful | Should be successful |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, releaseNativeGraphicsWindowHandle_pos) {
    int64_t nativeHandle = 0;
    ::android::binder::Status status;
    bool graphicsPlaneFound = false;
    uint32_t planeCount = 0;

    PLANE_CONTROL_FOREACH_GRAPHICS_PLANE(planeCount, planeIndex, {
        graphicsPlaneFound = true;

        status = gPlaneControlservice->getNativeGraphicsWindowHandle(planeIndex, &nativeHandle);
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(nativeHandle != 0);

        std::string inputParams = "IN:planeResourceIndex:[" + std::to_string(planeIndex) + "], IN:nativeWindowHandle:[0x" + std::to_string(nativeHandle) + "]";
        status = gPlaneControlservice->releaseNativeGraphicsWindowHandle(planeIndex, nativeHandle);
        std::string outputParams = "IN:planeResourceIndex:[" + std::to_string(planeIndex) + "], IN:nativeWindowHandle:[0x" + std::to_string(nativeHandle) + "]";

        UT_LOG_INFO("releaseNativeGraphicsWindowHandle", inputParams, outputParams, status, status.isOk());
        UT_ASSERT_TRUE(status.isOk());
    });

    if (!graphicsPlaneFound) {
        UT_FAIL("No GRAPHICS plane found in capabilities from gPlaneControl_hfp");
    }
}

/**
 * @brief Test the negative scenarios for releasing a native graphics window handle
 *
 * This test evaluates the behavior of the releaseNativeGraphicsWindowHandle function when provided with invalid inputs. It checks the function's response to an invalid plane resource index and an invalid native window handle.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 022@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve the plane resource index from YAML using the macro | planeCount = UT_KVP_PROFILE_GET_LIST_COUNT(IPlaneControl.planes) | Should be successful |  |
 * | 02 | Identify the first VIDEO type plane index for invalid test | invalidPlaneResourceIndex = videoPlaneIndices[0] | Should be successful |  |
 * | 03 | Identify a valid plane resource index that is not VIDEO type | validPlaneResourceIndex = i | Should be successful |  |
 * | 04 | Call releaseNativeGraphicsWindowHandle with invalid plane resource index | invalidPlaneResourceIndex = videoPlaneIndices[0], validNativeWindowHandle = 123456 | Should Fail |  |
 * | 05 | Call releaseNativeGraphicsWindowHandle with invalid native window handle | validPlaneResourceIndex = i, invalidNativeWindowHandle = -1 | Should Fail |  |
 */
UT_ADD_TEST(VTS_L1_PlaneControl, releaseNativeGraphicsWindowHandle_neg) {
    // Retrieve the plane resource index from YAML using the macro
    int64_t nativeGraphicsWindowHandle = 0;
    ::android::binder::Status status;

    // Retrieve the count of plane resources from YAML using the macro
    std::string plane_count_path = "IPlaneControl.planes";
    uint32_t planeResourceCount = UT_KVP_PROFILE_GET_UINT32(plane_count_path.c_str());
    bool graphics_index_flag = false;

    // Iterate through the plane resources to find a valid GRAPHICS type
    for (int32_t i = 0; i < planeResourceCount; ++i) {
        char planeTypeString[256];
        std::string plane_type_path = "IPlaneControl.planes."+std::to_string(i)+".type";
        UT_KVP_PROFILE_GET_STRING(plane_type_path.c_str(), planeTypeString);

        if (planeTypeString == "GRAPHICS") {

            graphics_index_flag = true;
            std::string plane_index_path = "IPlaneControl.planes." + std::to_string(i);
            uint32_t plane_indexfrom_profile = UT_KVP_PROFILE_GET_UINT32(plane_index_path.c_str());
            UT_LOG_INFO("Calling getNativeGraphicsWindowHandle(IN:planeResourceIndex:[%u], OUT:nativeGraphicsWindowHandle[])", plane_indexfrom_profile);
            status = gPlaneControlservice->getNativeGraphicsWindowHandle(plane_indexfrom_profile, &nativeGraphicsWindowHandle);
            UT_LOG_INFO("Result getNativeGraphicsWindowHandle(IN:planeResourceIndex:[%u], OUT:nativeGraphicsWindowHandle[0x%llX]), status:[isOK:%s, exception:%s]",
                        plane_indexfrom_profile,
                        nativeGraphicsWindowHandle,
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());

            // Test Case 1: Invalid Plane Resource Index
            uint32_t invalidPlaneResourceIndex = plane_indexfrom_profile+1;
            UT_LOG_INFO("Calling releaseNativeGraphicsWindowHandle(IN:planeResourceIndex:[%d], IN:nativeWindowHandle:[%lld])", invalidPlaneResourceIndex, nativeGraphicsWindowHandle);
            status = gPlaneControlservice->releaseNativeGraphicsWindowHandle(invalidPlaneResourceIndex, nativeGraphicsWindowHandle);
            UT_LOG_INFO("Result releaseNativeGraphicsWindowHandle(IN:planeResourceIndex:[%d], IN:nativeWindowHandle:[%lld]), status:[isOK:%s, exception:%s]", invalidPlaneResourceIndex, nativeGraphicsWindowHandle, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
            // Test Case 2: Invalid Native Window Handle
            int64_t invalidNativeWindowHandle = nativeGraphicsWindowHandle+1; // Invalid handle
            UT_LOG_INFO("Calling releaseNativeGraphicsWindowHandle(IN:planeResourceIndex:[%d], IN:nativeWindowHandle:[%lld])", plane_indexfrom_profile, invalidNativeWindowHandle);
            status = gPlaneControlservice->releaseNativeGraphicsWindowHandle(plane_indexfrom_profile, invalidNativeWindowHandle);
            UT_LOG_INFO("Result releaseNativeGraphicsWindowHandle(IN:planeResourceIndex:[%d], IN:nativeWindowHandle:[%lld]), status:[isOK:%s, exception:%s]", plane_indexfrom_profile, invalidNativeWindowHandle, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
        }
    }
}

/** @} */ // End of Plane_Control_Halif_Test_L1
/** @} */ // End of Plane_Control_Halif_Test
/** @} */ // End of Plane_Control
/** @} */ // End of HPK
