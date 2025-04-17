
/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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

#include "com/rdk/hal/planecontrol/IPlaneControl.h"
#include "com/rdk/hal/planecontrol/BnPlaneControlListener.h"

using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;

using namespace com::rdk::hal;
using namespace com::rdk::hal::videodecoder;
using namespace com::rdk::hal::planecontrol;

bool ValidatePropertyList(uint32_t planeIndex, const std::vector<::com::rdk::hal::planecontrol::Property>& expectedProperties, const std::string& planeType);
sp<IPlaneControl> service;

/* Status::Exception */
const static ut_control_keyStringMapping_t statusException_mapTable[] =
{
    {"EX_NONE",                  (int32_t) Status::EX_NONE},
    {"EX_SECURITY",              (int32_t) Status::EX_SECURITY},
    {"EX_BAD_PARCELABLE",        (int32_t) Status::EX_BAD_PARCELABLE},
    {"EX_ILLEGAL_ARGUMENT",      (int32_t) Status::EX_ILLEGAL_ARGUMENT},
    {"EX_NULL_POINTER",          (int32_t) Status::EX_NULL_POINTER},
    {"EX_ILLEGAL_STATE",         (int32_t) Status::EX_ILLEGAL_STATE},
    {"EX_NETWORK_MAIN_THREAD",   (int32_t) Status::EX_NETWORK_MAIN_THREAD},
    {"EX_UNSUPPORTED_OPERATION", (int32_t) Status::EX_UNSUPPORTED_OPERATION},
    {"EX_SERVICE_SPECIFIC",      (int32_t) Status::EX_SERVICE_SPECIFIC},
    {"EX_PARCELABLE",            (int32_t) Status::EX_PARCELABLE},
    {"EX_HAS_REPLY_HEADER",      (int32_t) Status::EX_HAS_REPLY_HEADER},
    {"EX_TRANSACTION_FAILED",    (int32_t) Status::EX_TRANSACTION_FAILED},
    {NULL, -1}
};

/* bool */
const static ut_control_keyStringMapping_t bool_mapTable[] =
{
    {"false", (int32_t) false},
    {"true",  (int32_t) true},
    {NULL, -1}
};

// Macro to iterate over all planes and filter by type (VIDEO or GRAPHICS)
#define FOREACH_PLANE_BY_TYPE(planeCount, planeTypeString, desiredType, planeIndexVar, body) \
    do { \
        std::string plane_count_path = "IPlaneControl.planes"; \
        planeCount = UT_KVP_PROFILE_GET_UINT32(plane_count_path.c_str()); \
        for (int32_t i = 0; i < planeCount; ++i) { \
            char planeTypeString[256]; \
            std::string plane_type_path = "IPlaneControl.planes." + std::to_string(i) + ".type"; \
            UT_KVP_PROFILE_GET_STRING(plane_type_path.c_str(), planeTypeString); \
            if (std::string(planeTypeString) == desiredType) { \
                std::string plane_index_path = "IPlaneControl.planes." + std::to_string(i); \
                uint32_t planeIndexVar = UT_KVP_PROFILE_GET_UINT32(plane_index_path.c_str()); \
                body \
            } \
        } \
    } while (0)

// Macro to iterate over VIDEO planes
#define FOREACH_VIDEO_PLANE(planeCount, planeIndexVar, body) \
    FOREACH_PLANE_BY_TYPE(planeCount, planeTypeString, "VIDEO", planeIndexVar, body)

// Macro to iterate over GRAPHICS planes
#define FOREACH_GRAPHICS_PLANE(planeCount, planeIndexVar, body) \
    FOREACH_PLANE_BY_TYPE(planeCount, planeTypeString, "GRAPHICS", planeIndexVar, body)

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

// Helper function to log API calls and results
void LogApiCall(const std::string& apiName, const std::string& inputParams, const std::string& outputParams, const android::binder::Status& status, bool result) {
    UT_LOG_INFO("Calling %s(%s)", apiName.c_str(), inputParams.c_str());
    UT_LOG_INFO("Result %s(%s), status:[isOK:%s, exception:%s], result:[%s]",
                apiName.c_str(), outputParams.c_str(),
                UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()),
                result ? "true" : "false");
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
    auto status = service->getPropertyMulti(planeIndex, &propertyKVList, &aidl_return);

    std::string outputParams = "IN:planeResourceIndex:[" + std::to_string(planeIndex) + "], IN/OUT:propertyKVList[], OUT:_aidl_return:[" + (aidl_return ? "true" : "false") + "]";
    LogApiCall("getPropertyMulti with " + planeType + " plane index", inputParams, outputParams, status, aidl_return);

    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_TRUE(aidl_return);
    UT_ASSERT_FALSE(propertyKVList.empty());

    for (const auto& kvPair : propertyKVList) {
        UT_ASSERT_TRUE(kvPair.propertyValue.value.has_value());
    }

    return true;
}

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