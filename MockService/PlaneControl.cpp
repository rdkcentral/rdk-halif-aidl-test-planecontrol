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

using namespace com::rdk::hal;
//using com::rdk::hal::PropertyValue::Value;
using namespace com::rdk::hal::planecontrol;
using namespace com::rdk::hal::videodecoder;
using android::binder::Status;
using std::vector;


Status PlaneControl::getCapabilities(
        std::vector<Capabilities>* _aidl_return) {

    if (!_aidl_return) {
        return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
    }

    // Populate the PlaneResource array
    std::vector<Capabilities> resources;

    // Example resource for video plane
    Capabilities videoPlaneResource;
    videoPlaneResource.planeIndex = 0;
    videoPlaneResource.pixelFormats = {PixelFormat::YCBCR422, PixelFormat::RGB};
    videoPlaneResource.maxWidth = 3840;
    videoPlaneResource.maxHeight = 2160;
    videoPlaneResource.frameWidth = 3840;
    videoPlaneResource.frameHeight = 2160;
    videoPlaneResource.supportedDynamicRanges = {
            DynamicRange::SDR, DynamicRange::HDR10, DynamicRange::DOLBY_VISION
    };
    videoPlaneResource.colorDepth = 10;
    videoPlaneResource.maxFrameRate = 60;
    videoPlaneResource.supportsAlpha = true;
    videoPlaneResource.supportsZOrder = true;
    videoPlaneResource.vsyncDisplayLatency = 2; // Measured in vsync periods

    resources.push_back(videoPlaneResource);

    // Example resource for graphics plane
    Capabilities graphicsPlaneResource;
    graphicsPlaneResource.planeIndex = 1;
    graphicsPlaneResource.pixelFormats = {PixelFormat::RGB};
    graphicsPlaneResource.maxWidth = 1920;
    graphicsPlaneResource.maxHeight = 1080;
    graphicsPlaneResource.frameWidth = 1920;
    graphicsPlaneResource.frameHeight = 1080;
    graphicsPlaneResource.supportedDynamicRanges = {DynamicRange::SDR};
    graphicsPlaneResource.colorDepth = 8;
    graphicsPlaneResource.maxFrameRate = 60;
    graphicsPlaneResource.supportsAlpha = false;
    graphicsPlaneResource.supportsZOrder = false; // Primary graphics plane
    graphicsPlaneResource.vsyncDisplayLatency = 1; // Measured in vsync periods

    resources.push_back(graphicsPlaneResource);

    *_aidl_return = std::move(resources);
    return Status::ok();
}

Status PlaneControl::getNativeGraphicsWindowHandle(int32_t planeResourceIndex, int64_t* _aidl_return) {
    if (planeResourceIndex < 0) {
        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }

    // Directly return a dummy handle for demonstration purposes
    static int64_t dummyHandle = 1234567890;  // Example handle
    *_aidl_return = dummyHandle;

    return android::binder::Status::ok();
}

Status PlaneControl::releaseNativeGraphicsWindowHandle(int32_t planeResourceIndex, int64_t nativeWindowHandle) {
    if (planeResourceIndex < 0 || nativeWindowHandle <= 0) {
        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }

    // In this simplified implementation, we are not tracking handles,
    // so we'll assume the release is always successful.
    return android::binder::Status::ok();
}

Status PlaneControl::flipGraphicsBuffer(int32_t planeResourceIndex,bool* _aidl_return) {
if (planeResourceIndex < 0) {
// Return an exception if the planeResourceIndex is invalid
return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

// Simulate the flipping of the graphics buffer
// In a real implementation, you would interact with your graphics subsystem here.
// For now, just print a message for demonstration purposes.
std::cout << "Flipping graphics buffer for plane resource index: " << planeResourceIndex << std::endl;

// Return success status
return android::binder::Status::ok();
}

Status PlaneControl::setVideoSourceDestinationPlaneMapping(
        const std::vector<SourcePlaneMapping>& listSourcePlaneMapping ,bool* _aidl_return) {

    // Map to keep track of current plane mappings.
    std::unordered_map<int, int> currentMappings; // <sourceIndex, destinationPlaneIndex>

    for (const auto& mapping : listSourcePlaneMapping) {
        int sourceIndex = mapping.sourceIndex;
        int destinationPlaneIndex = mapping.destinationPlaneIndex;

        // Check if the destination plane is already mapped
        auto it = currentMappings.find(sourceIndex);
        if (it != currentMappings.end() && it->second != destinationPlaneIndex) {
            return Status::fromExceptionCode(Status::EX_ILLEGAL_STATE);
        }

        // Update the mapping
        currentMappings[sourceIndex] = destinationPlaneIndex;
    }

    // Apply the mappings in the system (simulated here)
    for (const auto& [sourceIndex, destinationPlaneIndex] : currentMappings) {
        // Apply the mapping to the system
        // In a real implementation, you would interact with system services or hardware here
    }

    return Status::ok();
}


Status PlaneControl::getVideoSourceDestinationPlaneMapping(::std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping>* _aidl_return){
// Dummy implementation: return OK
    return Status::ok();
}

Status PlaneControl::getProperty(int32_t planeResourceIndex, com::rdk::hal::planecontrol::Property property,  ::std::optional<::com::rdk::hal::PropertyValue>* _aidl_return) {
    // Validate planeResourceIndex
    if (planeResourceIndex < 0) {
        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }

    // Initialize the PropertyValue
    com::rdk::hal::PropertyValue value;

    switch (property) {
        case com::rdk::hal::planecontrol::Property::X:
            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(100);  // Example value
            break;
        case com::rdk::hal::planecontrol::Property::Y:
            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(200);  // Example value
            break;
        case com::rdk::hal::planecontrol::Property::WIDTH:
            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(1920);  // Example value
            break;
        case com::rdk::hal::planecontrol::Property::HEIGHT:
            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(1080);  // Example value
            break;
        case com::rdk::hal::planecontrol::Property::ALPHA:
            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::byteValue>(static_cast<int8_t>(255));  // Example value
            break;
        case com::rdk::hal::planecontrol::Property::ZORDER:
            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(1);  // Example value
            break;
        case com::rdk::hal::planecontrol::Property::VISIBLE:
            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::booleanValue>(true);  // Example value
            break;
        case com::rdk::hal::planecontrol::Property::OVERSCAN:
            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(0);  // Example value
            break;
        case com::rdk::hal::planecontrol::Property::ASPECT_RATIO:
            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(16);  // Example value (e.g., 16:9)
            break;
        default:
            return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }

    *_aidl_return = value;
    return android::binder::Status::ok();
}


Status PlaneControl::setProperty(int32_t planeResourceIndex, Property property, const ::com::rdk::hal::PropertyValue& propertyValue, bool* _aidl_return) {
    // Dummy implementation: return OK
    return Status::ok();
}

Status PlaneControl::getPropertyMulti(int32_t planeResourceIndex, ::std::vector<::com::rdk::hal::planecontrol::PropertyKVPair>* propertyKVList, bool* _aidl_return) {
    if (planeResourceIndex < 0) {
        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }

    for (auto& pair : *propertyKVList) {
        switch (pair.property) {
            case Property::X:
                pair.propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(100);  // Example value
                break;
            case Property::Y:
                pair.propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(200);  // Example value
                break;
            case Property::WIDTH:
                pair.propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(1920);  // Example value
                break;
            case Property::HEIGHT:
                pair.propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(1080);  // Example value
                break;
            case Property::ALPHA:
                pair.propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::byteValue>(static_cast<int8_t>(255));  // Example value
                break;
            case Property::ZORDER:
                pair.propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(1);  // Example value
                break;
            case Property::VISIBLE:
                pair.propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::booleanValue>(true);  // Example value
                break;
            case Property::OVERSCAN:
                pair.propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(0);  // Example value
                break;
            case Property::ASPECT_RATIO:
                pair.propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(16);  // Example value (e.g., 16:9)
                break;
            default:
                return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
        }
    }
    return android::binder::Status::ok();
}

Status PlaneControl::setPropertyMultiAtomic(
        int32_t planeResourceIndex, const ::std::vector<::com::rdk::hal::planecontrol::PropertyKVPair>& propertyKVList, bool* _aidl_return) {

    // Validate input
    if (planeResourceIndex < 0) {
        return android::binder::Status::fromExceptionCode(
                android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }

    // Process each property key-value pair
    for (const auto &pair: propertyKVList) {
        switch (pair.property) {
            case com::rdk::hal::planecontrol::Property::X:
                // Handle Property::X
                // Example: Update X coordinate
                // plane->setXCoordinate(pair.propertyValue.value->intValue);
                break;

            case com::rdk::hal::planecontrol::Property::Y:
                // Handle Property::Y
                // Example: Update Y coordinate
                // plane->setYCoordinate(pair.propertyValue.value->intValue);
                break;

            case com::rdk::hal::planecontrol::Property::WIDTH:
                // Handle Property::WIDTH
                // Example: Update width
                // plane->setWidth(pair.propertyValue.value->intValue);
                break;

            case com::rdk::hal::planecontrol::Property::HEIGHT:
                // Handle Property::HEIGHT
                // Example: Update height
                // plane->setHeight(pair.propertyValue.value->intValue);
                break;

            case com::rdk::hal::planecontrol::Property::ALPHA:
                // Handle Property::ALPHA
                // Example: Update alpha value
                // plane->setAlpha(pair.propertyValue.value->byteValue);
                break;

            case com::rdk::hal::planecontrol::Property::ZORDER:
                // Handle Property::ZORDER
                // Example: Update Z-order
                // plane->setZOrder(pair.propertyValue.value->intValue);
                break;

            case com::rdk::hal::planecontrol::Property::VISIBLE:
                // Handle Property::VISIBLE
                // Example: Update visibility
                // plane->setVisible(pair.propertyValue.value->booleanValue);
                break;

            case com::rdk::hal::planecontrol::Property::OVERSCAN:
                // Handle Property::OVERSCAN
                // Example: Update overscan value
                // plane->setOverscan(pair.propertyValue.value->intValue);
                break;

            case com::rdk::hal::planecontrol::Property::ASPECT_RATIO:
                // Handle Property::ASPECT_RATIO
                // Example: Update aspect ratio
                // plane->setAspectRatio(pair.propertyValue.value->intValue);
                break;

            default:
                return android::binder::Status::fromExceptionCode(
                        android::binder::Status::EX_ILLEGAL_ARGUMENT);
        }
    }

    return android::binder::Status::ok();
}

Status PlaneControl::registerListener(const sp<IPlaneControlListener>& listener, bool* _aidl_return) {
    // Dummy implementation: return OK
    if (_aidl_return) {
        *_aidl_return = true;
    }
    return Status::ok();
}

Status PlaneControl::unregisterListener(const sp<IPlaneControlListener>& listener, bool* _aidl_return) {
    // Dummy implementation: return OK
    if (_aidl_return) {
        *_aidl_return = true;
    }
    return Status::ok();
}

