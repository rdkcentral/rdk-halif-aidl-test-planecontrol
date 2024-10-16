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

// Define PixelFormat and DynamicRange mapping from string to enum
std::unordered_map<string, PixelFormat> pixelFormatMap = {
	{"YCBCR422", PixelFormat::YCBCR422},
	{"RGB", PixelFormat::RGB}
};

std::unordered_map<string, DynamicRange> dynamicRangeMap = {
	{"SDR", DynamicRange::SDR},
	{"HDR10", DynamicRange::HDR10},
	{"DOLBY_VISION", DynamicRange::DOLBY_VISION}
};


Status PlaneControl::getCapabilities(vector<Capabilities>* _aidl_return) {
	if (!_aidl_return) {
		return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
	}
	vector<Capabilities> resources;
	YAML::Node config = YAML::LoadFile("planes_config.yaml");
	// Create and populate videoPlaneResource object from YAML
	Capabilities videoPlaneResource;
	videoPlaneResource.planeIndex = 0; 
	for (const auto& format : config["videoPlane"]["pixelFormats"]) {
		string formatStr = format.as<string>();
		if (pixelFormatMap.find(formatStr) != pixelFormatMap.end()) {
			videoPlaneResource.pixelFormats.push_back(pixelFormatMap[formatStr]);
		}
	}
	videoPlaneResource.maxWidth = config["videoPlane"]["maxWidth"].as<int>();
	videoPlaneResource.maxHeight = config["videoPlane"]["maxHeight"].as<int>();
	videoPlaneResource.frameWidth = config["videoPlane"]["frameWidth"].as<int>();
	videoPlaneResource.frameHeight = config["videoPlane"]["frameHeight"].as<int>();
	videoPlaneResource.colorDepth = config["videoPlane"]["colorDepth"].as<int>();
	videoPlaneResource.maxFrameRate = config["videoPlane"]["maxFrameRate"].as<int>();
	videoPlaneResource.supportsAlpha = config["videoPlane"]["supportsAlpha"].as<bool>();
	videoPlaneResource.supportsZOrder = config["videoPlane"]["supportsZOrder"].as<bool>();
	videoPlaneResource.vsyncDisplayLatency = config["videoPlane"]["vsyncDisplayLatency"].as<int>();
	for (const auto& dynamicRange : config["videoPlane"]["supportedDynamicRanges"]) {
		string rangeStr = dynamicRange.as<string>();
		if (dynamicRangeMap.find(rangeStr) != dynamicRangeMap.end()) {
			videoPlaneResource.supportedDynamicRanges.push_back(dynamicRangeMap[rangeStr]);
		}
	}
	resources.push_back(videoPlaneResource);
	// Create and populate graphicsPlaneResource object from YAML
	Capabilities graphicsPlaneResource;
	graphicsPlaneResource.planeIndex = 1; 
	for (const auto& format : config["graphicsPlane"]["pixelFormats"]) {
		string formatStr = format.as<string>();
		if (pixelFormatMap.find(formatStr) != pixelFormatMap.end()) {
			graphicsPlaneResource.pixelFormats.push_back(pixelFormatMap[formatStr]);
		}
	}
	graphicsPlaneResource.maxWidth = config["graphicsPlane"]["maxWidth"].as<int>();
	graphicsPlaneResource.maxHeight = config["graphicsPlane"]["maxHeight"].as<int>();
	graphicsPlaneResource.frameWidth = config["graphicsPlane"]["frameWidth"].as<int>();
	graphicsPlaneResource.frameHeight = config["graphicsPlane"]["frameHeight"].as<int>();
	graphicsPlaneResource.colorDepth = config["graphicsPlane"]["colorDepth"].as<int>();
	graphicsPlaneResource.maxFrameRate = config["graphicsPlane"]["maxFrameRate"].as<int>();
	graphicsPlaneResource.supportsAlpha = config["graphicsPlane"]["supportsAlpha"].as<bool>();
	graphicsPlaneResource.supportsZOrder = config["graphicsPlane"]["supportsZOrder"].as<bool>();
	graphicsPlaneResource.vsyncDisplayLatency = config["graphicsPlane"]["vsyncDisplayLatency"].as<int>();
	for (const auto& dynamicRange : config["graphicsPlane"]["supportedDynamicRanges"]) {
		string rangeStr = dynamicRange.as<string>();
		if (dynamicRangeMap.find(rangeStr) != dynamicRangeMap.end()) {
			graphicsPlaneResource.supportedDynamicRanges.push_back(dynamicRangeMap[rangeStr]);
		}
	}
	resources.push_back(graphicsPlaneResource);
	*_aidl_return = std::move(resources); 
	return Status::ok(); 
}


Status PlaneControl::getNativeGraphicsWindowHandle(int32_t planeResourceIndex, int64_t* _aidl_return) {
	if (planeResourceIndex < 0) {
		return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
	}
	// Directly return a dummy handle for demonstration purposes
	static int64_t dummyHandle = 1234567890;  // Example handle
	*_aidl_return = dummyHandle;
	return Status::ok();
}

Status PlaneControl::releaseNativeGraphicsWindowHandle(int32_t planeResourceIndex, int64_t nativeWindowHandle) {
	if (planeResourceIndex < 0 || nativeWindowHandle <= 0) {
		return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
	}

	// In this simplified implementation, we are not tracking handles,
	// so we'll assume the release is always successful.
	return Status::ok();
}

Status PlaneControl::flipGraphicsBuffer(int32_t planeResourceIndex,bool* _aidl_return) {
	if (planeResourceIndex < 0) {
		// Return an exception if the planeResourceIndex is invalid
		return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
	}
	// Simulate the flipping of the graphics buffer
	// In a real implementation, you would interact with your graphics subsystem here.
	// For now, just print a message for demonstration purposes.
	std::cout << "Flipping graphics buffer for plane resource index: " << planeResourceIndex << std::endl;
	// Return success status
	return Status::ok();
}

Status PlaneControl::setVideoSourceDestinationPlaneMapping(const vector<SourcePlaneMapping>& listSourcePlaneMapping ,bool* _aidl_return) {
	// Clear current mappings for a fresh start
	currentMappings.clear(); 
	std::unordered_set<int> seenSourceIndexes;    // To check for duplicate source indexes
	std::unordered_set<int> seenPlaneIndexes;      // To check for duplicate plane indexes
	for (const auto& mapping : listSourcePlaneMapping) {
		int sourceIndex = mapping.sourceIndex;
		int destinationPlaneIndex = mapping.destinationPlaneIndex;
		// Check for duplicate source index
		if (seenSourceIndexes.find(sourceIndex) != seenSourceIndexes.end()) {
			return Status::fromExceptionCode(Status::EX_ILLEGAL_STATE);
		}
		seenSourceIndexes.insert(sourceIndex);
		// Check for duplicate destination plane index
		if (seenPlaneIndexes.find(destinationPlaneIndex) != seenPlaneIndexes.end()) {
			return Status::fromExceptionCode(Status::EX_ILLEGAL_STATE);
		}
		seenPlaneIndexes.insert(destinationPlaneIndex);
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


Status PlaneControl::getVideoSourceDestinationPlaneMapping(vector<SourcePlaneMapping>* _aidl_return){
	if (!_aidl_return) {
		return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
	}
	// Convert the currentMappings into a vector of SourcePlaneMapping
	vector<SourcePlaneMapping> mappings;
	for (const auto& mapping : currentMappings) {
		SourcePlaneMapping spm;
		spm.sourceIndex = mapping.first;
		spm.destinationPlaneIndex = mapping.second;
		mappings.push_back(spm);
	}
	*_aidl_return = std::move(mappings); // Return the mappings
}

Status PlaneControl::getProperty(int32_t planeResourceIndex, Property property,  ::std::optional<PropertyValue>* _aidl_return) {
	// Check if the plane exists in the map
	if (planePropertiesMap.find(planeResourceIndex) == planePropertiesMap.end()) {
		return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
	}
	// Fetch the plane's properties from the map
	const PlaneProperties& props = planePropertiesMap[planeResourceIndex];
	PropertyValue value;
	Capabilities PlaneResource;
	// Retrieve the property value based on the requested key and validate against hardcoded ranges
	switch (property) {
		case Property::RESOURCE_ID:
			value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(props.resource_id);
			break;
		case Property::X:
			if (props.X < 0 || props.X > 1920) { 
				return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT); // Invalid X
			}
			value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(props.X);
			break;
		case Property::Y:
			if (props.Y < 0 || props.Y > 1080) {
				return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT); // Invalid Y
			}
			value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(props.Y);
			break;
		case Property::WIDTH:
			if (props.width < 1 || props.width > 1920) {  
				return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT); // Invalid width
			}
			value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(props.width);
			break;
		case Property::HEIGHT:
			if (props.height < 1 || props.height > 1080) {
				return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT); // Invalid height
			}
			value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(props.height);
			break;
		case Property::ALPHA: 
			if (PlaneResource.supportsAlpha != true) {  
				return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT); // Invalid alpha
			}
			value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(props.alpha);
			break;
		case Property::ZORDER:
			if (props.zorder < 0 || props.zorder > 10) { 
				return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT); // Invalid z-order
			}
			value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(props.zorder);
			break;
		case Property::VISIBLE:
			if (props.visible == 0 || props.visible == 1) {
				return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT); // Invalid z-order
			}
			value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::booleanValue>(props.visible);
			break;
		case Property::OVERSCAN:
			if (props.overscan < 0 || props.overscan > 100) { 
				return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT); // Invalid overscan
			}
			value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(props.overscan);
			break;
		case Property::ASPECT_RATIO:
			if (props.aspect_ratio < 1 || props.aspect_ratio > 3) {  
				return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT); // Invalid aspect ratio
			}
			value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(props.aspect_ratio);
			break;

		default:
			return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT); // Invalid property
	} 
	*_aidl_return = value;
	return Status::ok();
}


Status PlaneControl::setProperty(int32_t planeResourceIndex, Property property, const PropertyValue& propertyValue, bool* _aidl_return) {
	// Check if the plane exists in the map
	if (planePropertiesMap.find(planeResourceIndex) == planePropertiesMap.end()) {
		return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
	}
	// Get the plane's properties from the map
	PlaneProperties& props = planePropertiesMap[planeResourceIndex];
	// Set the property based on the input
	switch (property) {
		case Property::RESOURCE_ID:
			props.resource_id = 1;
			break;
		case Property::X:
			props.X = 100;
			break;
		case Property::Y:
			props.Y =  200;    
			break;
		case Property::WIDTH:
			props.width = 1920;    
			break;
		case Property::HEIGHT:
			props.height = 1080;
			break;
		case planecontrol::Property::ALPHA:
			props.alpha =  255;    
			break;
		case Property::ZORDER:
		case Property::VISIBLE:
		case Property::OVERSCAN:
		case Property::ASPECT_RATIO:
			if (planeResourceIndex == 0) {  // Only Video plane
				if (property == Property::ZORDER)
					props.zorder =  1;
				if (property == Property::VISIBLE)
					props.visible =  true;
				if (property == Property::OVERSCAN)
					props.overscan =  0;
				if (property == Property::ASPECT_RATIO)
					props.aspect_ratio =  16;
			} else {
				return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);  // Invalid for graphics plane
			}
		default:
			return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT); 
	}
	*_aidl_return = true;
	return Status::ok();
}

Status PlaneControl::getPropertyMulti(int32_t planeResourceIndex, vector<PropertyKVPair>* propertyKVList, bool* _aidl_return) {
	if (planeResourceIndex < 0) {
		return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
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
				return Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
		}
	}
	if (_aidl_return) {
		*_aidl_return = true;
	}
	return Status::ok();
}

Status PlaneControl::setPropertyMultiAtomic(int32_t planeResourceIndex, const vector<PropertyKVPair>& propertyKVList, bool* _aidl_return) {
	// Validate input
	if (planeResourceIndex < 0) {
		return android::binder::Status::fromExceptionCode(
				android::binder::Status::EX_ILLEGAL_ARGUMENT);
	}
	// Process each property key-value pair
	for (const auto &pair: propertyKVList) {
		switch (pair.property) {
			case Property::X:
				// Handle Property::X
				// Example: Update X coordinate
				// plane->setXCoordinate(pair.propertyValue.value->intValue);
				break;
			case Property::Y:
				// Handle Property::Y
				// Example: Update Y coordinate
				// plane->setYCoordinate(pair.propertyValue.value->intValue);
				break;
			case Property::WIDTH:
				// Handle Property::WIDTH
				// Example: Update width
				// plane->setWidth(pair.propertyValue.value->intValue);
				break;
			case Property::HEIGHT:
				// Handle Property::HEIGHT
				// Example: Update height
				// plane->setHeight(pair.propertyValue.value->intValue);
				break;
			case Property::ALPHA:
				// Handle Property::ALPHA
				// Example: Update alpha value
				// plane->setAlpha(pair.propertyValue.value->byteValue);
				break;
			case Property::ZORDER:
				// Handle Property::ZORDER
				// Example: Update Z-order
				// plane->setZOrder(pair.propertyValue.value->intValue);
				break;
			case Property::VISIBLE:
				// Handle Property::VISIBLE
				// Example: Update visibility
				// plane->setVisible(pair.propertyValue.value->booleanValue);
				break;
			case Property::OVERSCAN:
				// Handle Property::OVERSCAN
				// Example: Update overscan value
				// plane->setOverscan(pair.propertyValue.value->intValue);
				break;
			case Property::ASPECT_RATIO:
				// Handle Property::ASPECT_RATIO
				// Example: Update aspect ratio
				// plane->setAspectRatio(pair.propertyValue.value->intValue);
				break;
			default:
				return Status::fromExceptionCode(
						Status::EX_ILLEGAL_ARGUMENT);
		}
	}
	if (_aidl_return) {
		*_aidl_return = true;
	}
	return Status::ok();
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

