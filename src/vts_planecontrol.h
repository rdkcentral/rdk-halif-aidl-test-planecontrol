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
 * @defgroup Plane_Control_Halif_Test_Header Plane Control HALIF Tests Header File
 * @{
 * @parblock
 *
 * ### Header functions for Plane Control VTS HALIF AIDL tests :
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

#ifndef __VTS_PLANECONTROL_H__
#define __VTS_PLANECONTROL_H__

#include <ut_kvp_profile.h>
#include <ut.h>
#include <ut_log.h>
#include <ut_control_plane.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/Status.h>
#include <utils/String16.h>
#include <stdio.h>
#include <vector>
#include <optional>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <unordered_set>
#include <cstring>  // Added for std::strlen
#include <string>
#include <vector>
#include <unistd.h>

#include "com/rdk/hal/planecontrol/IPlaneControl.h"
#include "com/rdk/hal/planecontrol/BnPlaneControlListener.h"

using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;

using namespace ::com::rdk::hal;
using namespace ::com::rdk::hal::videodecoder;
using namespace ::com::rdk::hal::planecontrol;

#define PLANE_CONTROL_MGR "IPlaneControl"
#define PLANE_CONTROL_MGR_PLANES (std::string(PLANE_CONTROL_MGR) + ".planes")

#define UT_KVP_MAX_KEY_SIZE 128

typedef struct Capabilities {
    int32_t planeIndex;
    PlaneType type;
    std::vector<PixelFormat> pixelFormats;
    std::vector<DynamicRange> supportedDynamicRanges;
    uint32_t maxWidth;
    uint32_t maxHeight;
    uint32_t frameWidth;
    uint32_t frameHeight;
    uint32_t colorDepth;
    uint32_t maxFrameRate;
    bool supportsAlpha;
    bool supportsZOrder;
    uint32_t vsyncDisplayLatency;
}planeControl_Cap;

extern std::vector<planeControl_Cap> gPlaneControl_hfp;
extern sp<IPlaneControl> gPlaneControlservice;

// Function to parse the PlaneControl HFP profile
UT_status_t vts_planecontrol_parsePlaneControlHFP();
UT_status_t getPlaneControlService();

bool ValidatePropertyList(uint32_t planeIndex, const std::vector<::com::rdk::hal::planecontrol::Property>& expectedProperties, const std::string& planeType);
UT_status_t VTS_PlaneControl_init();
void validateCapabilityMatch(const ::com::rdk::hal::planecontrol::Capabilities& actual,
    const planeControl_Cap& expected);

/* Status::Exception */
const static ut_control_keyStringMapping_t statusException_mapTable[] =
{
    {(char*)"EX_NONE",                  (int32_t) Status::EX_NONE},
    {(char*)"EX_SECURITY",              (int32_t) Status::EX_SECURITY},
    {(char*)"EX_BAD_PARCELABLE",        (int32_t) Status::EX_BAD_PARCELABLE},
    {(char*)"EX_ILLEGAL_ARGUMENT",      (int32_t) Status::EX_ILLEGAL_ARGUMENT},
    {(char*)"EX_NULL_POINTER",          (int32_t) Status::EX_NULL_POINTER},
    {(char*)"EX_ILLEGAL_STATE",         (int32_t) Status::EX_ILLEGAL_STATE},
    {(char*)"EX_NETWORK_MAIN_THREAD",   (int32_t) Status::EX_NETWORK_MAIN_THREAD},
    {(char*)"EX_UNSUPPORTED_OPERATION", (int32_t) Status::EX_UNSUPPORTED_OPERATION},
    {(char*)"EX_SERVICE_SPECIFIC",      (int32_t) Status::EX_SERVICE_SPECIFIC},
    {(char*)"EX_PARCELABLE",            (int32_t) Status::EX_PARCELABLE},
    {(char*)"EX_HAS_REPLY_HEADER",      (int32_t) Status::EX_HAS_REPLY_HEADER},
    {(char*)"EX_TRANSACTION_FAILED",    (int32_t) Status::EX_TRANSACTION_FAILED},
    {NULL, -1}
};

/* bool */
const static ut_control_keyStringMapping_t bool_mapTable[] =
{
    {(char*)"false", (int32_t) false},
    {(char*)"true",  (int32_t) true},
    {NULL, -1}
};

#if 0
// Helper functions to convert strings to enums
::com::rdk::hal::planecontrol::PlaneType stringToPlaneType(const std::string& typeStr);




::com::rdk::hal::planecontrol::SourceType stringToSourceType(const std::string& str);

::com::rdk::hal::planecontrol::SourceType stringToSourceType(const std::string& str) {
    if (str == "NONE") return ::com::rdk::hal::planecontrol::SourceType::NONE;
    if (str == "VIDEO_SINK") return ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
    if (str == "HDMI") return ::com::rdk::hal::planecontrol::SourceType::HDMI;
    if (str == "COMPOSITE") return ::com::rdk::hal::planecontrol::SourceType::COMPOSITE;
    throw std::invalid_argument("Invalid source type: " + str);
}
#endif


// Macro to iterate over all planes and filter by type (VIDEO or GRAPHICS)
#define FOREACH_PLANE_BY_TYPE(planeCount, planeTypeEnum, desiredType, planeIndexVar, body) \
    do { \
        std::vector<::com::rdk::hal::planecontrol::Capabilities> capabilities; \
        auto status = gPlaneControlservice->getCapabilities(&capabilities); \
        UT_ASSERT_TRUE(status.isOk()); \
        planeCount = capabilities.size(); \
        for (const auto& cap : capabilities) { \
            if (cap.type == desiredType) { \
                uint32_t planeIndexVar = cap.planeIndex; \
                body \
            } \
        } \
    } while (0)



// Macro to iterate over VIDEO planes
#define FOREACH_VIDEO_PLANE(planeCount, planeIndexVar, body) \
    FOREACH_PLANE_BY_TYPE(planeCount, planeTypeEnum, ::com::rdk::hal::planecontrol::PlaneType::VIDEO, planeIndexVar, body)

// Macro to iterate over GRAPHICS planes
#define FOREACH_GRAPHICS_PLANE(planeCount, planeIndexVar, body) \
    FOREACH_PLANE_BY_TYPE(planeCount, planeTypeEnum, ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS, planeIndexVar, body)


// Macro to ensure at least one plane of the desired type exists
#define ENSURE_PLANE_TYPE_EXISTS(planeCount, desiredType, failMessage) \
    do { \
        uint32_t count = 0; \
        FOREACH_PLANE_BY_TYPE(count, planeTypeString, desiredType, planeIndexVar, { count++; }); \
        if (count == 0) { \
            UT_FAIL(failMessage); \
        } \
    } while (0)

// Macro to ensure at least N planes of the desired type exist
#define ENSURE_MIN_PLANE_TYPE_COUNT(planeCount, desiredType, minCount, failMessage) \
    do { \
        uint32_t count = 0; \
        FOREACH_PLANE_BY_TYPE(count, planeTypeString, desiredType, planeIndexVar, { count++; }); \
        UT_ASSERT_GREATER_EQUAL(count, minCount); \
        if (count < minCount) { \
            UT_FAIL(failMessage); \
        } \
    } while (0)

#define LOG_API_CALL(apiName, inputParams, outputParams, status, result) \
    do { \
        UT_LOG_INFO("Calling %s(%s)", apiName, (inputParams).c_str()); \
        UT_LOG_INFO("Result %s(%s), status:[isOK:%s, exception:%s], result:[%s]", \
                    apiName, (outputParams).c_str(), \
                    UT_Control_GetMapString(bool_mapTable, (status).isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, (status).exceptionCode()), \
                    (result) ? "true" : "false"); \
    } while (0)

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

#define SET_AND_ASSERT_PROPERTY(planeResourceIndex, propertyEnum, intVal)                                              \
    {                                                                                                                  \
        ::com::rdk::hal::PropertyValue propertyValue;                                                                  \
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(intVal); \
        UT_LOG_INFO("Calling setProperty(IN:planeResourceIndex:[%d], IN:property:[%s], IN:propertyValue:[intValue:%d])", \
                    planeResourceIndex, #propertyEnum, intVal);                                                       \
        status = gPlaneControlservice->setProperty(planeResourceIndex, ::com::rdk::hal::planecontrol::Property::propertyEnum, propertyValue, &aidl_return); \
        UT_LOG_INFO("Result setProperty(IN:planeResourceIndex:[%d], IN:property:[%s], IN:propertyValue:[intValue:%d]), status:[isOK:%s, exception:%s], aidl_return:[%s]", \
                    planeResourceIndex, #propertyEnum, intVal,                                                        \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),                                            \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()),                        \
                    aidl_return ? "true" : "false");                                                                  \
        UT_ASSERT_TRUE(status.isOk());                                                                                 \
        UT_ASSERT_TRUE(aidl_return);                                                                                   \
    }

// Macro: Builds and populates a property list
#define BUILD_PROPERTY_KV_LIST(propertyList, kvList) \
    do { \
        kvList.clear(); \
        for (const auto& property : propertyList) { \
            ::com::rdk::hal::planecontrol::PropertyKVPair kvPair; \
            kvPair.property = property; \
            kvList.push_back(kvPair); \
        } \
    } while(0)

// Macro: Executes getPropertyMulti and asserts
#define VALIDATE_GET_PROPERTY_MULTI(index, kvList, planeTypeLabel) \
    do { \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling getPropertyMulti [%s] plane (IN:planeResourceIndex:[%d], IN/OUT:propertyKVList[], OUT:_aidl_return)", planeTypeLabel, index); \
        ::android::binder::Status status = gPlaneControlservice->getPropertyMulti(index, &kvList, &_aidl_return); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_FALSE(kvList.empty()); \
        for (const auto& kvPair : kvList) { \
            UT_ASSERT_TRUE(kvPair.propertyValue.value.has_value()); \
        } \
        UT_LOG_INFO("Result getPropertyMulti [%s] plane (IN:planeResourceIndex:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s])", \
            planeTypeLabel, index, \
            UT_Control_GetMapString(bool_mapTable, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
    } while(0)

// Macro: Executes getPropertyMulti with expected failure and logs/asserts
#define NEGATIVE_TEST_GET_PROPERTY_MULTI(index, kvList, label) \
    do { \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling getPropertyMulti [%s] (IN:planeResourceIndex:[%d], IN:propertyKVList[], OUT:_aidl_return)", label, index); \
        ::android::binder::Status status = gPlaneControlservice->getPropertyMulti(index, &kvList, &_aidl_return); \
        UT_LOG_INFO("Result getPropertyMulti [%s] (IN:planeResourceIndex:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s])", \
                    label, index, \
                    UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(_aidl_return); \
        UT_ASSERT_EQUAL(status.isOk(), true); \
        UT_ASSERT_EQUAL(status.exceptionCode(), ::android::binder::Status::EX_NONE); \
    } while(0)

#define ADD_PROP_TO_LIST(kvList, propEnum, intVal) { \
        ::com::rdk::hal::planecontrol::PropertyKVPair kvPair; \
        kvPair.property = propEnum; \
        kvPair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make< \
            ::com::rdk::hal::PropertyValue::Value::Tag::intValue>(intVal); \
        kvList.push_back(kvPair); \
    }

// Macro to log and assert API call status and result
#define PLANE_CONTROL_ASSERT_STATUS(api_name, status, result, plane_index, ...) \
    do { \
        UT_LOG_INFO("Result %s(planeIndex:[%d], " #__VA_ARGS__ ", OUT:result:[%s], status:[isOK:%s, exception:%s])", \
        #api_name, plane_index, UT_Control_GetMapString(bool_mapTable, result), \
        UT_Control_GetMapString(bool_mapTable, status.isOk()), \
        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "Failed to call " #api_name " for plane " << plane_index; \
        UT_ASSERT_TRUE(result) << "Result failed for " #api_name " for plane " << plane_index; \
    } while (0)

#define PLANE_CONTROL_SET_GET_CONFIG_UINT32_PROPERTY(PLANE_INDEX, PROP, PROP_NAME, PATH_SUFFIX) \
    do { \
        std::string propPath = "IPlaneControl.planes." + std::to_string(PLANE_INDEX) + "." #PATH_SUFFIX; \
        uint32_t value = UT_KVP_PROFILE_GET_UINT32(propPath.c_str()); \
        ::com::rdk::hal::PropertyValue propertyValue; \
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(value); \
        bool setResult; \
        UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[%s], IN:propertyValue:[%d], OUT:setResult)", \
                    PLANE_INDEX, #PROP_NAME, value); \
        auto status = gPlaneControlservice->setProperty(PLANE_INDEX, ::com::rdk::hal::planecontrol::Property::PROP, propertyValue, &setResult); \
        PLANE_CONTROL_ASSERT_STATUS(setProperty, status, setResult, PLANE_INDEX, PROP_NAME); \
        std::optional<::com::rdk::hal::PropertyValue> getPropertyValue; \
        UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue)", \
                    PLANE_INDEX, #PROP_NAME); \
        status = gPlaneControlservice->getProperty(PLANE_INDEX, ::com::rdk::hal::planecontrol::Property::PROP, &getPropertyValue); \
        UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])", \
                    PLANE_INDEX, #PROP_NAME, \
                    getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "Failed to get property " #PROP_NAME " for plane " << PLANE_INDEX; \
        UT_ASSERT_TRUE(getPropertyValue.has_value()) << "No value returned for property " #PROP_NAME " for plane " << PLANE_INDEX; \
        UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), value) \
            << "Value mismatch for property " #PROP_NAME " for plane " << PLANE_INDEX; \
    } while (0)

#define PLANE_CONTROL_SET_GET_CONFIG_BOOL_PROPERTY(PLANE_INDEX, PROP, PROP_NAME, PATH_SUFFIX) \
    do { \
        std::string propPath = "IPlaneControl.planes." + std::to_string(PLANE_INDEX) + "." #PATH_SUFFIX; \
        bool value = UT_KVP_PROFILE_GET_BOOL(propPath.c_str()); \
        ::com::rdk::hal::PropertyValue propertyValue; \
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(value); \
        bool setResult; \
        UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[%s], IN:propertyValue:[%d], OUT:setResult)", \
                    PLANE_INDEX, #PROP_NAME, value); \
        auto status = gPlaneControlservice->setProperty(PLANE_INDEX, ::com::rdk::hal::planecontrol::Property::PROP, propertyValue, &setResult); \
        PLANE_CONTROL_ASSERT_STATUS(setProperty, status, setResult, PLANE_INDEX, PROP_NAME); \
        std::optional<::com::rdk::hal::PropertyValue> getPropertyValue; \
        UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue)", \
                    PLANE_INDEX, #PROP_NAME); \
        status = gPlaneControlservice->getProperty(PLANE_INDEX, ::com::rdk::hal::planecontrol::Property::PROP, &getPropertyValue); \
        UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])", \
                    PLANE_INDEX, #PROP_NAME, \
                    getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "Failed to get property " #PROP_NAME " for plane " << PLANE_INDEX; \
        UT_ASSERT_TRUE(getPropertyValue.has_value()) << "No value returned for property " #PROP_NAME " for plane " << PLANE_INDEX; \
        UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), value) \
            << "Value mismatch for property " #PROP_NAME " for plane " << PLANE_INDEX; \
    } while (0)

#define PLANE_CONTROL_SET_GET_INT_ZERO_PROPERTY(PLANE_INDEX, PROP_ENUM)                                                      \
    do {                                                                                                           \
        PropertyValue propertyValue;                                                                              \
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<                                        \
            ::com::rdk::hal::PropertyValue::Value::intValue>(0);                                                 \
        bool setResult;                                                                                           \
        UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[%s], IN:propertyValue:[intValue:0])",  \
                    PLANE_INDEX, #PROP_ENUM);                                                                     \
        auto status = gPlaneControlservice->setProperty(PLANE_INDEX, PROP_ENUM, propertyValue, &setResult);      \
        UT_LOG_INFO("Result setProperty(IN:planeIndex:[%d], IN:property:[%s], IN:propertyValue:[intValue:0], status:[isOK:%s, exception:%s], setResult:[%s])", \
                    PLANE_INDEX, #PROP_ENUM,                                                                      \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),                                       \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()),                   \
                    setResult ? "true" : "false");                                                                \
        UT_ASSERT_TRUE(status.isOk());                                                                            \
        UT_ASSERT_TRUE(setResult);                                                                                \
                                                                                                                  \
        std::optional<PropertyValue> getPropertyValue;                                                            \
        UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue)",           \
                    PLANE_INDEX, #PROP_ENUM);                                                                     \
        status = gPlaneControlservice->getProperty(PLANE_INDEX, PROP_ENUM, &getPropertyValue);                   \
        UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])", \
                    PLANE_INDEX, #PROP_ENUM,                                                                      \
                    getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(),        \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),                                       \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));                  \
        UT_ASSERT_TRUE(status.isOk());                                                                            \
        UT_ASSERT_TRUE(getPropertyValue.has_value());                                                             \
        UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(),    \
                        propertyValue.value);                                                                     \
    } while (0)

#define PLANE_CONTROL_SET_GET_INT_ONE_PROPERTY(PLANE_INDEX, PROP_ENUM)                                                      \
    do {                                                                                                           \
        PropertyValue propertyValue;                                                                              \
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<                                        \
            ::com::rdk::hal::PropertyValue::Value::intValue>(1);                                                 \
        bool setResult;                                                                                           \
        UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[%s], IN:propertyValue:[intValue:1])",  \
                    PLANE_INDEX, #PROP_ENUM);                                                                     \
        auto status = gPlaneControlservice->setProperty(PLANE_INDEX, PROP_ENUM, propertyValue, &setResult);      \
        UT_LOG_INFO("Result setProperty(IN:planeIndex:[%d], IN:property:[%s], IN:propertyValue:[intValue:1], status:[isOK:%s, exception:%s], setResult:[%s])", \
                    PLANE_INDEX, #PROP_ENUM,                                                                      \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),                                       \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()),                   \
                    setResult ? "true" : "false");                                                                \
        UT_ASSERT_TRUE(status.isOk());                                                                            \
        UT_ASSERT_TRUE(setResult);                                                                                \
                                                                                                                  \
        std::optional<PropertyValue> getPropertyValue;                                                            \
        UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue)",           \
                    PLANE_INDEX, #PROP_ENUM);                                                                     \
        status = gPlaneControlservice->getProperty(PLANE_INDEX, PROP_ENUM, &getPropertyValue);                   \
        UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])", \
                    PLANE_INDEX, #PROP_ENUM,                                                                      \
                    getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(),        \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),                                       \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));                  \
        UT_ASSERT_TRUE(status.isOk());                                                                            \
        UT_ASSERT_TRUE(getPropertyValue.has_value());                                                             \
        UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(),    \
                        propertyValue.value);                                                                     \
    } while (0)

// Macro to set and verify video source destination plane mapping
#define PLANE_CONTROL_SET_GET_MAPPING(source_type, plane_index_1, plane_index_2) \
    do { \
        std::vector<SourcePlaneMapping> unmapMappings; \
        for (const auto& unmapSource : sAllSourceTypes) { \
            for (int sourceIndex = 0; sourceIndex <= 1; ++sourceIndex) { \
                SourcePlaneMapping unmap; \
                unmap.sourceType = unmapSource; \
                unmap.sourceIndex = sourceIndex; \
                unmap.destinationPlaneIndex = -1; \
                unmapMappings.push_back(unmap); \
            } \
        } \
        bool mappingResult; \
        UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping to unmap all sources for planes:[%d,%d]", \
                    plane_index_1, plane_index_2); \
        auto status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(unmapMappings, &mappingResult); \
        PLANE_CONTROL_ASSERT_STATUS(setVideoSourceDestinationPlaneMapping, status, mappingResult, -1, unmap); \
        std::vector<SourcePlaneMapping> sourcePlaneMappings; \
        SourcePlaneMapping mapdata_1; \
        mapdata_1.sourceType = source_type; \
        mapdata_1.sourceIndex = 0; \
        mapdata_1.destinationPlaneIndex = plane_index_1; \
        SourcePlaneMapping mapdata_2; \
        mapdata_2.sourceType = source_type; \
        mapdata_2.sourceIndex = 1; \
        mapdata_2.destinationPlaneIndex = plane_index_2; \
        sourcePlaneMappings = {mapdata_1, mapdata_2}; \
        UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping(IN:sourceType:%d, planes:[%d,%d])", \
                    static_cast<int>(source_type), plane_index_1, plane_index_2); \
        status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(sourcePlaneMappings, &mappingResult); \
        PLANE_CONTROL_ASSERT_STATUS(setVideoSourceDestinationPlaneMapping, status, mappingResult, -1, sourceType); \
        std::vector<SourcePlaneMapping> retrievedMappings; \
        UT_LOG_INFO("Calling getVideoSourceDestinationPlaneMapping(OUT:retrievedMappings[])"); \
        status = gPlaneControlservice->getVideoSourceDestinationPlaneMapping(&retrievedMappings); \
        UT_LOG_INFO("Result getVideoSourceDestinationPlaneMapping(OUT:retrievedMappings[], status:[isOK:%s, exception:%s])", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "Failed to get mappings for sourceType " << static_cast<int>(source_type); \
        for (const auto& mapping : sourcePlaneMappings) { \
            auto it = std::find_if(retrievedMappings.begin(), retrievedMappings.end(), \
                                [&mapping](const SourcePlaneMapping& retrieved) { \
                                    return retrieved.destinationPlaneIndex == mapping.destinationPlaneIndex && \
                                           retrieved.sourceType == mapping.sourceType && \
                                           retrieved.sourceIndex == mapping.sourceIndex; \
                                }); \
            UT_ASSERT_NOT_EQUAL(it, retrievedMappings.end()) << "Mapping not found for sourceType " << static_cast<int>(source_type) \
                                                            << ", sourceIndex " << mapping.sourceIndex \
                                                            << ", plane " << mapping.destinationPlaneIndex; \
            UT_ASSERT_EQUAL(it->sourceType, mapping.sourceType) << "SourceType mismatch for plane " << mapping.destinationPlaneIndex; \
            UT_ASSERT_EQUAL(it->sourceIndex, mapping.sourceIndex) << "SourceIndex mismatch for plane " << mapping.destinationPlaneIndex; \
        } \
    } while (0)

#endif //__VTS_PLANECONTROL_H__

/** @} */ // End of Plane_Control_Halif_Test_Header
/** @} */ // End of Plane_Control_Halif_Test
/** @} */ // End of Plane_Control
/** @} */ // End of HPK