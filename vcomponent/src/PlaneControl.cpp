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

/** @brief
 * PlaneControl.cpp
 *
 * The implementation file contains the dummy implementations for the PlaneControl service methods.
 */

#include "PlaneControl.h"

namespace com {
    namespace rdk {
        namespace hal {
            namespace planecontrol {


                int32_t g_plane_indexes_video[NUM_PLANES_VIDEO];
                int32_t g_plane_indexes_graphic[NUM_PLANES_GRAPHIC];

                ::com::rdk::hal::planecontrol::Capabilities PlaneControl::buildCapabilities(
                        int index,
                        ::com::rdk::hal::planecontrol::PlaneType type,
                        std::vector<::com::rdk::hal::videodecoder::PixelFormat> pixelFormats,
                        std::vector<::com::rdk::hal::planecontrol::SourceType> sourceTypes,
                        int maxWidth,
                        int maxHeight,
                        int frameWidth,
                        int frameHeight,
                        std::vector<::com::rdk::hal::videodecoder::DynamicRange> dynamicRanges,
                        int colorDepth,
                        int maxFrameRate,
                        bool supportsAlpha,
                        bool supportsZOrder,
                        int vsyncDisplayLatency
                ) {
                    Capabilities cap;
                    cap.planeIndex = index;
                    cap.type = type;
                    cap.pixelFormats = std::move(pixelFormats);
                    cap.sourceTypes = std::move(sourceTypes);
                    cap.maxWidth = maxWidth;
                    cap.maxHeight = maxHeight;
                    cap.frameWidth = frameWidth;
                    cap.frameHeight = frameHeight;
                    cap.supportedDynamicRanges = std::move(dynamicRanges);
                    cap.colorDepth = colorDepth;
                    cap.maxFrameRate = maxFrameRate;
                    cap.supportsAlpha = supportsAlpha;
                    cap.supportsZOrder = supportsZOrder;
                    cap.vsyncDisplayLatency = vsyncDisplayLatency;
                    return cap;
                }

                std::map<::com::rdk::hal::planecontrol::Property, ::com::rdk::hal::PropertyValue> PlaneControl::buildProperties(::com::rdk::hal::planecontrol::Capabilities capabilities)
                {
                    std::map<::com::rdk::hal::planecontrol::Property, ::com::rdk::hal::PropertyValue> defaults;

                    auto makeIntValue = [](int32_t val) {
                        ::com::rdk::hal::PropertyValue v;
                        v.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(val);
                        return v;
                    };
                    defaults[::com::rdk::hal::planecontrol::Property::RESOURCE_ID] = makeIntValue(capabilities.planeIndex);
                    defaults[::com::rdk::hal::planecontrol::Property::X] = makeIntValue(0);
                    defaults[::com::rdk::hal::planecontrol::Property::Y] = makeIntValue(0);
                    defaults[::com::rdk::hal::planecontrol::Property::WIDTH] = makeIntValue(capabilities.maxWidth);
                    defaults[::com::rdk::hal::planecontrol::Property::HEIGHT] = makeIntValue(capabilities.maxHeight);
                    defaults[::com::rdk::hal::planecontrol::Property::ALPHA] = makeIntValue(255);
                    defaults[::com::rdk::hal::planecontrol::Property::ZORDER] = makeIntValue(capabilities.planeIndex);
                    defaults[::com::rdk::hal::planecontrol::Property::VISIBLE] = makeIntValue(0);

                    if (capabilities.type == ::com::rdk::hal::planecontrol::PlaneType::VIDEO) {
                        // Defaults for grapics planes
                        defaults[::com::rdk::hal::planecontrol::Property::OVERSCAN] = makeIntValue(5);
                        defaults[::com::rdk::hal::planecontrol::Property::ASPECT_RATIO] = makeIntValue(0);
                    }

                    return defaults;
                }

                PlaneControl::PlaneControl()
                {
                    for (int i = 0; i < NUM_PLANES_VIDEO; i++) {
                        g_plane_indexes_video[i] = i;
                    }

                    for (int j = 0; j < NUM_PLANES_GRAPHIC; j++) {
                        g_plane_indexes_graphic[j] = j + NUM_PLANES_VIDEO;
                    }

                    for (int i = 0; i < NUM_PLANES_VIDEO; i++) {
                        auto cap = buildCapabilities(
                                g_plane_indexes_video[i],
                                ::com::rdk::hal::planecontrol::PlaneType::VIDEO,
                                {::com::rdk::hal::videodecoder::PixelFormat::NATIVE},
                                {::com::rdk::hal::planecontrol::SourceType::NONE},
                                3840, 2160, 3840, 2160,
                                {::com::rdk::hal::videodecoder::DynamicRange::SDR, ::com::rdk::hal::videodecoder::DynamicRange::HDR10, ::com::rdk::hal::videodecoder::DynamicRange::HDR10PLUS, ::com::rdk::hal::videodecoder::DynamicRange::HLG, ::com::rdk::hal::videodecoder::DynamicRange::DOLBY_VISION},
                                12, 60, true, true, 3
                        );
                        if (NUM_PLANES_VIDEO > 1 && i == 1) {  // For example, video plane 1 could have different properties.
                            cap.supportedDynamicRanges = {::com::rdk::hal::videodecoder::DynamicRange::SDR};
                            cap.colorDepth = 10;
                        }
                        mPlaneProperties[i] = buildProperties(cap);
                        mCapabilities.push_back(std::move(cap));

                    }
                        for (int i = 0; i < NUM_PLANES_GRAPHIC; i++) {
                            auto cap = buildCapabilities(
                                    g_plane_indexes_graphic[i],
                                    ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS,
                                    { ::com::rdk::hal::videodecoder::PixelFormat::RGB },
                                    { ::com::rdk::hal::planecontrol::SourceType::NONE },
                                    1920, 1080, 1920, 1080,
                                    { ::com::rdk::hal::videodecoder::DynamicRange::SDR },
                                    8, 60, true, false, 1
                            );

                            mPlaneProperties[i+NUM_PLANES_VIDEO] = buildProperties(cap);
                            mCapabilities.push_back(std::move(cap));
                        }


                    printf("[SERVICE] PlaneControl service initialized\n");
                }

                PlaneControl::~PlaneControl() {
                    printf("[SERVICE] PlaneControl service destroyed\n");
                }



                android::binder::Status PlaneControl::getCapabilities(::std::vector<::com::rdk::hal::planecontrol::Capabilities>* _aidl_return)
                {
                    *_aidl_return = mCapabilities;
                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::getNativeGraphicsWindowHandle(int32_t planeResourceIndex, int64_t *_aidl_return)
                {
                    std::lock_guard<std::mutex> lock(mHandleLock);

                    if (planeResourceIndex < 0 || planeResourceIndex >= (NUM_PLANES_GRAPHIC + NUM_PLANES_VIDEO)) {
                        std::cout << "[SERVICE] invalid planeResourceIndex "<< std::endl;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    // Check if the index is valid and belongs to a GRAPHICS plane
                    bool isGraphicPlane = false;
                    for (int i = 0; i < NUM_PLANES_GRAPHIC; ++i) {
                        if (g_plane_indexes_graphic[i] == planeResourceIndex) {
                            isGraphicPlane = true;
                            break;
                        }
                    }
                    if (!isGraphicPlane) {
                        std::cout << "[SERVICE] planeResourceIndex " << planeResourceIndex << " is not a GRAPHICS plane." << std::endl;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    auto it = mNativeWindowHandles.find(planeResourceIndex);
                    if (it != mNativeWindowHandles.end()) {
                        *_aidl_return = it->second;
                        std::cout << "[SERVICE] Returning existing handle: " << *_aidl_return << " for plane: " << planeResourceIndex << std::endl;
                        return android::binder::Status::ok();
                    }

                    //TBD
                    int64_t newHandle = mNextHandle++;
                    mNativeWindowHandles[planeResourceIndex] = newHandle;
                    *_aidl_return = newHandle;
                    std::cout << "[SERVICE] Created new handle: " << newHandle << " for plane: " << planeResourceIndex << std::endl;
                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::releaseNativeGraphicsWindowHandle(int32_t planeResourceIndex, int64_t nativeWindowHandle)
                {
                    std::lock_guard<std::mutex> lock(mHandleLock);

                    if (planeResourceIndex < 0 || planeResourceIndex >= (NUM_PLANES_GRAPHIC + NUM_PLANES_VIDEO)) {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT, "Invalid plane resource index");
                    }

                    auto it = mNativeWindowHandles.find(planeResourceIndex);
                    if (it == mNativeWindowHandles.end()) {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT, "No handle stored for given index");
                    }

                    if (it->second != nativeWindowHandle) {
                        return android::binder::Status::fromExceptionCode(
                                android::binder::Status::EX_ILLEGAL_ARGUMENT,
                                "Provided handle does not match stored handle");
                    }

                    mNativeWindowHandles.erase(it);
                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::flipGraphicsBuffer(int32_t planeResourceIndex, bool* _aidl_return)
                {
                    std::lock_guard<std::mutex> lock(mHandleLock);

                    if (planeResourceIndex < 0 || planeResourceIndex >= (NUM_PLANES_GRAPHIC + NUM_PLANES_VIDEO)) {
                        *_aidl_return = false;
                        std::cout << "[SERVICE] invalid planeResourceIndex "<< std::endl;
                        return android::binder::Status::ok();
                    }

                    //flip only if a native window handle exists for the plane
                    if (mNativeWindowHandles.find(planeResourceIndex) == mNativeWindowHandles.end()) {
                        *_aidl_return = false;
                        std::cout << "[SERVICE] NativeWindowHandle doesnt exist for the planeResourceIndex "<< std::endl;
                        return android::binder::Status::ok();
                    }

                    // Simulate the flip logic //TBD
                    *_aidl_return = true;
                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::setVideoSourceDestinationPlaneMapping(const ::std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping>& listSourcePlaneMapping, bool* _aidl_return)
                {
                    std::lock_guard<std::mutex> lock(mHandleLock);
                    // Sets to track duplicates in input
                    std::set<std::pair<SourceType, int>> seenSources;
                    std::set<int> seenPlanes;

                    // Check for invalid input: duplicate sources or planes
                    for (const auto& mapping : listSourcePlaneMapping)
                    {
                        if (mapping.destinationPlaneIndex >= NUM_PLANES_VIDEO)
                        {
                            *_aidl_return = false;
                            std::cout << "[SERVICE] video plane doesnt exist "<< std::endl;
                            return android::binder::Status::ok();
                        }
                        if (mapping.destinationPlaneIndex != -1)
                        {
                            if (!seenPlanes.insert(mapping.destinationPlaneIndex).second)
                            {
                                *_aidl_return = false;
                                std::cout << "[SERVICE] duplicate plane "<< std::endl;
                                return android::binder::Status::ok();
                            }
                        }
                        if (mapping.sourceType != SourceType::NONE)
                        {
                            auto key = std::make_pair(mapping.sourceType, mapping.sourceIndex);
                            if (!seenSources.insert(key).second)
                            {
                                *_aidl_return = false;
                                std::cout << "[SERVICE] duplicate sources "<< std::endl;
                                return android::binder::Status::ok();
                            }
                        }
                    }

                    // Check for conflicts: destination already mapped to another source
                    for (const auto& mapping : listSourcePlaneMapping)
                    {
                        if (mapping.destinationPlaneIndex != -1)
                        {
                            auto it = mVideoPlaneMappings.find(mapping.destinationPlaneIndex);
                            if (it != mVideoPlaneMappings.end())
                            {
                                const auto& currentMapping = it->second;
                                if (currentMapping.sourceType != SourceType::NONE && currentMapping.sourceIndex != -1)
                                {
                                    std::cout << "[SERVICE] Plane already mapped to another Source "<< std::endl;
                                    return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                                }
                            }
                        }
                    }

                    // Apply all valid mappings
                    for (const auto& mapping : listSourcePlaneMapping)
                    {
                        if (mapping.destinationPlaneIndex == -1) {
                            // Unmap the source: remove from any mapped plane
                            for (auto it = mVideoPlaneMappings.begin(); it != mVideoPlaneMappings.end();)
                            {
                                if (it->second.sourceType == mapping.sourceType &&
                                    it->second.sourceIndex == mapping.sourceIndex) {
                                    std::cout << "[SERVICE] unmapping destinationPlaneIndex  "<< std::endl;
                                    it = mVideoPlaneMappings.erase(it); // erase and move iterator
                                } else {
                                    ++it;
                                }
                            }
                        } else
                        {
                            // Map the source to the destination plane
                            std::cout << "[SERVICE] Map source to the destination plane "<< std::endl;
                            mVideoPlaneMappings[mapping.destinationPlaneIndex] = mapping;
                        }
                    }
                    *_aidl_return = true;
                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::getVideoSourceDestinationPlaneMapping(::std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping>* _aidl_return)
                {
                    std::lock_guard<std::mutex> lock(mHandleLock);
                    _aidl_return->clear();
                    // Return the current video source → video plane mapping
                    for (int planeIndex = 0; planeIndex < NUM_PLANES_VIDEO; ++planeIndex) {
                        ::com::rdk::hal::planecontrol::SourcePlaneMapping mapping;

                        auto it = mVideoPlaneMappings.find(planeIndex);
                        if (it != mVideoPlaneMappings.end()) {
                            mapping = it->second;
                        } else {
                            // No mapping for this video plane: set to NONE
                            mapping.sourceType = SourceType::NONE;
                            mapping.sourceIndex = -1;
                            mapping.destinationPlaneIndex = planeIndex;
                        }

                        _aidl_return->push_back(mapping);
                    }

                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::getProperty(int32_t planeResourceIndex, com::rdk::hal::planecontrol::Property property, ::std::optional<::com::rdk::hal::PropertyValue>* _aidl_return)
                {
                    switch (property) {
                        case Property::X:
                        case Property::Y:
                        case Property::WIDTH:
                        case Property::HEIGHT:
                        case Property::ALPHA:
                        case Property::ZORDER:
                        case Property::VISIBLE:
                        case Property::OVERSCAN:
                        case Property::ASPECT_RATIO:
                            break;
                        default:
                            return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    auto planeIt = mPlaneProperties.find(planeResourceIndex);
                    if (planeIt == mPlaneProperties.end()) {
                        // Invalid plane index; return null
                        *_aidl_return = std::nullopt;
                        return android::binder::Status::ok();
                    }

                    const auto& propertyMap = planeIt->second;
                    // Check if the requested property exists
                    auto propIt = propertyMap.find(property);
                    if (propIt == propertyMap.end()) {
                        *_aidl_return = std::nullopt;
                        return android::binder::Status::ok();
                    }

                    *_aidl_return = propIt->second;
                    return android::binder::Status::ok();
                }


                android::binder::Status PlaneControl::setProperty(int32_t planeResourceIndex, ::com::rdk::hal::planecontrol::Property property, const ::com::rdk::hal::PropertyValue& propertyValue, bool* _aidl_return)
                {
                    if (planeResourceIndex < 0 || planeResourceIndex > (NUM_PLANES_VIDEO + NUM_PLANES_GRAPHIC))
                    {
                        *_aidl_return = false;
                        std::cout << "[SERVICE] planeResourceIndex is invalid "<< std::endl;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    switch (property) {
                        case ::com::rdk::hal::planecontrol::Property::X:
                        case ::com::rdk::hal::planecontrol::Property::Y:
                        case ::com::rdk::hal::planecontrol::Property::WIDTH:
                        case ::com::rdk::hal::planecontrol::Property::HEIGHT:
                        case ::com::rdk::hal::planecontrol::Property::ALPHA:
                        case ::com::rdk::hal::planecontrol::Property::ZORDER:
                        case ::com::rdk::hal::planecontrol::Property::VISIBLE:
                        case ::com::rdk::hal::planecontrol::Property::OVERSCAN:
                        case ::com::rdk::hal::planecontrol::Property::ASPECT_RATIO:
                            break; // valid property
                        default:
                            *_aidl_return = false;
                            return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }
                    if (propertyValue.value.has_value() && propertyValue.value->getTag() != PropertyValue::Value::Tag::intValue) {
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    mPlaneProperties[planeResourceIndex][property] = propertyValue;

                    *_aidl_return = true;
                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::getPropertyMulti(int32_t planeResourceIndex, std::vector <PropertyKVPair> *propertyKVList, bool* _aidl_return)
                {
                    if (propertyKVList == nullptr ||
                        mPlaneProperties.find(planeResourceIndex) == mPlaneProperties.end()) {
                        *_aidl_return = false;
                        std::cout << "[SERVICE] planeResourceIndex is not present in the mPlaneProperties "<< std::endl;
                        return android::binder::Status::ok();
                    }

                    for (auto& kv : *propertyKVList) {
                        Property property = kv.property;
                        auto planeIt = mPlaneProperties.find(planeResourceIndex);
                        if (planeIt != mPlaneProperties.end()) {
                            const auto& propertyMap = planeIt->second;
                            auto propIt = propertyMap.find(property);
                            if (propIt != propertyMap.end()) {
                                kv.propertyValue = propIt->second;
                            } else {
                                PropertyValue invalidValue;
                                invalidValue.value = std::nullopt;
                                kv.propertyValue = invalidValue;
                            }
                        } else {
                            PropertyValue invalidValue;
                            invalidValue.value = std::nullopt;
                            kv.propertyValue = invalidValue;
                        }
                    }
                    *_aidl_return = true;
                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::setPropertyMultiAtomic( int32_t planeResourceIndex, const std::vector <com::rdk::hal::planecontrol::PropertyKVPair> &propertyKVList, bool* _aidl_return)
                {
                    // Input validation
                    if (planeResourceIndex < 0 || planeResourceIndex > (NUM_PLANES_VIDEO + NUM_PLANES_GRAPHIC)) {
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }
                    std::map<Property, PropertyValue> tempMap;
                    for (const auto& kv : propertyKVList) {
                        Property prop = kv.property;
                        const PropertyValue& val = kv.propertyValue;

                        // Validate property
                        switch (prop) {
                            case Property::X:
                            case Property::Y:
                            case Property::WIDTH:
                            case Property::HEIGHT:
                            case Property::ALPHA:
                            case Property::ZORDER:
                            case Property::VISIBLE:
                            case Property::OVERSCAN:
                            case Property::ASPECT_RATIO:
                                break; // Valid
                            default:
                                *_aidl_return = false;
                                return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                        }

                        if (val.value.has_value() && val.value->getTag() != PropertyValue::Value::Tag::intValue)
                        {
                            *_aidl_return = false;
                            return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                        }
                        tempMap[prop] = val;
                    }

                    // If all are valid, commit atomically
                    for (const auto& [prop, val] : tempMap) {
                        mPlaneProperties[planeResourceIndex][prop] = val;
                    }
                    *_aidl_return = true;
                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::registerListener(const ::android::sp<::com::rdk::hal::planecontrol::IPlaneControlListener>& listener, bool* _aidl_return)
                {
                    if (listener == nullptr)
                    {
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }
                    android::sp<IBinder> binder = IInterface::asBinder(listener);
                    for (const auto& l : mListeners)
                    {
                        if (IInterface::asBinder(l) == binder)
                        {
                            *_aidl_return = false;
                            std::cout << "[SERVICE] listener already registered "<< std::endl;
                            return android::binder::Status::ok();
                        }
                    }
                    mListeners.insert(listener);
                    *_aidl_return = true;
                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::unregisterListener(const  ::android::sp<::com::rdk::hal::planecontrol::IPlaneControlListener>& listener, bool* _aidl_return)
                {
                    if (listener == nullptr)
                    {
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }
                    android::sp<IBinder> binderToRemove = IInterface::asBinder(listener);
                    for (auto it = mListeners.begin(); it != mListeners.end(); ++it)
                    {
                        if (IInterface::asBinder(*it) == binderToRemove) {
                            mListeners.erase(it);
                            *_aidl_return = true;
                            return android::binder::Status::ok();
                        }
                    }
                    *_aidl_return = false; // Listener not found
                    return android::binder::Status::ok();
                }
            }
        }
    }
}

