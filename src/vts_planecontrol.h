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

// Global counters for VIDEO and GRAPHICS planes
static uint32_t gNoVideoTypePlanes = 0;
static uint32_t gNoGraphicsTypePlanes = 0;

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

// Macro to iterate over all planes and filter by type (VIDEO or GRAPHICS)
#define PLANE_CONTROL_FOREACH_PLANE_BY_TYPE(planeCount, planeTypeEnum, desiredType, planeIndexVar, body) \
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

// Macro to call and validate getCapabilities API
#define PLANE_CONTROL_GET_CAPABILITIES(capabilities_var) \
    do { \
        UT_LOG_INFO("Calling getCapabilities(OUT:capabilities[])"); \
        ::android::binder::Status status = gPlaneControlservice->getCapabilities(&capabilities_var); \
        UT_LOG_INFO("Result getCapabilities(OUT:capabilities[size=%zu]), status:[isOK:%s, exception:%s]", \
                    capabilities_var.size(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "getCapabilities status not OK"; \
        UT_ASSERT_FALSE(capabilities_var.empty()) << "Capabilities list is empty"; \
        UT_ASSERT_EQUAL(capabilities_var.size(), gPlaneControl_hfp.size()) << "Capabilities size mismatch with HFP"; \
    } while (0)

// Macro to call and validate getNativeGraphicsWindowHandle API (positive case)
#define PLANE_CONTROL_GET_NATIVE_GRAPHICS_HANDLE_SUCCESS(plane_index, native_handle) \
    do { \
        UT_LOG_INFO("Testing getNativeGraphicsWindowHandle for GRAPHICS plane index: %u", plane_index); \
        std::string input_params = "IN:planeResourceIndex:[" + std::to_string(plane_index) + "], OUT:nativeGraphicsWindowHandle[]"; \
        ::android::binder::Status status = gPlaneControlservice->getNativeGraphicsWindowHandle(plane_index, &native_handle); \
        std::string output_params = "IN:planeResourceIndex:[" + std::to_string(plane_index) + "], OUT:nativeGraphicsWindowHandle[0x" + std::to_string(native_handle) + "]"; \
        UT_LOG_INFO("getNativeGraphicsWindowHandle(%s) => %s, status:[isOK:%s, exception:%s]", \
                    input_params.c_str(), output_params.c_str(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "getNativeGraphicsWindowHandle failed for plane index: " << plane_index; \
        UT_ASSERT_TRUE(native_handle < 0) << "Handle is zero for plane index: " << plane_index; \
    } while (0)

// Macro to call and validate getNativeGraphicsWindowHandle API (negative case)
#define PLANE_CONTROL_GET_NATIVE_GRAPHICS_HANDLE_FAILURE(plane_index, native_handle, expected_exception) \
    do { \
        UT_LOG_INFO("Calling getNativeGraphicsWindowHandle(IN:planeResourceIndex:[%u], OUT:nativeGraphicsWindowHandle[])", plane_index); \
        ::android::binder::Status status = gPlaneControlservice->getNativeGraphicsWindowHandle(plane_index, &native_handle); \
        UT_LOG_INFO("Result getNativeGraphicsWindowHandle(IN:planeResourceIndex:[%u], OUT:nativeGraphicsWindowHandle[0x%llX]), status:[isOK:%s, exception:%s]", \
                    plane_index, native_handle, \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expected_exception) << "Expected exception " << #expected_exception << " for plane index " << plane_index; \
    } while (0)

// Macro to add a property to a PropertyKVPair list
#define PLANE_CONTROL_ADD_PROPERTY_KV(prop, value, value_type) \
    do { \
        ::com::rdk::hal::planecontrol::PropertyKVPair kv_pair; \
        kv_pair.property = ::com::rdk::hal::planecontrol::Property::prop; \
        kv_pair.propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::value_type>(value); \
        propertyKVList.push_back(kv_pair); \
    } while (0)



// Macro to set multiple properties atomically
#define PLANE_CONTROL_SET_PROPERTY_MULTI_ATOMIC(plane_index, prop_list) \
    do { \
        bool aidl_return = false; \
        std::string input_params = "IN:planeResourceIndex:[" + std::to_string(plane_index) + "], IN:propertyKVList[]"; \
        ::android::binder::Status status = gPlaneControlservice->setPropertyMultiAtomic(plane_index, prop_list, &aidl_return); \
        std::string output_params = "OUT:_aidl_return:[" + std::string(aidl_return ? "true" : "false") + "]"; \
        PLANE_CONTROL_LOG_API_CALL("setPropertyMultiAtomic", input_params, output_params, status, aidl_return); \
        UT_ASSERT_TRUE(status.isOk()) << "setPropertyMultiAtomic failed for plane index " << plane_index; \
        UT_ASSERT_TRUE(aidl_return) << "setPropertyMultiAtomic returned false for plane index " << plane_index; \
    } while (0)

#define PLANE_CONTROL_SET_PROPERTY_MULTI_ATOMIC_NEG(plane_index, prop_list, label) \
    do { \
        bool aidl_return = false; \
        UT_LOG_INFO("Calling setPropertyMultiAtomic [%s] (IN:planeResourceIndex:[%d], IN:propertyKVList[], OUT:_aidl_return)", label, plane_index); \
        ::android::binder::Status status = gPlaneControlservice->setPropertyMultiAtomic(plane_index, prop_list, &aidl_return); \
        std::string input_params = "IN:planeResourceIndex:[" + std::to_string(plane_index) + "], IN:propertyKVList[]"; \
        std::string output_params = "OUT:_aidl_return:[" + std::string(aidl_return ? "true" : "false") + "]"; \
        PLANE_CONTROL_LOG_API_CALL("setPropertyMultiAtomic", input_params, output_params, status, aidl_return); \
        UT_ASSERT_FALSE(aidl_return) << "setPropertyMultiAtomic should fail for " << label << " index " << plane_index; \
        UT_ASSERT_TRUE(status.isOk()) << "Status should be OK for " << label << " index " << plane_index; \
    } while (0)

// Macro to call and validate flipGraphicsBuffer API (positive case)
#define PLANE_CONTROL_FLIP_GRAPHICS_BUFFER_POS(plane_index, aidl_return) \
    do { \
        std::string input_params = "IN:planeResourceIndex:[" + std::to_string(plane_index) + "]"; \
        ::android::binder::Status status = gPlaneControlservice->flipGraphicsBuffer(plane_index, &aidl_return); \
        std::string output_params = "IN:planeResourceIndex:[" + std::to_string(plane_index) + "], OUT:_aidl_return:[" + (aidl_return ? "true" : "false") + "]"; \
        UT_LOG_INFO("flipGraphicsBuffer(%s) => %s, status:[isOK:%s, exception:%s]", \
                    input_params.c_str(), output_params.c_str(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "flipGraphicsBuffer failed for plane index: " << plane_index; \
        UT_ASSERT_TRUE(aidl_return) << "flipGraphicsBuffer returned false for plane index: " << plane_index; \
    } while (0)

// Macro to call and validate flipGraphicsBuffer API (negative case)
#define PLANE_CONTROL_FLIP_GRAPHICS_BUFFER_NEG(plane_index, aidl_return) \
    do { \
        UT_LOG_INFO("Calling flipGraphicsBuffer(IN:planeResourceIndex:[%u], OUT:_aidl_return)", plane_index); \
        ::android::binder::Status status = gPlaneControlservice->flipGraphicsBuffer(plane_index, &aidl_return); \
        UT_LOG_INFO("Result flipGraphicsBuffer(IN:planeResourceIndex:[%u], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    plane_index, aidl_return ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), ::android::binder::Status::EX_ILLEGAL_ARGUMENT) << "flipGraphicsBuffer should true for plane index: " << plane_index; \
        UT_ASSERT_FALSE(aidl_return) << "flipGraphicsBuffer _aidl_return should be false for plane index: " << plane_index; \
    } while (0)

// Macro to call and validate getVideoSourceDestinationPlaneMapping API (negative case with expected invalid mappings)
#define PLANE_CONTROL_GET_VIDEO_SOURCE_MAPPING_NEG(aidl_return, video_plane_count, label) \
    do { \
        UT_LOG_INFO("Calling getVideoSourceDestinationPlaneMapping [%s] (OUT:SourcePlaneMapping[])", label); \
        ::android::binder::Status status = gPlaneControlservice->getVideoSourceDestinationPlaneMapping(&aidl_return); \
        std::string input_params = ""; \
        std::string output_params = "OUT:SourcePlaneMapping[size:" + std::to_string(aidl_return.size()) + "]"; \
        PLANE_CONTROL_LOG_API_CALL("getVideoSourceDestinationPlaneMapping", input_params, output_params, status, status.isOk()); \
        if (video_plane_count > 0) { \
            ASSERT_TRUE(status.isOk()) << "getVideoSourceDestinationPlaneMapping failed for " << label; \
            ASSERT_EQ(aidl_return.size(), video_plane_count) << "Mismatch in mapping count for " << label << ": expected " << video_plane_count << ", got " << aidl_return.size(); \
            for (const auto& mapping : aidl_return) { \
                std::vector<::com::rdk::hal::planecontrol::Capabilities> capabilities; \
                PLANE_CONTROL_GET_CAPABILITIES(capabilities); \
                auto it = std::find_if(capabilities.begin(), capabilities.end(), \
                                       [&mapping](const auto& cap) { \
                                           return cap.planeIndex == mapping.destinationPlaneIndex && \
                                                  cap.type == ::com::rdk::hal::planecontrol::PlaneType::VIDEO; \
                                       }); \
                bool is_valid_source_type = std::find(std::vector<::com::rdk::hal::planecontrol::SourceType>{ \
                    ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK, \
                    ::com::rdk::hal::planecontrol::SourceType::HDMI, \
                    ::com::rdk::hal::planecontrol::SourceType::COMPOSITE, \
                    ::com::rdk::hal::planecontrol::SourceType::NONE \
                }.begin(), \
                std::vector<::com::rdk::hal::planecontrol::SourceType>{ \
                    ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK, \
                    ::com::rdk::hal::planecontrol::SourceType::HDMI, \
                    ::com::rdk::hal::planecontrol::SourceType::COMPOSITE, \
                    ::com::rdk::hal::planecontrol::SourceType::NONE \
                }.end(), mapping.sourceType) != \
                std::vector<::com::rdk::hal::planecontrol::SourceType>{ \
                    ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK, \
                    ::com::rdk::hal::planecontrol::SourceType::HDMI, \
                    ::com::rdk::hal::planecontrol::SourceType::COMPOSITE, \
                    ::com::rdk::hal::planecontrol::SourceType::NONE \
                }.end(); \
                bool is_valid_source_index = (mapping.sourceIndex >= 0 && mapping.sourceIndex <= 1); \
                ASSERT_TRUE(it == capabilities.end() || !is_valid_source_type || !is_valid_source_index) \
                    << "Invalid mapping for " << label << ": destinationPlaneIndex=" << mapping.destinationPlaneIndex \
                    << ", sourceType=" << static_cast<int>(mapping.sourceType) << ", sourceIndex=" << mapping.sourceIndex; \
            } \
        } else { \
            ASSERT_TRUE(status.isOk()) << "getVideoSourceDestinationPlaneMapping failed for " << label; \
            ASSERT_TRUE(aidl_return.empty()) << "Expected empty mappings for no VIDEO planes, got " << aidl_return.size(); \
        } \
    } while (0)

// Macro to call and validate registerListener API (positive case with expected success)
#define PLANE_CONTROL_REGISTER_LISTENER_POS(listener, aidl_return) \
    do { \
        UT_LOG_INFO("Calling registerListener(IN:listener:[0x%X], OUT:_aidl_return)", listener.get()); \
        ::android::binder::Status status = gPlaneControlservice->registerListener(listener, &aidl_return); \
        std::string input_params = "IN:listener:[0x" + std::to_string(reinterpret_cast<uintptr_t>(listener.get())) + "]"; \
        std::string output_params = "OUT:_aidl_return:[" + std::string(aidl_return ? "true" : "false") + "]"; \
        PLANE_CONTROL_LOG_API_CALL("registerListener", input_params, output_params, status, aidl_return); \
        UT_ASSERT_TRUE(status.isOk()) << "registerListener failed for listener " << listener.get(); \
        UT_ASSERT_TRUE(aidl_return) << "registerListener returned false for listener " << listener.get(); \
    } while (0)

// Macro to call and validate registerListener API (negative case with expected failure)
#define PLANE_CONTROL_REGISTER_LISTENER_NEG(listener, aidl_return, label) \
    do { \
        UT_LOG_INFO("Calling registerListener [%s] (IN:listener:[0x%X], OUT:_aidl_return)", label, listener.get()); \
        ::android::binder::Status status = gPlaneControlservice->registerListener(listener, &aidl_return); \
        std::string input_params = "IN:listener:[0x" + (listener ? std::to_string(reinterpret_cast<uintptr_t>(listener.get())) : "nullptr") + "]"; \
        std::string output_params = "OUT:_aidl_return:[" + std::string(aidl_return ? "true" : "false") + "]"; \
        PLANE_CONTROL_LOG_API_CALL("registerListener", input_params, output_params, status, aidl_return); \
        UT_ASSERT_FALSE(status.isOk()) << "registerListener should fail for " << label; \
        UT_ASSERT_FALSE(aidl_return) << "registerListener should return false for " << label; \
    } while (0)

// Macro to call and validate unregisterListener API (positive case with expected success)
#define PLANE_CONTROL_UNREGISTER_LISTENER_POS(listener, aidl_return) \
    do { \
        UT_LOG_INFO("Calling unregisterListener(IN:listener:[0x%X], OUT:_aidl_return)", listener.get()); \
        ::android::binder::Status status = gPlaneControlservice->unregisterListener(listener, &aidl_return); \
        std::string input_params = "IN:listener:[0x" + std::to_string(reinterpret_cast<uintptr_t>(listener.get())) + "]"; \
        std::string output_params = "OUT:_aidl_return:[" + std::string(aidl_return ? "true" : "false") + "]"; \
        PLANE_CONTROL_LOG_API_CALL("unregisterListener", input_params, output_params, status, aidl_return); \
        UT_ASSERT_TRUE(status.isOk()) << "unregisterListener failed for listener " << listener.get(); \
        UT_ASSERT_TRUE(aidl_return) << "unregisterListener returned false for listener " << listener.get(); \
    } while (0)

// Macro to call and validate unregisterListener API (negative case with expected failure)
#define PLANE_CONTROL_UNREGISTER_LISTENER_NEG(listener, aidl_return, label) \
    do { \
        UT_LOG_INFO("Calling unregisterListener [%s] (IN:listener:[0x%X], OUT:_aidl_return)", label, listener.get()); \
        ::android::binder::Status status = gPlaneControlservice->unregisterListener(listener, &aidl_return); \
        std::string input_params = "IN:listener:[0x" + (listener ? std::to_string(reinterpret_cast<uintptr_t>(listener.get())) : "nullptr") + "]"; \
        std::string output_params = "OUT:_aidl_return:[" + std::string(aidl_return ? "true" : "false") + "]"; \
        PLANE_CONTROL_LOG_API_CALL("unregisterListener", input_params, output_params, status, aidl_return); \
        UT_ASSERT_FALSE(status.isOk()) << "unregisterListener should fail for " << label; \
        UT_ASSERT_FALSE(aidl_return) << "unregisterListener should return false for " << label; \
    } while (0)

// Macro to call and validate releaseNativeGraphicsWindowHandle API (positive case with expected success)
#define PLANE_CONTROL_RELEASE_NATIVE_GRAPHICS_WINDOW_POS(plane_index, native_handle) \
    do { \
        UT_LOG_INFO("Calling releaseNativeGraphicsWindowHandle(IN:planeResourceIndex:[%u], IN:nativeWindowHandle:[0x%llX])", plane_index, native_handle); \
        ::android::binder::Status status = gPlaneControlservice->releaseNativeGraphicsWindowHandle(plane_index, native_handle); \
        std::string input_params = "IN:planeResourceIndex:[" + std::to_string(plane_index) + "], IN:nativeWindowHandle:[0x" + std::to_string(native_handle) + "]"; \
        std::string output_params = input_params; \
        PLANE_CONTROL_LOG_API_CALL("releaseNativeGraphicsWindowHandle", input_params, output_params, status, status.isOk()); \
        UT_ASSERT_TRUE(status.isOk()) << "releaseNativeGraphicsWindowHandle failed for plane index " << plane_index << " and handle " << native_handle; \
    } while (0)

// Macro to call and validate releaseNativeGraphicsWindowHandle API (negative case with expected failure)
#define PLANE_CONTROL_RELEASE_NATIVE_GRAPHICS_WINDOW_NEG(plane_index, native_handle, label) \
    do { \
        UT_LOG_INFO("Calling releaseNativeGraphicsWindowHandle [%s] (IN:planeResourceIndex:[%u], IN:nativeWindowHandle:[0x%llX])", label, plane_index, native_handle); \
        ::android::binder::Status status = gPlaneControlservice->releaseNativeGraphicsWindowHandle(plane_index, native_handle); \
        std::string input_params = "IN:planeResourceIndex:[" + std::to_string(plane_index) + "], IN:nativeWindowHandle:[0x" + std::to_string(native_handle) + "]"; \
        std::string output_params = input_params; \
        PLANE_CONTROL_LOG_API_CALL("releaseNativeGraphicsWindowHandle", input_params, output_params, status, status.isOk()); \
        UT_ASSERT_FALSE(status.isOk()) << "releaseNativeGraphicsWindowHandle should fail for " << label; \
        ASSERT_EQ(status.exceptionCode(), ::android::binder::Status::EX_ILLEGAL_ARGUMENT) << "Expected EX_ILLEGAL_ARGUMENT for " << label; \
    } while (0)

// Macro to iterate over VIDEO planes
#define PLANE_CONTROL_FOREACH_VIDEO_PLANE(planeCount, planeIndexVar, body) \
    PLANE_CONTROL_FOREACH_PLANE_BY_TYPE(planeCount, planeTypeEnum, ::com::rdk::hal::planecontrol::PlaneType::VIDEO, planeIndexVar, body)

// Macro to iterate over GRAPHICS planes
#define PLANE_CONTROL_FOREACH_GRAPHICS_PLANE(planeCount, planeIndexVar, body) \
    PLANE_CONTROL_FOREACH_PLANE_BY_TYPE(planeCount, planeTypeEnum, ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS, planeIndexVar, body)


// Macro to ensure at least one plane of the desired type exists
#define PLANE_CONTROL_ENSURE_PLANE_TYPE_EXISTS(planeCount, desiredType, failMessage) \
    do { \
        uint32_t count = 0; \
        PLANE_CONTROL_FOREACH_PLANE_BY_TYPE(count, planeTypeString, desiredType, planeIndexVar, { count++; }); \
        if (count == 0) { \
            UT_FAIL(failMessage); \
        } \
    } while (0)

// Macro to call and validate getVideoSourceDestinationPlaneMapping API
#define PLANE_CONTROL_GET_VIDEO_SOURCE_MAPPING(mappings, expected_count) \
    do { \
        UT_LOG_INFO("Calling getVideoSourceDestinationPlaneMapping(OUT:mappings[])"); \
        ::android::binder::Status status = gPlaneControlservice->getVideoSourceDestinationPlaneMapping(&mappings); \
        UT_LOG_INFO("Result getVideoSourceDestinationPlaneMapping(OUT:mappings[size=%zu]), status:[isOK:%s, exception:%s]", \
                    mappings.size(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "getVideoSourceDestinationPlaneMapping failed"; \
        UT_ASSERT_EQUAL(mappings.size(), expected_count) << "Number of mappings does not match expected count"; \
    } while (0)

// Macro to ensure at least N planes of the desired type exist
#define PLANE_CONTROL_ENSURE_MIN_PLANE_TYPE_COUNT(planeCount, desiredType, minCount, failMessage) \
    do { \
        uint32_t count = 0; \
        FOREACH_PLANE_BY_TYPE(count, planeTypeString, desiredType, planeIndexVar, { count++; }); \
        UT_ASSERT_GREATER_EQUAL(count, minCount); \
        if (count < minCount) { \
            UT_FAIL(failMessage); \
        } \
    } while (0)

#define PLANE_CONTROL_LOG_API_CALL(apiName, inputParams, outputParams, status, result) \
    do { \
        UT_LOG_INFO("Calling %s(%s)", apiName, (inputParams).c_str()); \
        UT_LOG_INFO("Result %s(%s), status:[isOK:%s, exception:%s], result:[%s]", \
                    apiName, (outputParams).c_str(), \
                    UT_Control_GetMapString(bool_mapTable, (status).isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, (status).exceptionCode()), \
                    (result) ? "true" : "false"); \
    } while (0)

#define PLANE_CONTROL_SET_AND_ASSERT_PROPERTY(planeResourceIndex, propertyEnum, intVal)                                              \
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
#define PLANE_CONTROL_BUILD_PROPERTY_KV_LIST(propertyList, kvList) \
    do { \
        kvList.clear(); \
        for (const auto& property : propertyList) { \
            ::com::rdk::hal::planecontrol::PropertyKVPair kvPair; \
            kvPair.property = property; \
            kvList.push_back(kvPair); \
        } \
    } while(0)

// Macro: Executes getPropertyMulti and asserts
#define PLANE_CONTROL_VALIDATE_GET_PROPERTY_MULTI(index, kvList, planeTypeLabel) \
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
#define PLANE_CONTROL_NEGATIVE_TEST_GET_PROPERTY_MULTI(index, kvList, label) \
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

#define PLANE_CONTROL_ADD_PROP_TO_LIST(kvList, propEnum, intVal) { \
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

		// Macro to set a uint32_t property from configuration
#define PLANE_CONTROL_SET_CONFIG_UINT32_PROPERTY(PLANE_INDEX, PROP, PROP_NAME, PATH_SUFFIX) \
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
    } while (0)

// Macro to get and verify a uint32_t property
#define PLANE_CONTROL_GET_CONFIG_UINT32_PROPERTY(PLANE_INDEX, PROP, PROP_NAME, EXPECTED_VALUE) \
    do { \
        std::optional<::com::rdk::hal::PropertyValue> getPropertyValue; \
        UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue)", \
                    PLANE_INDEX, #PROP_NAME); \
        auto status = gPlaneControlservice->getProperty(PLANE_INDEX, ::com::rdk::hal::planecontrol::Property::PROP, &getPropertyValue); \
        UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])", \
                    PLANE_INDEX, #PROP_NAME, \
                    getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "Failed to get property " #PROP_NAME " for plane " << PLANE_INDEX; \
        UT_ASSERT_TRUE(getPropertyValue.has_value()) << "No value returned for property " #PROP_NAME " for plane " << PLANE_INDEX; \
        UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), EXPECTED_VALUE) \
            << "Value mismatch for property " #PROP_NAME " for plane " << PLANE_INDEX; \
    } while (0)

// Macro to set a boolean property from configuration
#define PLANE_CONTROL_SET_CONFIG_BOOL_PROPERTY(PLANE_INDEX, PROP, PROP_NAME, PATH_SUFFIX) \
    do { \
        std::string propPath = "IPlaneControl.planes." + std::to_string(PLANE_INDEX) + "." #PATH_SUFFIX; \
        bool value = UT_KVP_PROFILE_GET_BOOL(propPath.c_str()); \
        ::com::rdk::hal::PropertyValue propertyValue; \
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>(static_cast<int32_t>(value)); \
        bool setResult; \
        UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[%s], IN:propertyValue:[%d], OUT:setResult)", \
                    PLANE_INDEX, #PROP_NAME, static_cast<int32_t>(value)); \
        auto status = gPlaneControlservice->setProperty(PLANE_INDEX, ::com::rdk::hal::planecontrol::Property::PROP, propertyValue, &setResult); \
        PLANE_CONTROL_ASSERT_STATUS(setProperty, status, setResult, PLANE_INDEX, PROP_NAME); \
    } while (0)

// Macro to get and verify a boolean property
#define PLANE_CONTROL_GET_CONFIG_BOOL_PROPERTY(PLANE_INDEX, PROP, PROP_NAME, EXPECTED_VALUE) \
    do { \
        std::optional<::com::rdk::hal::PropertyValue> getPropertyValue; \
        UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue)", \
                    PLANE_INDEX, #PROP_NAME); \
        auto status = gPlaneControlservice->getProperty(PLANE_INDEX, ::com::rdk::hal::planecontrol::Property::PROP, &getPropertyValue); \
        UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])", \
                    PLANE_INDEX, #PROP_NAME, \
                    getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "Failed to get property " #PROP_NAME " for plane " << PLANE_INDEX; \
        UT_ASSERT_TRUE(getPropertyValue.has_value()) << "No value returned for property " #PROP_NAME " for plane " << PLANE_INDEX; \
        UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), static_cast<int32_t>(EXPECTED_VALUE)) \
            << "Value mismatch for property " #PROP_NAME " for plane " << PLANE_INDEX; \
    } while (0)

// set and get a boolean property from configuration
#define PLANE_CONTROL_SET_GET_CONFIG_BOOL_PROPERTY(PLANE_INDEX, PROP, PROP_NAME, PATH_SUFFIX) \
    do { \
        std::string propPath = "IPlaneControl.planes." + std::to_string(PLANE_INDEX) + "." #PATH_SUFFIX; \
        bool value = UT_KVP_PROFILE_GET_BOOL(propPath.c_str()); \
        PLANE_CONTROL_SET_CONFIG_BOOL_PROPERTY(PLANE_INDEX, PROP, PROP_NAME, PATH_SUFFIX); \
        PLANE_CONTROL_GET_CONFIG_BOOL_PROPERTY(PLANE_INDEX, PROP, PROP_NAME, value); \
    } while (0)

// Macro to set a property to int32_t value 0
#define PLANE_CONTROL_SET_INT_ZERO_PROPERTY(PLANE_INDEX, PROP_ENUM, PROP_NAME) \
    do { \
        ::com::rdk::hal::PropertyValue propertyValue; \
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>((0)); \
        bool setResult; \
        UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[%s], IN:propertyValue:[intValue:0])", \
                    PLANE_INDEX, #PROP_NAME); \
        auto status = gPlaneControlservice->setProperty(PLANE_INDEX, ::com::rdk::hal::planecontrol::Property::PROP_ENUM, propertyValue, &setResult); \
        PLANE_CONTROL_ASSERT_STATUS(setProperty, status, setResult, PLANE_INDEX, PROP_NAME); \
    } while (0)

// Macro to get and verify a property with int32_t value 0
#define PLANE_CONTROL_GET_INT_ZERO_PROPERTY(PLANE_INDEX, PROP_ENUM, PROP_NAME) \
    do { \
        std::optional<::com::rdk::hal::PropertyValue> getPropertyValue; \
        UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue)", \
                    PLANE_INDEX, #PROP_NAME); \
        auto status = gPlaneControlservice->getProperty(PLANE_INDEX, ::com::rdk::hal::planecontrol::Property::PROP_ENUM, &getPropertyValue); \
        UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])", \
                    PLANE_INDEX, #PROP_NAME, \
                    getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "Failed to get property " #PROP_NAME " for plane " << PLANE_INDEX; \
        UT_ASSERT_TRUE(getPropertyValue.has_value()) << "No value returned for property " #PROP_NAME " for plane " << PLANE_INDEX; \
        UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), (0)) \
            << "Value mismatch for property " #PROP_NAME " for plane " << PLANE_INDEX; \
    } while (0)

// Macro to set a property to int32_t value 1
#define PLANE_CONTROL_SET_INT_ONE_PROPERTY(PLANE_INDEX, PROP_ENUM, PROP_NAME) \
    do { \
        ::com::rdk::hal::PropertyValue propertyValue; \
        propertyValue.value = ::com::rdk::hal::PropertyValue::Value::make<::com::rdk::hal::PropertyValue::Value::intValue>((1)); \
        bool setResult; \
        UT_LOG_INFO("Calling setProperty(IN:planeIndex:[%d], IN:property:[%s], IN:propertyValue:[intValue:1])", \
                    PLANE_INDEX, #PROP_NAME); \
        auto status = gPlaneControlservice->setProperty(PLANE_INDEX, ::com::rdk::hal::planecontrol::Property::PROP_ENUM, propertyValue, &setResult); \
        PLANE_CONTROL_ASSERT_STATUS(setProperty, status, setResult, PLANE_INDEX, PROP_NAME); \
    } while (0)

// Macro to get and verify a property with int32_t value 1
#define PLANE_CONTROL_GET_INT_ONE_PROPERTY(PLANE_INDEX, PROP_ENUM, PROP_NAME) \
    do { \
        std::optional<::com::rdk::hal::PropertyValue> getPropertyValue; \
        UT_LOG_INFO("Calling getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue)", \
                    PLANE_INDEX, #PROP_NAME); \
        auto status = gPlaneControlservice->getProperty(PLANE_INDEX, ::com::rdk::hal::planecontrol::Property::PROP_ENUM, &getPropertyValue); \
        UT_LOG_INFO("Result getProperty(IN:planeIndex:[%d], IN:property:[%s], OUT:getPropertyValue:[%d], status:[isOK:%s, exception:%s])", \
                    PLANE_INDEX, #PROP_NAME, \
                    getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "Failed to get property " #PROP_NAME " for plane " << PLANE_INDEX; \
        UT_ASSERT_TRUE(getPropertyValue.has_value()) << "No value returned for property " #PROP_NAME " for plane " << PLANE_INDEX; \
        UT_ASSERT_EQUAL(getPropertyValue->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>(), (1)) \
            << "Value mismatch for property " #PROP_NAME " for plane " << PLANE_INDEX; \
    } while (0)

// Helper function to get string representation of Property enum
static const char* PropertyToString(::com::rdk::hal::planecontrol::Property prop) {
    switch (prop) {
        case ::com::rdk::hal::planecontrol::Property::RESOURCE_ID: return "RESOURCE_ID";
        case ::com::rdk::hal::planecontrol::Property::X: return "X";
        case ::com::rdk::hal::planecontrol::Property::Y: return "Y";
        case ::com::rdk::hal::planecontrol::Property::WIDTH: return "WIDTH";
        case ::com::rdk::hal::planecontrol::Property::HEIGHT: return "HEIGHT";
        case ::com::rdk::hal::planecontrol::Property::ZORDER: return "ZORDER";
        case ::com::rdk::hal::planecontrol::Property::ALPHA: return "ALPHA";
        case ::com::rdk::hal::planecontrol::Property::ASPECT_RATIO: return "ASPECT_RATIO";
        default: return "UNKNOWN";
    }
}

// Macro to call and validate setProperty API (positive case)
#define PLANE_CONTROL_SET_PROPERTY_POS(plane_index, property, property_value, aidl_return) \
    do { \
        UT_LOG_INFO("Calling setProperty(IN:planeResourceIndex:[%d], IN:property:[%s], IN:propertyValue:[%s], OUT:aidl_return)", \
                    plane_index, PropertyToString(property), \
                    property_value.value.has_value() ? "valid" : "null"); \
        ::android::binder::Status status = gPlaneControlservice->setProperty(plane_index, property, property_value, &aidl_return); \
        UT_LOG_INFO("Result setProperty(IN:planeResourceIndex:[%d], IN:property:[%s], IN:propertyValue:[%s], OUT:aidl_return:[%s], status:[isOK:%s, exception:%s])", \
                    plane_index, PropertyToString(property), \
                    property_value.value.has_value() ? "valid" : "null", \
                    aidl_return ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "setProperty failed for plane index " << plane_index << ", property " << PropertyToString(property); \
        UT_ASSERT_TRUE(aidl_return) << "setProperty returned false for plane index " << plane_index << ", property " << PropertyToString(property); \
    } while (0)

// Macro to call and validate setProperty API (negative case)
#define PLANE_CONTROL_SET_PROPERTY_NEG(plane_index, property, property_value, aidl_return, expected_exception) \
    do { \
        UT_LOG_INFO("Calling setProperty(IN:planeResourceIndex:[%d], IN:property:[%s], IN:propertyValue:[%s], OUT:aidl_return)", \
                    plane_index, PropertyToString(property), \
                    property_value.value.has_value() ? "valid" : "null"); \
        ::android::binder::Status status = gPlaneControlservice->setProperty(plane_index, property, property_value, &aidl_return); \
        UT_LOG_INFO("Result setProperty(IN:planeResourceIndex:[%d], IN:property:[%s], IN:propertyValue:[%s], OUT:aidl_return:[%s], status:[isOK:%s, exception:%s])", \
                    plane_index, PropertyToString(property), \
                    property_value.value.has_value() ? "valid" : "null", \
                    aidl_return ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(aidl_return) << "setProperty should return false for plane index " << plane_index << ", property " << PropertyToString(property); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expected_exception) << "Expected exception " << #expected_exception << " for plane index " << plane_index; \
    } while (0)

// Macro to call and validate getProperty API (positive case)
#define PLANE_CONTROL_GET_PROPERTY_POS(plane_index, property, property_value) \
    do { \
        UT_LOG_INFO("Calling getProperty(IN:planeResourceIndex:[%d], IN:property:[%s], OUT:propertyValue)", \
                    plane_index, PropertyToString(property)); \
        ::android::binder::Status status = gPlaneControlservice->getProperty(plane_index, property, &property_value); \
        UT_LOG_INFO("Result getProperty(IN:planeResourceIndex:[%d], IN:property:[%s], OUT:propertyValue:[%s], status:[isOK:%s, exception:%s])", \
                    plane_index, PropertyToString(property), \
                    property_value.has_value() ? "valid" : "null", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "getProperty failed for plane index " << plane_index << ", property " << PropertyToString(property); \
        UT_ASSERT_TRUE(property_value.has_value()) << "No value returned for property " << PropertyToString(property) << " for plane " << plane_index; \
    } while (0)

// Macro to call and validate getProperty API (negative case)
#define PLANE_CONTROL_GET_PROPERTY_NEG(plane_index, property, property_value, expected_exception) \
    do { \
        UT_LOG_INFO("Calling getProperty(IN:planeResourceIndex:[%d], IN:property:[%s], OUT:propertyValue)", \
                    plane_index, PropertyToString(property)); \
        ::android::binder::Status status = gPlaneControlservice->getProperty(plane_index, property, &property_value); \
        UT_LOG_INFO("Result getProperty(IN:planeResourceIndex:[%d], IN:property:[%s], OUT:propertyValue:[%s], status:[isOK:%s, exception:%s])", \
                    plane_index, PropertyToString(property), \
                    property_value.has_value() ? "valid" : "null", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()) << "getProperty should fail for plane index " << plane_index << ", property " << PropertyToString(property); \
        UT_ASSERT_EQUAL(status.exceptionCode(), expected_exception) << "Expected exception " << #expected_exception << " for plane index " << plane_index; \
        UT_ASSERT_FALSE(property_value.has_value()) << "Property value should be null for plane index " << plane_index << ", property " << PropertyToString(property); \
    } while (0)

// Macro to set video source destination plane mapping
#define PLANE_CONTROL_SET_MAPPING(SOURCE_TYPE, PLANE_INDEX_1, PLANE_INDEX_2) \
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
                    PLANE_INDEX_1, PLANE_INDEX_2); \
        auto status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(unmapMappings, &mappingResult); \
        PLANE_CONTROL_ASSERT_STATUS(setVideoSourceDestinationPlaneMapping, status, mappingResult, -1, unmap); \
        std::vector<SourcePlaneMapping> sourcePlaneMappings; \
        SourcePlaneMapping mapdata_1; \
        mapdata_1.sourceType = SOURCE_TYPE; \
        mapdata_1.sourceIndex = 0; \
        mapdata_1.destinationPlaneIndex = PLANE_INDEX_1; \
        SourcePlaneMapping mapdata_2; \
        mapdata_2.sourceType = SOURCE_TYPE; \
        mapdata_2.sourceIndex = 1; \
        mapdata_2.destinationPlaneIndex = PLANE_INDEX_2; \
        sourcePlaneMappings = {mapdata_1, mapdata_2}; \
        UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping(IN:sourceType:%d, planes:[%d,%d])", \
                    static_cast<int>(SOURCE_TYPE), PLANE_INDEX_1, PLANE_INDEX_2); \
        status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(sourcePlaneMappings, &mappingResult); \
        PLANE_CONTROL_ASSERT_STATUS(setVideoSourceDestinationPlaneMapping, status, mappingResult, -1, sourceType); \
    } while (0)

// Macro to get and verify video source destination plane mapping
#define PLANE_CONTROL_GET_MAPPING(SOURCE_TYPE, PLANE_INDEX_1, PLANE_INDEX_2) \
    do { \
        std::vector<SourcePlaneMapping> sourcePlaneMappings; \
        SourcePlaneMapping mapdata_1; \
        mapdata_1.sourceType = SOURCE_TYPE; \
        mapdata_1.sourceIndex = 0; \
        mapdata_1.destinationPlaneIndex = PLANE_INDEX_1; \
        SourcePlaneMapping mapdata_2; \
        mapdata_2.sourceType = SOURCE_TYPE; \
        mapdata_2.sourceIndex = 1; \
        mapdata_2.destinationPlaneIndex = PLANE_INDEX_2; \
        sourcePlaneMappings = {mapdata_1, mapdata_2}; \
        std::vector<SourcePlaneMapping> retrievedMappings; \
        UT_LOG_INFO("Calling getVideoSourceDestinationPlaneMapping(OUT:retrievedMappings[])"); \
        auto status = gPlaneControlservice->getVideoSourceDestinationPlaneMapping(&retrievedMappings); \
        UT_LOG_INFO("Result getVideoSourceDestinationPlaneMapping(OUT:retrievedMappings[], status:[isOK:%s, exception:%s])", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "Failed to get mappings for sourceType " << static_cast<int>(SOURCE_TYPE); \
        for (const auto& mapping : sourcePlaneMappings) { \
            auto it = std::find_if(retrievedMappings.begin(), retrievedMappings.end(), \
                                [&mapping](const SourcePlaneMapping& retrieved) { \
                                    return retrieved.destinationPlaneIndex == mapping.destinationPlaneIndex && \
                                           retrieved.sourceType == mapping.sourceType && \
                                           retrieved.sourceIndex == mapping.sourceIndex; \
                                }); \
            UT_ASSERT_NOT_EQUAL(it, retrievedMappings.end()) << "Mapping not found for sourceType " << static_cast<int>(SOURCE_TYPE) \
                                                            << ", sourceIndex " << mapping.sourceIndex \
                                                            << ", plane " << mapping.destinationPlaneIndex; \
            UT_ASSERT_EQUAL(it->sourceType, mapping.sourceType) << "SourceType mismatch for plane " << mapping.destinationPlaneIndex; \
            UT_ASSERT_EQUAL(it->sourceIndex, mapping.sourceIndex) << "SourceIndex mismatch for plane " << mapping.destinationPlaneIndex; \
        } \
    } while (0)

// Macro to add a source plane mapping to a list
#define PLANE_CONTROL_ADD_SOURCE_PLANE_MAPPING(mappings, src_type, src_index, dest_plane_index) \
    do { \
        ::com::rdk::hal::planecontrol::SourcePlaneMapping mapping; \
        mapping.sourceType = src_type; \
        mapping.sourceIndex = src_index; \
        mapping.destinationPlaneIndex = dest_plane_index; \
        mappings.push_back(mapping); \
    } while (0)

// Macro to call and validate setVideoSourceDestinationPlaneMapping API (positive case)
#define PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_POS(mappings, aidl_return) \
    do { \
        UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping(IN:mappings, OUT:_aidl_return)"); \
        ::android::binder::Status status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(mappings, &aidl_return); \
        UT_LOG_INFO("Result setVideoSourceDestinationPlaneMapping(IN:mappings, OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    aidl_return ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()) << "setVideoSourceDestinationPlaneMapping failed"; \
        UT_ASSERT_TRUE(aidl_return) << "setVideoSourceDestinationPlaneMapping returned false"; \
    } while (0)

// Macro to call and validate setVideoSourceDestinationPlaneMapping API (negative case with expected failure)
#define PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_NEG(mappings, aidl_return) \
    do { \
        UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping(IN:mappings, OUT:_aidl_return)"); \
        ::android::binder::Status status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(mappings, &aidl_return); \
        UT_LOG_INFO("Result setVideoSourceDestinationPlaneMapping(IN:mappings, OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    aidl_return ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(aidl_return) << "setVideoSourceDestinationPlaneMapping should return false"; \
    } while (0)

// Macro to call and validate setVideoSourceDestinationPlaneMapping API (negative case with expected exception)
#define PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_NEG_WITH_EXCEPTION(mappings, aidl_return, expected_exception) \
    do { \
        UT_LOG_INFO("Calling setVideoSourceDestinationPlaneMapping(IN:mappings, OUT:_aidl_return)"); \
        ::android::binder::Status status = gPlaneControlservice->setVideoSourceDestinationPlaneMapping(mappings, &aidl_return); \
        UT_LOG_INFO("Result setVideoSourceDestinationPlaneMapping(IN:mappings, OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    aidl_return ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(aidl_return) << "setVideoSourceDestinationPlaneMapping should return false"; \
        UT_ASSERT_EQUAL(status.exceptionCode(), expected_exception) << "Expected exception " << #expected_exception; \
    } while (0)

// Macro to verify a specific source plane mapping
#define PLANE_CONTROL_VERIFY_SOURCE_PLANE_MAPPING(retrieved_mappings, src_type, src_index, dest_plane_index) \
    do { \
        bool found = false; \
        for (const auto& mapping : retrieved_mappings) { \
            if (mapping.destinationPlaneIndex == dest_plane_index && \
                mapping.sourceType == ::com::rdk::hal::planecontrol::SourceType::src_type && \
                mapping.sourceIndex == src_index) { \
                found = true; \
                break; \
            } \
        } \
        UT_ASSERT_TRUE(found) << #src_type << " mapping not found for plane " << dest_plane_index; \
    } while (0)

// Macro to unmap all source types for cleanup
#define PLANE_CONTROL_UNMAP_ALL_SOURCES(unmap_mappings, aidl_return) \
    do { \
        std::vector<::com::rdk::hal::planecontrol::SourceType> allSourceTypes = { \
            ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK, \
            ::com::rdk::hal::planecontrol::SourceType::HDMI, \
            ::com::rdk::hal::planecontrol::SourceType::COMPOSITE \
        }; \
        unmap_mappings.clear(); \
        for (const auto& source : allSourceTypes) { \
            for (int sourceIndex = 0; sourceIndex <= 1; ++sourceIndex) { \
                PLANE_CONTROL_ADD_SOURCE_PLANE_MAPPING(unmap_mappings, source, sourceIndex, -1); \
            } \
        } \
        PLANE_CONTROL_SET_VIDEO_SOURCE_MAPPING_POS(unmap_mappings, aidl_return); \
    } while (0)


#endif //__VTS_PLANECONTROL_H__

/** @} */ // End of Plane_Control_Halif_Test_Header
/** @} */ // End of Plane_Control_Halif_Test
/** @} */ // End of Plane_Control
/** @} */ // End of HPK