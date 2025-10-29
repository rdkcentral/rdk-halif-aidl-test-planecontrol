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
 * @defgroup Plane_Control_Halif_Test_Util Plane Control HALIF Tests util File
 * @{
 * @parblock
 *
 * ### utility functions for Plane Control VTS HALIF AIDL tests :
 *
 *
 *
 * **Pre-Conditions:**  None@n
 * **Dependencies:** None@n
 *
 * Ref to API Definition specification documentation :
 *
 * @endparblock
 */


#include "vts_planecontrol.h"


// Global vector to store parsed plane configurations
std::vector<planeControl_Cap> gPlaneControl_hfp;

extern sp<IPlaneControl> gPlaneControlservice;
// Conversion functions (assumed to exist)

std::string planeTypeToString(::com::rdk::hal::planecontrol::PlaneType type) {
    switch (type) {
        case ::com::rdk::hal::planecontrol::PlaneType::VIDEO: return "VIDEO";
        case ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS: return "GRAPHICS";
        default: return "UNKNOWN";
    }
}

std::string pixelFormatToString(::com::rdk::hal::videodecoder::PixelFormat format) {
    switch (format) {
        case ::com::rdk::hal::videodecoder::PixelFormat::YCBCR422: return "YCBCR422";
        case ::com::rdk::hal::videodecoder::PixelFormat::YCBCR444: return "YCBCR444";
        case ::com::rdk::hal::videodecoder::PixelFormat::YCBCR420: return "YCBCR420";
        case ::com::rdk::hal::videodecoder::PixelFormat::RGB: return "RGB";
        case ::com::rdk::hal::videodecoder::PixelFormat::NATIVE: return "NATIVE";
        default: return "UNKNOWN";
    }
}

std::string sourceTypeToString(::com::rdk::hal::planecontrol::SourceType type) {
    switch (type) {
        case ::com::rdk::hal::planecontrol::SourceType::NONE: return "NONE";
        case ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK: return "VIDEO_SINK";
        case ::com::rdk::hal::planecontrol::SourceType::HDMI: return "HDMI";
        case ::com::rdk::hal::planecontrol::SourceType::COMPOSITE: return "COMPOSITE";
        default: return "UNKNOWN";
    }
}

std::string dynamicRangeToString(::com::rdk::hal::videodecoder::DynamicRange range) {
    switch (range) {
        case ::com::rdk::hal::videodecoder::DynamicRange::UNKNOWN: return "UNKNOWN";
        case ::com::rdk::hal::videodecoder::DynamicRange::SDR: return "SDR";
        case ::com::rdk::hal::videodecoder::DynamicRange::HLG: return "HLG";
        case ::com::rdk::hal::videodecoder::DynamicRange::HDR10: return "HDR10";
        case ::com::rdk::hal::videodecoder::DynamicRange::HDR10PLUS: return "HDR10PLUS";
        case ::com::rdk::hal::videodecoder::DynamicRange::DOLBY_VISION: return "DOLBY_VISION";
        default: return "UNKNOWN";
    }
}

::com::rdk::hal::planecontrol::PlaneType stringToPlaneType(const std::string& typeStr) {
    if (typeStr == "VIDEO") return ::com::rdk::hal::planecontrol::PlaneType::VIDEO;
    if (typeStr == "GRAPHICS") return ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS;
    throw std::invalid_argument("Unknown plane type: " + typeStr);
}

::com::rdk::hal::planecontrol::SourceType stringToSourceType(const std::string& typeStr) {
    if (typeStr == "NONE") return ::com::rdk::hal::planecontrol::SourceType::NONE;
    if (typeStr == "VIDEO_SINK") return ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
    if (typeStr == "HDMI") return ::com::rdk::hal::planecontrol::SourceType::HDMI;
    if (typeStr == "COMPOSITE") return ::com::rdk::hal::planecontrol::SourceType::COMPOSITE;
    throw std::invalid_argument("Unknown plane type: " + typeStr);
}

::com::rdk::hal::videodecoder::PixelFormat stringToPixelFormat(const std::string& str) {
    if (str == "YCBCR422") return ::com::rdk::hal::videodecoder::PixelFormat::YCBCR422;
    if (str == "YCBCR444") return ::com::rdk::hal::videodecoder::PixelFormat::YCBCR444;
    if (str == "YCBCR420") return ::com::rdk::hal::videodecoder::PixelFormat::YCBCR420;
    if (str == "RGB") return ::com::rdk::hal::videodecoder::PixelFormat::RGB;
    if (str == "NATIVE") return ::com::rdk::hal::videodecoder::PixelFormat::NATIVE;
    throw std::invalid_argument("Invalid pixel format: " + str);
}

::com::rdk::hal::videodecoder::DynamicRange stringToDynamicRange(const std::string& drStr) {
    if (drStr == "UNKNOWN") return ::com::rdk::hal::videodecoder::DynamicRange::UNKNOWN;
    if (drStr == "SDR") return ::com::rdk::hal::videodecoder::DynamicRange::SDR;
    if (drStr == "HLG") return ::com::rdk::hal::videodecoder::DynamicRange::HLG;
    if (drStr == "HDR10") return ::com::rdk::hal::videodecoder::DynamicRange::HDR10;
    if (drStr == "HDR10PLUS") return ::com::rdk::hal::videodecoder::DynamicRange::HDR10PLUS;
    if (drStr == "DOLBY_VISION") return ::com::rdk::hal::videodecoder::DynamicRange::DOLBY_VISION;
    throw std::invalid_argument("Unknown dynamic range: " + drStr);
}

::com::rdk::hal::planecontrol::SourceType stringToSourceType(const std::string& str) {
    if (str == "NONE") return ::com::rdk::hal::planecontrol::SourceType::NONE;
    if (str == "VIDEO_SINK") return ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
    if (str == "HDMI") return ::com::rdk::hal::planecontrol::SourceType::HDMI;
    if (str == "COMPOSITE") return ::com::rdk::hal::planecontrol::SourceType::COMPOSITE;
    throw std::invalid_argument("Invalid source type: " + str);
}

UT_status_t getPlaneControlService() {
    gPlaneControlservice = android::interface_cast<IPlaneControl>(
            defaultServiceManager()->getService(String16(IPlaneControl::serviceName().c_str())));
    if (gPlaneControlservice == nullptr) {
        printf("PlaneControl Service not available\n");
        return UT_STATUS_FAILURE;
    }
    return UT_STATUS_OK;
}


// Helper function to validate property lists for VIDEO or GRAPHICS planes
bool ValidatePropertyList(uint32_t planeIndex, const std::vector<::com::rdk::hal::planecontrol::Property>& expectedProperties, const std::string& planeType) {
    std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
    for (const auto& property : expectedProperties) {
        ::com::rdk::hal::planecontrol::PropertyKVPair kvPair;
        kvPair.property = property;
        propertyKVList.push_back(kvPair);
    }

    bool aidl_return = false;
    std::string inputParams = "IN:planeResourceIndex:[" + std::to_string(planeIndex) + "], IN/OUT:propertyKVList[], OUT:_aidl_return";
    auto status = gPlaneControlservice->getPropertyMulti(planeIndex, &propertyKVList, &aidl_return);

    std::string outputParams = "IN:planeResourceIndex:[" + std::to_string(planeIndex) + "], IN/OUT:propertyKVList[], OUT:_aidl_return:[" + (aidl_return ? "true" : "false") + "]";
    //UT_LOG_INFO("getPropertyMulti with " + planeType + " plane index", inputParams, outputParams, status, aidl_return);

    bool success = true;
    if (!status.isOk()) {
        success = false;
        EXPECT_TRUE(status.isOk()) << "Status check failed for getPropertyMulti";
    }
    if (!aidl_return) {
        success = false;
        EXPECT_TRUE(aidl_return) << "aidl_return is false for getPropertyMulti";
    }
    if (propertyKVList.empty()) {
        success = false;
        EXPECT_FALSE(propertyKVList.empty()) << "propertyKVList is empty";
    }

    for (const auto& kvPair : propertyKVList) {
        if (!kvPair.propertyValue.value.has_value()) {
            success = false;
            EXPECT_TRUE(kvPair.propertyValue.value.has_value()) << "Property value is not set";
        }
    }

    return success;
}

UT_status_t vts_planecontrol_parsePlaneControlHFP() {
    const std::string planeControlPath = "IPlaneControl.planes";
    int32_t number_of_planes = 0;

    // Reset global counters
    gNoVideoTypePlanes = 0;
    gNoGraphicsTypePlanes = 0;

    // Verify KVP instance
    ut_kvp_instance_t* kvp_instance = ut_kvp_profile_getInstance();
    if (!kvp_instance) {
        UT_LOG_ERROR("Failed to get KVP profile instance");
        return UT_STATUS_FAILURE;
    }

    UT_LOG_INFO("Parsing HFP from: %s", planeControlPath.c_str());

    // Get number of planes
    number_of_planes = UT_KVP_PROFILE_GET_LIST_COUNT(planeControlPath.c_str());
    UT_LOG_INFO("Number of planes: %d", number_of_planes);
    if (number_of_planes <= 0) {
        UT_LOG_ERROR("No planes found in HFP at path: %s", planeControlPath.c_str());
        return UT_STATUS_FAILURE;
    }

    gPlaneControl_hfp.clear();
    for (int32_t i = 0; i < number_of_planes; ++i) {
        std::string index = std::to_string(i);
        std::string planeBasePath = planeControlPath + "." + index + "." + index; // Updated path with nested index
        std::string typePath = planeBasePath + ".type";
        std::string pixelFormatsPath = planeBasePath + ".pixelFormats";
        std::string dynamicRangesPath = planeBasePath + ".supportedDynamicRanges";

        planeControl_Cap cap;
        cap.planeIndex = i;

        // Read plane type
        std::string result_kvp(UT_KVP_MAX_ELEMENT_SIZE, '\0');
        ut_kvp_status_t status = ut_kvp_getStringField(kvp_instance, typePath.c_str(), result_kvp.data(), UT_KVP_MAX_ELEMENT_SIZE);
        if (status != UT_KVP_STATUS_SUCCESS) {
            UT_LOG_WARNING("Failed to read plane type at path: %s, status: %d, skipping plane %d",
                           typePath.c_str(), status, i);
            continue;
        }
        result_kvp.resize(std::strlen(result_kvp.c_str()));
        UT_LOG_DEBUG("Reading plane type path: %s, value: %s", typePath.c_str(), result_kvp.c_str());
        if (result_kvp.empty()) {
            UT_LOG_WARNING("Empty plane type at path: %s, skipping plane %d", typePath.c_str(), i);
            continue;
        }

        // Update global counters based on plane type
        if (result_kvp == "VIDEO") {
            gNoVideoTypePlanes++;
        } else if (result_kvp == "GRAPHICS") {
            gNoGraphicsTypePlanes++;
        }

        try {
            cap.type = stringToPlaneType(result_kvp);
        } catch (const std::invalid_argument& e) {
            UT_LOG_WARNING("Invalid plane type at path: %s, value: %s, error: %s",
                           typePath.c_str(), result_kvp.c_str(), e.what());
            continue;
        }

        // Read pixel formats
        int32_t pixel_format_count = UT_KVP_PROFILE_GET_LIST_COUNT(pixelFormatsPath.c_str());
        UT_LOG_INFO("Plane %d: pixel format count=%d", i, pixel_format_count);
        cap.pixelFormats.clear();
        for (int32_t j = 0; j < pixel_format_count; ++j) {
            std::string formatPath = pixelFormatsPath + "." + std::to_string(j);
            result_kvp.resize(UT_KVP_MAX_ELEMENT_SIZE);
            status = ut_kvp_getStringField(kvp_instance, formatPath.c_str(), result_kvp.data(), UT_KVP_MAX_ELEMENT_SIZE);
            if (status != UT_KVP_STATUS_SUCCESS) {
                UT_LOG_WARNING("Failed to read pixel format at path: %s, status: %d", formatPath.c_str(), status);
                continue;
            }
            result_kvp.resize(std::strlen(result_kvp.c_str()));
            UT_LOG_DEBUG("Reading pixel format path: %s, value: %s", formatPath.c_str(), result_kvp.c_str());
            if (result_kvp.empty()) {
                UT_LOG_WARNING("Empty pixel format at path: %s", formatPath.c_str());
                continue;
            }
            try {
                cap.pixelFormats.push_back(stringToPixelFormat(result_kvp));
            } catch (const std::invalid_argument& e) {
                UT_LOG_WARNING("Invalid pixel format at path: %s, value: %s, error: %s",
                               formatPath.c_str(), result_kvp.c_str(), e.what());
            }
        }
        if (cap.pixelFormats.empty()) {
            UT_LOG_WARNING("Plane %d has no valid pixel formats, skipping", i);
            continue;
        }

        // Read source types
        std::string sourceTypesPath = planeBasePath + ".sourceTypes";
        int32_t source_type_count = UT_KVP_PROFILE_GET_LIST_COUNT(sourceTypesPath.c_str());
        UT_LOG_INFO("Plane %d: source type count=%d", i, source_type_count);
        cap.sourceTypes.clear();
        for (int32_t j = 0; j < source_type_count; ++j) {
            std::string srcTypePath = sourceTypesPath + "." + std::to_string(j);
            result_kvp.resize(UT_KVP_MAX_ELEMENT_SIZE);
            status = ut_kvp_getStringField(kvp_instance, srcTypePath.c_str(), result_kvp.data(), UT_KVP_MAX_ELEMENT_SIZE);
            if (status != UT_KVP_STATUS_SUCCESS) {
                UT_LOG_WARNING("Failed to read source type at path: %s, status: %d", srcTypePath.c_str(), status);
                continue;
            }
            result_kvp.resize(std::strlen(result_kvp.c_str()));
            UT_LOG_DEBUG("Reading source type path: %s, value: %s", srcTypePath.c_str(), result_kvp.c_str());
            if (result_kvp.empty()) {
                UT_LOG_WARNING("Empty source type at path: %s", srcTypePath.c_str());
                continue;
            }
            try {
                cap.sourceTypes.push_back(stringToSourceType(result_kvp));
            } catch (const std::invalid_argument& e) {
                UT_LOG_WARNING("Invalid source type at path: %s, value: %s, error: %s",
                               srcTypePath.c_str(), result_kvp.c_str(), e.what());
            }
        }
        if (cap.sourceTypes.empty()) {
            UT_LOG_WARNING("Plane %d has no valid source types, skipping", i);
            continue;
        }

        // Read supported dynamic ranges
        int32_t dynamic_range_count = UT_KVP_PROFILE_GET_LIST_COUNT(dynamicRangesPath.c_str());
        UT_LOG_INFO("Plane %d: dynamic range count=%d", i, dynamic_range_count);
        cap.supportedDynamicRanges.clear();
        for (int32_t j = 0; j < dynamic_range_count; ++j) {
            std::string drPath = dynamicRangesPath + "." + std::to_string(j);
            result_kvp.resize(UT_KVP_MAX_ELEMENT_SIZE);
            status = ut_kvp_getStringField(kvp_instance, drPath.c_str(), result_kvp.data(), UT_KVP_MAX_ELEMENT_SIZE);
            if (status != UT_KVP_STATUS_SUCCESS) {
                UT_LOG_WARNING("Failed to read dynamic range at path: %s, status: %d", drPath.c_str(), status);
                continue;
            }
            result_kvp.resize(std::strlen(result_kvp.c_str()));
            UT_LOG_DEBUG("Reading dynamic range path: %s, value: %s", drPath.c_str(), result_kvp.c_str());
            if (result_kvp.empty()) {
                UT_LOG_WARNING("Empty dynamic range at path: %s", drPath.c_str());
                continue;
            }
            try {
                cap.supportedDynamicRanges.push_back(stringToDynamicRange(result_kvp));
            } catch (const std::invalid_argument& e) {
                UT_LOG_WARNING("Invalid dynamic range at path: %s, value: %s, error: %s",
                               drPath.c_str(), result_kvp.c_str(), e.what());
            }
        }
        if (cap.supportedDynamicRanges.empty()) {
            UT_LOG_WARNING("Plane %d has no valid dynamic ranges, skipping", i);
            continue;
        }

        // Read maxWidth, maxHeight, frameWidth, frameHeight
        cap.maxWidth = UT_KVP_PROFILE_GET_UINT32((planeBasePath + ".maxWidth").c_str());
        cap.maxHeight = UT_KVP_PROFILE_GET_UINT32((planeBasePath + ".maxHeight").c_str());
        cap.frameWidth = UT_KVP_PROFILE_GET_UINT32((planeBasePath + ".frameWidth").c_str());
        cap.frameHeight = UT_KVP_PROFILE_GET_UINT32((planeBasePath + ".frameHeight").c_str());
        if (cap.maxWidth <= 0 || cap.maxHeight <= 0) {
            UT_LOG_WARNING("Invalid maxWidth=%d or maxHeight=%d for plane %d, skipping",
                           cap.maxWidth, cap.maxHeight, i);
            continue;
        }

        // Read other properties
        cap.colorDepth = UT_KVP_PROFILE_GET_UINT32((planeBasePath + ".colorDepth").c_str());
        cap.maxFrameRate = UT_KVP_PROFILE_GET_UINT32((planeBasePath + ".maxFrameRate").c_str());
        cap.supportsAlpha = UT_KVP_PROFILE_GET_BOOL((planeBasePath + ".supportsAlpha").c_str());
        cap.supportsZOrder = UT_KVP_PROFILE_GET_BOOL((planeBasePath + ".supportsZOrder").c_str());
        cap.vsyncDisplayLatency = UT_KVP_PROFILE_GET_UINT32((planeBasePath + ".vsyncDisplayLatency").c_str());

        UT_LOG_INFO("Plane %d: index=%d, type=%d, %zu pixel formats, %zu source types, %zu dynamic ranges, colorDepth=%d, maxFrameRate=%d",
                    i, cap.planeIndex, static_cast<int>(cap.type), cap.pixelFormats.size(),
                    cap.sourceTypes.size(), cap.supportedDynamicRanges.size(), cap.colorDepth, cap.maxFrameRate);

        gPlaneControl_hfp.push_back(cap);
    }

    if (gPlaneControl_hfp.empty()) {
        UT_LOG_ERROR("No valid planes parsed from HFP");
        return UT_STATUS_FAILURE;
    }

    UT_LOG_INFO("Successfully parsed %zu planes, VIDEO planes: %u, GRAPHICS planes: %u",
                gPlaneControl_hfp.size(), gNoVideoTypePlanes, gNoGraphicsTypePlanes);
    return UT_STATUS_OK;
}

void validateCapabilityMatch(const ::com::rdk::hal::planecontrol::Capabilities& actual,
    const planeControl_Cap& expected) {
    // Verify planeIndex
    UT_ASSERT_EQUAL(actual.planeIndex, expected.planeIndex) << "Plane index mismatch";

    // Verify type
    UT_ASSERT_EQUAL(actual.type, expected.type) << "Plane type mismatch, expected: "
                        << planeTypeToString(expected.type)
                        << ", got: " << planeTypeToString(actual.type);

    // Verify pixelFormats
    auto expected_pixel_formats = expected.pixelFormats;
    auto actual_pixel_formats = actual.pixelFormats;
    std::sort(expected_pixel_formats.begin(), expected_pixel_formats.end());
    std::sort(actual_pixel_formats.begin(), actual_pixel_formats.end());
    UT_ASSERT_EQUAL(expected_pixel_formats.size(), actual_pixel_formats.size()) << "Pixel format count mismatch";
    bool pixel_formats_match = std::equal(expected_pixel_formats.begin(), expected_pixel_formats.end(), actual_pixel_formats.begin());
    if (!pixel_formats_match) {
    std::string expected_str, actual_str;
    for (const auto& pf : expected_pixel_formats) expected_str += pixelFormatToString(pf) + ",";
    for (const auto& pf : actual_pixel_formats) actual_str += pixelFormatToString(pf) + ",";
    UT_FAIL("Pixel formats mismatch for plane " << actual.planeIndex
                            << ", expected: [" << expected_str << "]"
                            << ", got: [" << actual_str << "]");
    }

    // Verify sourceTypes
    auto expected_source_types = expected.sourceTypes;
    auto actual_source_types = actual.sourceTypes;
    std::sort(expected_source_types.begin(), expected_source_types.end());
    std::sort(actual_source_types.begin(), actual_source_types.end());
    UT_ASSERT_EQUAL(expected_source_types.size(), actual_source_types.size()) << "Source type count mismatch";
    bool source_types_match = std::equal(expected_source_types.begin(), expected_source_types.end(), actual_source_types.begin());
    if (!source_types_match) {
        std::string expected_str, actual_str;
        for (const auto& st : expected_source_types) expected_str += sourceTypeToString(st) + ",";
        for (const auto& st : actual_source_types) actual_str += sourceTypeToString(st) + ",";
        UT_FAIL("Source types mismatch for plane " << actual.planeIndex
                                << ", expected: [" << expected_str << "]"
                                << ", got: [" << actual_str << "]");
    }

    // Verify supportedDynamicRanges
    auto expected_dynamic_ranges = expected.supportedDynamicRanges;
    auto actual_dynamic_ranges = actual.supportedDynamicRanges;
    std::sort(expected_dynamic_ranges.begin(), expected_dynamic_ranges.end());
    std::sort(actual_dynamic_ranges.begin(), actual_dynamic_ranges.end());
    UT_ASSERT_EQUAL(expected_dynamic_ranges.size(), actual_dynamic_ranges.size()) << "Dynamic range count mismatch";
    bool dynamic_ranges_match = std::equal(expected_dynamic_ranges.begin(), expected_dynamic_ranges.end(), actual_dynamic_ranges.begin());
    if (!dynamic_ranges_match) {
    std::string expected_str, actual_str;
    for (const auto& dr : expected_dynamic_ranges) expected_str += dynamicRangeToString(dr) + ",";
    for (const auto& dr : actual_dynamic_ranges) actual_str += dynamicRangeToString(dr) + ",";
    UT_FAIL("Dynamic ranges mismatch for plane " << actual.planeIndex
                                << ", expected: [" << expected_str << "]"
                                << ", got: [" << actual_str << "]");
    }

    // Verify other fields
    UT_ASSERT_EQUAL(actual.maxWidth, expected.maxWidth) << "maxWidth mismatch";
    UT_ASSERT_EQUAL(actual.maxHeight, expected.maxHeight) << "maxHeight mismatch";
    UT_ASSERT_EQUAL(actual.frameWidth, expected.frameWidth) << "frameWidth mismatch";
    UT_ASSERT_EQUAL(actual.frameHeight, expected.frameHeight) << "frameHeight mismatch";
    UT_ASSERT_EQUAL(actual.colorDepth, expected.colorDepth) << "colorDepth mismatch";
    UT_ASSERT_EQUAL(actual.maxFrameRate, expected.maxFrameRate) << "maxFrameRate mismatch";
    UT_ASSERT_EQUAL(actual.supportsAlpha, expected.supportsAlpha) << "supportsAlpha mismatch";
    UT_ASSERT_EQUAL(actual.supportsZOrder, expected.supportsZOrder) << "supportsZOrder mismatch";
    UT_ASSERT_EQUAL(actual.vsyncDisplayLatency, expected.vsyncDisplayLatency) << "vsyncDisplayLatency mismatch";
}


/** @} */ // End of Plane_Control_Halif_Test_Util
/** @} */ // End of Plane_Control_Halif_Test
/** @} */ // End of Plane_Control
/** @} */ // End of HPK