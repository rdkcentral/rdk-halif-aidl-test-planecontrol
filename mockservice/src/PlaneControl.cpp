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

/** @brief
 * PlaneControl.cpp
 *
 * The implementation file contains the dummy implementations for the PlaneControl service methods.
 */

#include "PlaneControl.h"
#include <chrono>
#include <fcntl.h>
#include <time.h>
#include <unordered_set>
#include <unistd.h>

namespace com {
    namespace rdk {
        namespace hal {
            namespace planecontrol {

                class MockGraphicsFbProvider : public BnGraphicsFbProvider {
                public:
                    explicit MockGraphicsFbProvider(const ::android::sp<::com::rdk::hal::planecontrol::IGraphicsFbProviderListener>& listener)
                        : mListener(listener), mNextGraphicsFbId(1), mCurrentGraphicsFbId(-1) {}

                    ::android::binder::Status getCapabilities(::com::rdk::hal::planecontrol::GraphicsFbCapabilities* _aidl_return) override {
                        _aidl_return->maxGraphicsFrameBuffers = 3;
                        _aidl_return->maxGraphicsFbWidth = 1920;
                        _aidl_return->maxGraphicsFbHeight = 1080;
                        _aidl_return->format = static_cast<int32_t>(::com::rdk::hal::videodecoder::PixelFormat::RGB);
                        _aidl_return->modifier = 0;
                        return ::android::binder::Status::ok();
                    }

                    ::android::binder::Status commitGraphicsFb(int32_t graphicsFbId, bool* _aidl_return) override {
                        if (mAllocatedGraphicsFbIds.find(graphicsFbId) == mAllocatedGraphicsFbIds.end()) {
                            return ::android::binder::Status::fromExceptionCode(::android::binder::Status::EX_ILLEGAL_ARGUMENT);
                        }

                        const int32_t oldGraphicsFbId = mCurrentGraphicsFbId;
                        mCurrentGraphicsFbId = graphicsFbId;
                        *_aidl_return = true;

                        if (mListener != nullptr) {
                            struct timespec timestampSpec {};
                            clock_gettime(CLOCK_MONOTONIC, &timestampSpec);
                            const int64_t timestamp =
                                static_cast<int64_t>(timestampSpec.tv_sec) * 1000000000LL + timestampSpec.tv_nsec;
                            auto callbackStatus = mListener->onGraphicsFbReleased(oldGraphicsFbId, timestamp);
                            if (!callbackStatus.isOk()) {
                                return callbackStatus;
                            }
                        }

                        return ::android::binder::Status::ok();
                    }

                    ::android::binder::Status createGraphicsFb(int32_t width, int32_t height, ::com::rdk::hal::planecontrol::GraphicsFbInfo* outInfo, ::android::os::ParcelFileDescriptor* _aidl_return) override {
                        if (width <= 0 || height <= 0) {
                            return ::android::binder::Status::fromExceptionCode(::android::binder::Status::EX_ILLEGAL_ARGUMENT);
                        }
                        outInfo->graphicsFbId = mNextGraphicsFbId++;
                        mAllocatedGraphicsFbIds.insert(outInfo->graphicsFbId);
                        outInfo->pixelWidth = width;
                        outInfo->pixelHeight = height;
                        outInfo->stride = width * 4;
                        outInfo->offset = 0;
                        int fd = dup(STDIN_FILENO);
                        *_aidl_return = ::android::os::ParcelFileDescriptor(::android::base::unique_fd(fd));
                        return ::android::binder::Status::ok();
                    }

                    ::android::binder::Status destroyGraphicsFb(int32_t graphicsFbId) override {
                        if (mAllocatedGraphicsFbIds.erase(graphicsFbId) == 0) {
                            return ::android::binder::Status::fromExceptionCode(::android::binder::Status::EX_ILLEGAL_ARGUMENT);
                        }

                        if (mCurrentGraphicsFbId == graphicsFbId) {
                            mCurrentGraphicsFbId = -1;
                        }
                        return ::android::binder::Status::ok();
                    }

                private:
                    ::android::sp<::com::rdk::hal::planecontrol::IGraphicsFbProviderListener> mListener;
                    std::unordered_set<int32_t> mAllocatedGraphicsFbIds;
                    int32_t mNextGraphicsFbId;
                    int32_t mCurrentGraphicsFbId;
                };


                int32_t g_plane_indexes_video[NUM_PLANES_VIDEO];
                int32_t g_plane_indexes_graphic[NUM_PLANES_GRAPHIC];

                ::com::rdk::hal::planecontrol::PlaneCapabilities PlaneControl::buildCapabilities(
                    int planeIndex,
                    ::com::rdk::hal::planecontrol::PlaneType type,
                    std::vector<::com::rdk::hal::videodecoder::PixelFormat> pixelFormats,
                    int maxWidth,
                    int maxHeight,
                    int frameWidth,
                    int frameHeight,
                    std::vector<::com::rdk::hal::videodecoder::DynamicRange> supportedDynamicRanges,
                    int colorDepth,
                    int maxFrameRate,
                    bool supportsAlpha,
                    bool supportsZOrder,
                    int vsyncDisplayLatency
            )
                {
                    ::com::rdk::hal::planecontrol::PlaneCapabilities cap;
                    cap.planeIndex = planeIndex;
                    cap.type = type;
                    cap.pixelFormats = pixelFormats;
                    cap.maxWidth = maxWidth;
                    cap.maxHeight = maxHeight;
                    cap.frameWidth = frameWidth;
                    cap.frameHeight = frameHeight;
                    cap.supportedDynamicRanges = supportedDynamicRanges;
                    cap.colorDepth = colorDepth;
                    cap.maxFrameRate = maxFrameRate;
                    cap.supportsAlpha = supportsAlpha;
                    cap.supportsZOrder = supportsZOrder;
                    cap.vsyncDisplayLatency = vsyncDisplayLatency;
                    return cap;
                }


                std::map<::com::rdk::hal::planecontrol::Property, ::com::rdk::hal::PropertyValue> PlaneControl::buildProperties(::com::rdk::hal::planecontrol::PlaneCapabilities capabilities)
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
                    defaults[::com::rdk::hal::planecontrol::Property::VISIBLE] = makeIntValue(0);

                    if (capabilities.supportsZOrder) {
                        defaults[::com::rdk::hal::planecontrol::Property::ZORDER] = makeIntValue(capabilities.planeIndex);
                    }

                    if (capabilities.type == ::com::rdk::hal::planecontrol::PlaneType::VIDEO) {
                        // Defaults for grapics planes
                        defaults[::com::rdk::hal::planecontrol::Property::OVERSCAN] = makeIntValue(5);
                        defaults[::com::rdk::hal::planecontrol::Property::ASPECT_RATIO] = makeIntValue(0);
                    }

                    return defaults;
                }

                PlaneControl::PlaneControl() : mVideoPlaneCount(0), mGraphicsPlaneCount(0) {
                    struct PlaneData {
                        int index;
                        ::com::rdk::hal::planecontrol::PlaneType type;
                        std::vector<::com::rdk::hal::videodecoder::PixelFormat> pixelFormats;
                        int maxWidth;
                        int maxHeight;
                        int frameWidth;
                        int frameHeight;
                        std::vector<::com::rdk::hal::videodecoder::DynamicRange> dynamicRanges;
                        int colorDepth;
                        int maxFrameRate;
                        bool supportsAlpha;
                        bool supportsZOrder;
                        int vsyncDisplayLatency;
                    };

                    const std::vector<PlaneData> planes = {
                        {0, ::com::rdk::hal::planecontrol::PlaneType::VIDEO, {::com::rdk::hal::videodecoder::PixelFormat::NATIVE}, 3840, 2160, 3840, 2160, {::com::rdk::hal::videodecoder::DynamicRange::SDR, ::com::rdk::hal::videodecoder::DynamicRange::HLG, ::com::rdk::hal::videodecoder::DynamicRange::HDR10, ::com::rdk::hal::videodecoder::DynamicRange::HDR10_PLUS, ::com::rdk::hal::videodecoder::DynamicRange::DOLBY_VISION}, 12, 60, true, true, 3},
                        {1, ::com::rdk::hal::planecontrol::PlaneType::VIDEO, {::com::rdk::hal::videodecoder::PixelFormat::NATIVE}, 3840, 2160, 3840, 2160, {::com::rdk::hal::videodecoder::DynamicRange::SDR}, 10, 60, true, true, 3},
                        {2, ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS, {::com::rdk::hal::videodecoder::PixelFormat::RGB}, 1920, 1080, 1920, 1080, {::com::rdk::hal::videodecoder::DynamicRange::SDR}, 8, 60, true, false, 1}
                    };

                    int videoPlaneCount = 0;
                    int graphicsPlaneCount = 0;
                    for (const auto& plane : planes) {
                        if (plane.type == ::com::rdk::hal::planecontrol::PlaneType::VIDEO) {
                            if (videoPlaneCount < NUM_PLANES_VIDEO) {
                                g_plane_indexes_video[videoPlaneCount++] = plane.index;
                            }
                        } else if (plane.type == ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS) {
                            if (graphicsPlaneCount < NUM_PLANES_GRAPHIC) {
                                g_plane_indexes_graphic[graphicsPlaneCount++] = plane.index;
                            }
                        }
                    }

                    if (videoPlaneCount > NUM_PLANES_VIDEO) {
                        printf("[SERVICE] Warning: Profile defines %d video planes, but NUM_PLANES_VIDEO is %d. Using first %d video planes.\n",
                               videoPlaneCount, NUM_PLANES_VIDEO, NUM_PLANES_VIDEO);
                        videoPlaneCount = NUM_PLANES_VIDEO;
                    }
                    if (graphicsPlaneCount > NUM_PLANES_GRAPHIC) {
                        printf("[SERVICE] Warning: Profile defines %d graphics planes, but NUM_PLANES_GRAPHIC is %d. Using first %d graphics planes.\n",
                               graphicsPlaneCount, NUM_PLANES_GRAPHIC, NUM_PLANES_GRAPHIC);
                        graphicsPlaneCount = NUM_PLANES_GRAPHIC;
                    }
                    mVideoPlaneCount = videoPlaneCount;
                    mGraphicsPlaneCount = graphicsPlaneCount;

                    for (const auto& plane : planes) {
                        bool isVideoPlane = false;
                        bool isGraphicsPlane = false;
                        for (int i = 0; i < videoPlaneCount; i++) {
                            if (g_plane_indexes_video[i] == plane.index) {
                                isVideoPlane = true;
                                break;
                            }
                        }
                        for (int i = 0; i < graphicsPlaneCount; i++) {
                            if (g_plane_indexes_graphic[i] == plane.index) {
                                isGraphicsPlane = true;
                                break;
                            }
                        }
                        if (!isVideoPlane && !isGraphicsPlane) {
                            continue;
                        }

                        auto cap = buildCapabilities(
                            plane.index,
                            plane.type,
                            plane.pixelFormats,
                            plane.maxWidth,
                            plane.maxHeight,
                            plane.frameWidth,
                            plane.frameHeight,
                            plane.dynamicRanges,
                            plane.colorDepth,
                            plane.maxFrameRate,
                            plane.supportsAlpha,
                            plane.supportsZOrder,
                            plane.vsyncDisplayLatency
                        );
                        mPlaneProperties[plane.index] = buildProperties(cap);
                        mCapabilities.push_back(std::move(cap));
                    }

                    printf("[SERVICE] PlaneControl service initialized with %d video planes and %d graphics planes\n",
                           mVideoPlaneCount, mGraphicsPlaneCount);
                }

                PlaneControl::~PlaneControl() {
                    printf("[SERVICE] PlaneControl service destroyed\n");
                }

                android::binder::Status PlaneControl::getCapabilities(
                    ::std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities>* _aidl_return)
                {
                    std::lock_guard<std::mutex> lock(mHandleLock);
                    *_aidl_return = mCapabilities;
                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::getGraphicsFbProvider(
                    int32_t planeResourceIndex,
                    const ::android::sp<::com::rdk::hal::planecontrol::IGraphicsFbProviderListener>& graphicsFbProviderListener,
                    ::android::sp<::com::rdk::hal::planecontrol::IGraphicsFbProvider>* _aidl_return)
                {
                    std::lock_guard<std::mutex> lock(mHandleLock);

                    if (graphicsFbProviderListener == nullptr) {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                    }

                    if (planeResourceIndex < 0 || planeResourceIndex >= (mVideoPlaneCount + mGraphicsPlaneCount)) {
                        *_aidl_return = nullptr;
                        return android::binder::Status::ok();
                    }

                    bool isGraphicPlane = false;
                    for (int i = 0; i < mGraphicsPlaneCount; ++i) {
                        if (g_plane_indexes_graphic[i] == planeResourceIndex) {
                            isGraphicPlane = true;
                            break;
                        }
                    }

                    if (!isGraphicPlane) {
                        *_aidl_return = nullptr;
                        return android::binder::Status::ok();
                    }

                    *_aidl_return = new MockGraphicsFbProvider(graphicsFbProviderListener);
                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::setVideoSourceDestinationPlaneMapping(
                    const ::std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping>& listSourcePlaneMapping,
                    bool* _aidl_return)
                {
                    std::lock_guard<std::mutex> lock(mHandleLock);
                    std::set<std::pair<SourceType, int>> seenSources;
                    std::set<int> seenPlanes;
                    auto isValidVideoPlaneIndex = [this](int32_t planeIndex) {
                        for (int i = 0; i < mVideoPlaneCount; ++i) {
                            if (g_plane_indexes_video[i] == planeIndex) {
                                return true;
                            }
                        }
                        return false;
                    };

                    // Step 1: Validate inputs
                    for (const auto& mapping : listSourcePlaneMapping) {
                        if (mapping.destinationPlaneIndex != -1 && !isValidVideoPlaneIndex(mapping.destinationPlaneIndex)) {
                            *_aidl_return = false;
                            std::cout << "[SERVICE] video plane doesnt exist " << mapping.destinationPlaneIndex << std::endl;
                            return android::binder::Status::ok();
                        }
                        if (mapping.destinationPlaneIndex != -1) {
                            if (!seenPlanes.insert(mapping.destinationPlaneIndex).second) {
                                *_aidl_return = false;
                                std::cout << "[SERVICE] duplicate plane " << mapping.destinationPlaneIndex << std::endl;
                                return android::binder::Status::ok();
                            }
                        }
                        if (mapping.sourceType != SourceType::NONE) {
                            auto key = std::make_pair(mapping.sourceType, mapping.sourceIndex);
                            if (!seenSources.insert(key).second) {
                                *_aidl_return = false;
                                std::cout << "[SERVICE] duplicate sources " << std::endl;
                                return android::binder::Status::ok();
                            }
                        }
                    }

                    // Step 2: Check for already mapped planes
                    for (const auto& mapping : listSourcePlaneMapping) {
                        if (mapping.destinationPlaneIndex != -1) {
                            auto it = mVideoPlaneMappings.find(mapping.destinationPlaneIndex);
                            if (it != mVideoPlaneMappings.end()) {
                                const auto& currentMapping = it->second;
                                if (currentMapping.sourceType != SourceType::NONE && currentMapping.sourceIndex != -1) {
                                    *_aidl_return = false;
                                    std::cout << "[SERVICE] Plane already mapped to another Source " << std::endl;
                                    return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_STATE);
                                }
                            }
                        }
                    }

                    // Step 3: Apply mappings or unmap
                    *_aidl_return = true; // Default to true for valid operations
                    for (const auto& mapping : listSourcePlaneMapping) {
                        if (mapping.destinationPlaneIndex == -1) {
                            for (auto it = mVideoPlaneMappings.begin(); it != mVideoPlaneMappings.end();) {
                                if (it->second.sourceType == mapping.sourceType &&
                                    it->second.sourceIndex == mapping.sourceIndex) {
                                    std::cout << "[SERVICE] unmapping destinationPlaneIndex " << it->first << std::endl;
                                    it = mVideoPlaneMappings.erase(it);
                                } else {
                                    ++it;
                                }
                            }
                        } else {
                            std::cout << "[SERVICE] Map source to the destination plane " << mapping.destinationPlaneIndex << std::endl;
                            mVideoPlaneMappings[mapping.destinationPlaneIndex] = mapping;
                        }
                    }

                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::getVideoSourceDestinationPlaneMapping(
                    ::std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping>* _aidl_return)
                {
                    std::lock_guard<std::mutex> lock(mHandleLock);
                    _aidl_return->clear();

                    for (int i = 0; i < mVideoPlaneCount; ++i) {
                        const int32_t planeIndex = g_plane_indexes_video[i];
                        auto it = mVideoPlaneMappings.find(planeIndex);
                        ::com::rdk::hal::planecontrol::SourcePlaneMapping mapping;
                        if (it != mVideoPlaneMappings.end()) {
                            mapping = it->second;
                        } else {
                            mapping.sourceType = ::com::rdk::hal::planecontrol::SourceType::NONE;
                            mapping.sourceIndex = -1;
                            mapping.destinationPlaneIndex = planeIndex;
                        }
                        _aidl_return->push_back(mapping);
                    }

                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::getProperty(int32_t planeResourceIndex, com::rdk::hal::planecontrol::Property property, ::std::optional<::com::rdk::hal::PropertyValue>* _aidl_return) {
                    // Validate plane index
                    if (planeResourceIndex < 0 || planeResourceIndex >= (mVideoPlaneCount + mGraphicsPlaneCount)) {
                        *_aidl_return = std::nullopt;
                        std::cout << "[SERVICE] Invalid planeResourceIndex " << planeResourceIndex << std::endl;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    // Validate property
                    switch (property) {
                        case Property::RESOURCE_ID:
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

                    // Since we now dynamically populate mPlaneProperties, this should always succeed for valid indices
                    const auto& propertyMap = mPlaneProperties[planeResourceIndex];
                    auto propIt = propertyMap.find(property);
                    if (propIt == propertyMap.end()) {
                        *_aidl_return = std::nullopt;
                        return android::binder::Status::ok();
                    }

                    *_aidl_return = propIt->second;
                    return android::binder::Status::ok();
                }


                android::binder::Status PlaneControl::setProperty(int32_t planeResourceIndex, ::com::rdk::hal::planecontrol::Property property, const ::com::rdk::hal::PropertyValue& propertyValue, bool* _aidl_return) {
                    if (planeResourceIndex < 0 || planeResourceIndex >= (mVideoPlaneCount + mGraphicsPlaneCount)) {
                        *_aidl_return = false;
                        std::cout << "[SERVICE] planeResourceIndex is invalid " << planeResourceIndex << std::endl;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    switch (property) {
                        case ::com::rdk::hal::planecontrol::Property::RESOURCE_ID:
                        case ::com::rdk::hal::planecontrol::Property::X:
                        case ::com::rdk::hal::planecontrol::Property::Y:
                        case ::com::rdk::hal::planecontrol::Property::WIDTH:
                        case ::com::rdk::hal::planecontrol::Property::HEIGHT:
                        case ::com::rdk::hal::planecontrol::Property::ALPHA:
                        case ::com::rdk::hal::planecontrol::Property::ZORDER:
                        case ::com::rdk::hal::planecontrol::Property::VISIBLE:
                        case ::com::rdk::hal::planecontrol::Property::OVERSCAN:
                        case ::com::rdk::hal::planecontrol::Property::ASPECT_RATIO:
                            break;
                        default:
                            *_aidl_return = false;
                            return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }
                    if (propertyValue.value.has_value() && propertyValue.value->getTag() != PropertyValue::Value::Tag::intValue) {
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    auto planeIt = mPlaneProperties.find(planeResourceIndex);
                    if (planeIt == mPlaneProperties.end() || planeIt->second.find(property) == planeIt->second.end()) {
                        *_aidl_return = false;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    mPlaneProperties[planeResourceIndex][property] = propertyValue;
                    *_aidl_return = true;
                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::getPropertyMulti(
                    int32_t planeResourceIndex,
                    const std::vector<Property>& properties,
                    std::vector<PropertyKVPair>* propertyKVList,
                    bool* _aidl_return) {

                    if (propertyKVList == nullptr || _aidl_return == nullptr) {
                        std::cout << "[SERVICE] Null propertyKVList or _aidl_return" << std::endl;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                    }

                    if (properties.empty()) {
                        *_aidl_return = false;
                        std::cout << "[SERVICE] Empty properties list" << std::endl;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    if (planeResourceIndex < 0 || planeResourceIndex >= (mVideoPlaneCount + mGraphicsPlaneCount)) {
                        *_aidl_return = false;
                        std::cout << "[SERVICE] Invalid planeResourceIndex " << planeResourceIndex << std::endl;
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }

                    const auto& propertyMap = mPlaneProperties[planeResourceIndex];
                    bool allValid = true;

                    propertyKVList->clear();

                    for (const auto& property : properties) {
                        // Validate that the enum value is within the known range
                        if (property < Property::RESOURCE_ID || property > Property::ASPECT_RATIO) {
                            std::cout << "[SERVICE] Invalid property enum: " << static_cast<int>(property) << std::endl;
                            allValid = false;
                            propertyKVList->clear();
                            break;
                        }

                        PropertyKVPair kv;
                        kv.property = property;

                        auto propIt = propertyMap.find(property);
                        if (propIt != propertyMap.end()) {
                            kv.propertyValue = propIt->second;
                        } else {
                            PropertyValue invalidValue;
                            invalidValue.value = std::nullopt;
                            kv.propertyValue = invalidValue;
                        }

                        propertyKVList->push_back(kv);
                    }

                    *_aidl_return = allValid;
                    return android::binder::Status::ok();
                }

                android::binder::Status PlaneControl::setPropertyMultiAtomic( int32_t planeResourceIndex, const std::vector <com::rdk::hal::planecontrol::PropertyKVPair> &propertyKVList, bool* _aidl_return)
                {
                    // Input validation
                    if (planeResourceIndex < 0 || planeResourceIndex >= (mVideoPlaneCount + mGraphicsPlaneCount)) {
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

                        auto planeIt = mPlaneProperties.find(planeResourceIndex);
                        if (planeIt == mPlaneProperties.end() || planeIt->second.find(prop) == planeIt->second.end()) {
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
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
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
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
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

