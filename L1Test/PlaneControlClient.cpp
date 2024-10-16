
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

#include <gtest/gtest.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/Status.h>
#include <utils/String16.h>
#include <stdio.h>
#include <vector>
#include <optional>
#include <climits>

#include "com/rdk/hal/planecontrol/IPlaneControl.h"

using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;

using com::rdk::hal::planecontrol::IPlaneControl;
using ::com::rdk::hal::PropertyValue;
using ::com::rdk::hal::planecontrol::Property;


sp<IPlaneControl> service;
sp<IPlaneControl> getService() {

    service = android::interface_cast<IPlaneControl>(

            defaultServiceManager()->getService(String16("PlaneControl")));

    if (service == nullptr) {
        printf("Service not available\n");
    }

    return service;
}

//Pre-requisite
class PlaneControlServiceTest : public ::testing::Test {
protected:
    static std::vector<int32_t> planeIndices;  // Static vector to store all planeIndex values

    // This function runs once before any test in the suite
    static void SetUpTestSuite() {
        // Call the getResourceCapabilities API
        std::vector<::com::rdk::hal::planecontrol::Capabilities> planeResources;
        ::android::binder::Status status = service->getCapabilities(&planeResources);

        // Check if status is OK
        ASSERT_TRUE(status.isOk());

        // Check the output vector is not empty
        ASSERT_FALSE(planeResources.empty());

        // Clear and populate the planeIndices vector
        planeIndices.clear();
        std::set<int32_t> uniquePlaneIndices;
        for (const auto& resource : planeResources) {
            // Check planeIndex is within valid range and unique
            ASSERT_TRUE(resource.planeIndex >= 0);
            ASSERT_TRUE(uniquePlaneIndices.insert(resource.planeIndex).second);  // Ensure uniqueness

            // Store the valid planeIndex
            planeIndices.push_back(resource.planeIndex);
        }

        // Ensure that we have stored valid planeIndices
        ASSERT_FALSE(planeIndices.empty());
    }

    // This function runs once after all tests in the suite are done
    static void TearDownTestSuite() {
        // If there is any cleanup required after all tests have run, add it here
    }
};


// Helper class to create a valid PlaneControlListener
class ValidPlaneControlListener : public ::com::rdk::hal::planecontrol::IPlaneControlListener {
public:

    ::android::IBinder* onAsBinder() override {
        // Implement if necessary
        return nullptr;
    }
    // Implementation details for the listener can be defined here.
};


//helper function
bool populateExpectedPropertyValue(const com::rdk::hal::planecontrol::PropertyKVPair& kvPair, com::rdk::hal::planecontrol::Property expectedProperty) {
    if (kvPair.property != expectedProperty)
        {
            return false;
        }
    else if (!kvPair.propertyValue.value.has_value())
        {
            return false;
        }
    else
        {
            return true;
        }
}

std::vector<int32_t> PlaneControlServiceTest::planeIndices;

TEST_F(PlaneControlServiceTest, FullCapabilityCheckPositive) {

    // Call the getResourceCapabilities API
    std::vector<::com::rdk::hal::planecontrol::Capabilities> planeResources;
    ::android::binder::Status status = service->getCapabilities(&planeResources);

    // Check if status is OK
    ASSERT_TRUE(status.isOk());

    // Check the output vector is not empty
    ASSERT_FALSE(planeResources.empty());

    // Check the PlaneResource objects
    std::set<int32_t> planeIndices;
    for (const auto& resource : planeResources) {
    // Check planeIndex is within valid range and unique
    ASSERT_TRUE(resource.planeIndex >= 0);

    // Check pixelFormats contains valid PixelFormat entries
    for (auto format : resource.pixelFormats) {
    ASSERT_TRUE(format == ::com::rdk::hal::videodecoder::PixelFormat::YCBCR422 ||
    format == ::com::rdk::hal::videodecoder::PixelFormat::YCBCR444 ||
    format == ::com::rdk::hal::videodecoder::PixelFormat::YCBCR420 ||
    format == ::com::rdk::hal::videodecoder::PixelFormat::RGB ||
    format == ::com::rdk::hal::videodecoder::PixelFormat::NATIVE);
    }

    // Check maxWidth and maxHeight are within expected sizes and non-zero
    ASSERT_GT(resource.maxWidth, 0);
    ASSERT_GT(resource.maxHeight, 0);


    // Check supportedDynamicRanges contains valid DynamicRange entries
    for (auto range : resource.supportedDynamicRanges) {
    ASSERT_TRUE(range == ::com::rdk::hal::videodecoder::DynamicRange::UNKNOWN ||
    range == ::com::rdk::hal::videodecoder::DynamicRange::SDR ||
    range == ::com::rdk::hal::videodecoder::DynamicRange::HLG ||
    range == ::com::rdk::hal::videodecoder::DynamicRange::HDR10 ||
    range == ::com::rdk::hal::videodecoder::DynamicRange::HDR10PLUS ||
    range == ::com::rdk::hal::videodecoder::DynamicRange::DOLBY_VISION);
    }

    ASSERT_GT(resource.maxFrameRate, 0);

    // Check supportsAlpha is correctly set
    ASSERT_TRUE(resource.supportsAlpha == true || resource.supportsAlpha == false);

    // Check supportsZOrder is correctly set and false for primary graphics plane
    ASSERT_TRUE(resource.supportsZOrder == true || resource.supportsZOrder == false);
    }
}

TEST_F(PlaneControlServiceTest, ValidPlaneResourceIndexPositive) {
    int64_t _aidl_return;
    for (const auto& planeResourceIndex : planeIndices) {
        Status status = service->getNativeGraphicsWindowHandle(planeResourceIndex, &_aidl_return);
        EXPECT_TRUE(status.isOk());
    }
}

TEST_F(PlaneControlServiceTest, InvalidNegativePlaneResourceIndex) {
    int32_t planeResourceIndex = -1;
    int64_t _aidl_return;
    Status status = service->getNativeGraphicsWindowHandle(planeResourceIndex, &_aidl_return);
    EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

TEST_F(PlaneControlServiceTest, SuccessiveCallsConsistency) {
    int64_t _aidl_return1, _aidl_return2;
    for (const auto& planeResourceIndex : planeIndices) {
        Status status1 = service->getNativeGraphicsWindowHandle(planeResourceIndex, &_aidl_return1);
        EXPECT_TRUE(status1.isOk());

        Status status2 = service->getNativeGraphicsWindowHandle(planeResourceIndex, &_aidl_return2);
        EXPECT_TRUE(status2.isOk());
        EXPECT_EQ(_aidl_return1, _aidl_return2);
    }
}


TEST_F(PlaneControlServiceTest, InvalidNativeWindowHandleNegativeValue) {
    int64_t nativeWindowHandle = 10; //random number for negative test
    for (const auto& planeResourceIndex : planeIndices) {
        Status status = service->releaseNativeGraphicsWindowHandle(planeResourceIndex, nativeWindowHandle);
        EXPECT_EQ(status.transactionError(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }
}

TEST_F(PlaneControlServiceTest, PositiveCase_ValidPlaneResourceIndex) {
    bool result;
    for (const auto& planeResourceIndex : planeIndices) {
        Status status = service->flipGraphicsBuffer(planeResourceIndex,&result);
        EXPECT_TRUE(status.isOk());
	EXPECT_TRUE(result);
    }
}

TEST_F(PlaneControlServiceTest, NegativeCase_InvalidPlaneResourceIndex_NegativeInteger) {
    int32_t planeResourceIndex = -1;
    bool result;
    Status status = service->flipGraphicsBuffer(planeResourceIndex,&result);
    EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    EXPECT_TRUE(result);
}

TEST_F(PlaneControlServiceTest, ValidSingleSourceToPlaneMappingVIDEO_SINK_main) {
    bool result;
    // Setup input
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(1);
    listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
    listSourcePlaneMapping[0].sourceIndex = 0; // sourceIndex = 0 means main
    listSourcePlaneMapping[0].destinationPlaneIndex = 0;

    // Call API
    Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping,&result);

    // Verify output
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(result);
}

TEST_F(PlaneControlServiceTest, ValidSingleSourceToPlaneMappingVIDEO_SINK_pip) {
    bool result;
    // Setup input
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(1);
    listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
    listSourcePlaneMapping[0].sourceIndex = 1; // sourceIndex = 1 means pip
    listSourcePlaneMapping[0].destinationPlaneIndex = 1;

    // Call API
    Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping,&result);

    // Verify output
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(result);
}

TEST_F(PlaneControlServiceTest, ValidSingleSourceToPlaneMappingVIDEO_SINK_swap) {
    bool result;
    // Setup input
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(2);
    listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
    listSourcePlaneMapping[0].sourceIndex = 0; // sourceIndex = 0 means main
    listSourcePlaneMapping[0].destinationPlaneIndex = 1;

    listSourcePlaneMapping[1].sourceType = ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
    listSourcePlaneMapping[1].sourceIndex = 1; // sourceIndex = 1 means pip
    listSourcePlaneMapping[1].destinationPlaneIndex = 0;    

    // Call API
    Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping,&result);

    // Verify output
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(result);
}

TEST_F(PlaneControlServiceTest, ValidSingleSourceToPlaneMappingHDMI_main) {
    bool result;
    // Setup input
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(1);
    listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::HDMI;
    listSourcePlaneMapping[0].sourceIndex = 0; // sourceIndex = 0 means main
    listSourcePlaneMapping[0].destinationPlaneIndex = 0;

    // Call API
    Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping,&result);

    // Verify output
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(result);
}

TEST_F(PlaneControlServiceTest, ValidSingleSourceToPlaneMappingHDMI_pip) {
    bool result;
     	// Setup input
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(1);
    listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::HDMI;
    listSourcePlaneMapping[0].sourceIndex = 1; // sourceIndex = 1 means pip
    listSourcePlaneMapping[0].destinationPlaneIndex = 1;

    // Call API
    Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping,&result);

    // Verify output
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(result);
}

TEST_F(PlaneControlServiceTest, ValidSingleSourceToPlaneMappingHDMI_swap) {
    bool result;
     	// Setup input
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(2);
    listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::HDMI;
    listSourcePlaneMapping[0].sourceIndex = 0; // sourceIndex = 0 means main
    listSourcePlaneMapping[0].destinationPlaneIndex = 1;

    listSourcePlaneMapping[1].sourceType = ::com::rdk::hal::planecontrol::SourceType::HDMI;
    listSourcePlaneMapping[1].sourceIndex = 1; // sourceIndex = 1 means pip
    listSourcePlaneMapping[1].destinationPlaneIndex = 0;    

    // Call API
    Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping,&result);

    // Verify output
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(result);
}

TEST_F(PlaneControlServiceTest, ValidSingleSourceToPlaneMappingCOMPOSITE_main) {
    bool result;
   	// Setup input
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(1);
    listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::COMPOSITE;
    listSourcePlaneMapping[0].sourceIndex = 0; // sourceIndex = 0 means main
    listSourcePlaneMapping[0].destinationPlaneIndex = 0;

    // Call API
    Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping,&result);

    // Verify output
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(result);
}

TEST_F(PlaneControlServiceTest, ValidSingleSourceToPlaneMappingCOMPOSITE_pip) {
    bool result;
    // Setup input
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(1);
    listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::COMPOSITE;
    listSourcePlaneMapping[0].sourceIndex = 1; // sourceIndex = 1 means pip
    listSourcePlaneMapping[0].destinationPlaneIndex = 1;

    // Call API
    Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping,&result);

    // Verify output
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(result);
}

TEST_F(PlaneControlServiceTest, ValidSingleSourceToPlaneMappingCOMPOSITE_swap) {
    bool result;
    // Setup input
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(2);
    listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::COMPOSITE;
    listSourcePlaneMapping[0].sourceIndex = 0; // sourceIndex = 0 means main
    listSourcePlaneMapping[0].destinationPlaneIndex = 1;

    listSourcePlaneMapping[1].sourceType = ::com::rdk::hal::planecontrol::SourceType::COMPOSITE;
    listSourcePlaneMapping[1].sourceIndex = 1; // sourceIndex = 1 means pip
    listSourcePlaneMapping[1].destinationPlaneIndex = 0;    

    // Call API
    Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping,&result);

    // Verify output
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(result);
}

TEST_F(PlaneControlServiceTest, ValidMultipleSourceToPlaneMappings) {
    bool result;
    // Setup input
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(2);
    listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::HDMI;
    listSourcePlaneMapping[0].sourceIndex = 0;
    listSourcePlaneMapping[0].destinationPlaneIndex = 0;

    listSourcePlaneMapping[1].sourceType = ::com::rdk::hal::planecontrol::SourceType::COMPOSITE;
    listSourcePlaneMapping[1].sourceIndex = 0;
    listSourcePlaneMapping[1].destinationPlaneIndex = 1;

    // Call API
    Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping,&result);

    // Verify output
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(result);
}

TEST_F(PlaneControlServiceTest, NoSourceTypeSet) {
    bool result;
    	// Setup input
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(1);
    listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::NONE;
    listSourcePlaneMapping[0].sourceIndex = 0;
    listSourcePlaneMapping[0].destinationPlaneIndex = 0;

    // Call API
    Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping,&result);

    // Verify output
    EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    EXPECT_TRUE(result);
}

TEST_F(PlaneControlServiceTest, InvalidSourceIndex) {
    bool result;
    // Setup input
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(1);
    listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::HDMI;
    listSourcePlaneMapping[0].sourceIndex = -1;
    listSourcePlaneMapping[0].destinationPlaneIndex = 0;

    // Call API
    Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping,&result);

    // Verify output
    EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    EXPECT_TRUE(result);
}

TEST_F(PlaneControlServiceTest, DestinationPlaneIndexIsNegative) {
    bool result;
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(1);
    listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
    listSourcePlaneMapping[0].sourceIndex = 1;
    listSourcePlaneMapping[0].destinationPlaneIndex = -1;

    Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping,&result);
    EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    EXPECT_TRUE(result);
}

TEST_F(PlaneControlServiceTest, DuplicatePlaneMapping) {
    bool result;
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(2);
    listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
    listSourcePlaneMapping[0].sourceIndex = 1;
    listSourcePlaneMapping[0].destinationPlaneIndex = 0;
    
    listSourcePlaneMapping[1].sourceType = ::com::rdk::hal::planecontrol::SourceType::HDMI;
    listSourcePlaneMapping[1].sourceIndex = 1;
    listSourcePlaneMapping[1].destinationPlaneIndex = 0;

    Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping,&result);
    EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);
    EXPECT_TRUE(result);
}

TEST_F(PlaneControlServiceTest, TestGetPropertyWithValidPlaneResourceIndexAndPropertyX) {
    Property property = Property::X;
    std::optional<com::rdk::hal::PropertyValue> result;

    for (const auto& planeResourceIndex : planeIndices) {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.has_value());
    }
}

TEST_F(PlaneControlServiceTest, TestGetPropertyWithValidPlaneResourceIndexAndPropertyY) {
    Property property = Property::Y;
    std::optional<com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : planeIndices) {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.has_value());
    }
}

TEST_F(PlaneControlServiceTest, TestGetPropertyWithInvalidPlaneResourceIndex) {
    int32_t planeResourceIndex = -1;
    Property property = Property::X;
    std::optional<com::rdk::hal::PropertyValue> result;
    Status status = service->getProperty(planeResourceIndex, property, &result);
    ASSERT_FALSE(status.isOk());
    EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

TEST_F(PlaneControlServiceTest, TestGetPropertyWithValidPlaneResourceIndexAndPropertyWidth) {
    Property property = Property::WIDTH;
    std::optional<com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : planeIndices) {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.has_value());
    }
}

TEST_F(PlaneControlServiceTest, TestGetPropertyWithInvalidProperty) {
    Property property = static_cast<Property>(-1);
    std::optional<com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : planeIndices) {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_FALSE(status.isOk());
        EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }
}

TEST_F(PlaneControlServiceTest, TestGetPropertyWithValidPlaneResourceIndexAndPropertyHeight) {
    Property property = Property::HEIGHT;
    std::optional<com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : planeIndices) {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.has_value());
    }
}

TEST_F(PlaneControlServiceTest, TestGetPropertyWithValidPlaneResourceIndexAndPropertyAlpha) {
    Property property = Property::ALPHA;
    std::optional<com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : planeIndices) {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.has_value());

    }
//EXPECT_IN_RANGE(std::get<int32_t>(result.value->value),0,255);
}

TEST_F(PlaneControlServiceTest, TestGetPropertyWithValidPlaneResourceIndexAndPropertyZOrder) {
    Property property = Property::ZORDER;
    std::optional<com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : planeIndices) {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.has_value());
    }
}

TEST_F(PlaneControlServiceTest, TestGetPropertyWithValidPlaneResourceIndexAndPropertyVisible) {
    Property property = Property::VISIBLE;
    std::optional<com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : planeIndices) {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.has_value());
    }
}


TEST_F(PlaneControlServiceTest, TestGetPropertyWithValidPlaneResourceIndexAndPropertyOverScan) {
    Property property = Property::OVERSCAN;
    std::optional<com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : planeIndices) {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.has_value());
    }
}


TEST_F(PlaneControlServiceTest, TestGetPropertyWithValidPlaneResourceIndexAndPropertyAspectRatio) {
    Property property = Property::ASPECT_RATIO;
    std::optional<com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : planeIndices) {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.has_value());
    }
}

TEST_F(PlaneControlServiceTest, SetPropertyX) {
PropertyValue value;
bool result;
for (const auto& planeResourceIndex : planeIndices) {
    value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(100);
    Status status = service->setProperty(planeResourceIndex, Property::X, value,&result);
    ASSERT_TRUE(result);
    ASSERT_TRUE(status.isOk());
   
}
}

TEST_F(PlaneControlServiceTest, SetPropertyY) {
PropertyValue value;
bool result;
for (const auto& planeResourceIndex : planeIndices) {
    value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(200);
    Status status = service->setProperty(planeResourceIndex, Property::Y, value,&result);
   ASSERT_TRUE(result);
    ASSERT_TRUE(status.isOk());
}
}

TEST_F(PlaneControlServiceTest, SetPropertyWidth) {
PropertyValue value;
bool result;
for (const auto& planeResourceIndex : planeIndices) {
    value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(1280);
    Status status = service->setProperty(planeResourceIndex, Property::WIDTH, value,&result);
    ASSERT_TRUE(status.isOk());
   ASSERT_TRUE(result);
}
}

TEST_F(PlaneControlServiceTest, SetPropertyHeight) {
PropertyValue value;
bool result;
for (const auto& planeResourceIndex : planeIndices) {
    value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(720);
    Status status = service->setProperty(planeResourceIndex, Property::HEIGHT, value,&result);
    ASSERT_TRUE(result);
    ASSERT_TRUE(status.isOk());
}
}

TEST_F(PlaneControlServiceTest, SetPropertyAlpha) {
PropertyValue value;
bool result;
for (const auto& planeResourceIndex : planeIndices) {
    value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(128);
    Status status = service->setProperty(planeResourceIndex, Property::ALPHA, value,&result);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(result);
}
}

TEST_F(PlaneControlServiceTest, SetPropertyInvalidPlaneResourceIndex) {
int planeResourceIndex = -1;
PropertyValue value;
bool result;
value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(100);
Status status = service->setProperty(planeResourceIndex, Property::X, value,&result);
ASSERT_FALSE(status.isOk());
ASSERT_TRUE(result);
ASSERT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

TEST_F(PlaneControlServiceTest, SetPropertyInvalidProperty) {
PropertyValue value;
bool result;
value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(100);
for (const auto& planeResourceIndex : planeIndices) {
    Status status = service->setProperty(planeResourceIndex, static_cast<Property>(-1), value,&result);
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    ASSERT_TRUE(result);
}
}

TEST_F(PlaneControlServiceTest, SetPropertyInvalidPropertyValueType) {
PropertyValue value;
bool result;
//value.value = PropertyValue::Value{PropertyValue::Value::Tag::booleanValue, true};
value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::booleanValue>(true);
for (const auto& planeResourceIndex : planeIndices) {
    Status status = service->setProperty(planeResourceIndex, Property::WIDTH, value,&result);
    ASSERT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
   ASSERT_TRUE(result);
}
}

TEST_F(PlaneControlServiceTest, SetPropertyOverscan) {
PropertyValue value;
bool result;
for (const auto& planeResourceIndex : planeIndices) {
    value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(5);
    Status status = service->setProperty(planeResourceIndex, Property::OVERSCAN, value,&result);
    ASSERT_TRUE(status.isOk());
   ASSERT_TRUE(result);
}
}

TEST_F(PlaneControlServiceTest, GetPropertyMulti_ValidInput) {
    bool result;
    com::rdk::hal::planecontrol::PropertyKVPair pairX;
    pairX.property = com::rdk::hal::planecontrol::Property::X;
    pairX.propertyValue = com::rdk::hal::PropertyValue{};

    com::rdk::hal::planecontrol::PropertyKVPair pairAlpha;
    pairAlpha.property = com::rdk::hal::planecontrol::Property::ALPHA;
    pairAlpha.propertyValue = com::rdk::hal::PropertyValue{};

    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList = {pairX, pairAlpha};
    for (const auto& planeResourceIndex : planeIndices) {
        ::android::binder::Status status = service->getPropertyMulti(planeResourceIndex, &propertyKVList,&result);
        // Validate
        EXPECT_TRUE(status.isOk());
        EXPECT_EQ(propertyKVList.size(), 2u);

        EXPECT_TRUE(populateExpectedPropertyValue(propertyKVList[0], com::rdk::hal::planecontrol::Property::X));
        EXPECT_TRUE(populateExpectedPropertyValue(propertyKVList[1], com::rdk::hal::planecontrol::Property::ALPHA)); // Example value
        ASSERT_TRUE(result);
    }

}

TEST_F(PlaneControlServiceTest, GetPropertyMulti_InvalidPlaneResourceIndex) {
    int32_t planeResourceIndex = -1;
    bool result;

    com::rdk::hal::planecontrol::PropertyKVPair pairX;
    pairX.property = com::rdk::hal::planecontrol::Property::X;
    pairX.propertyValue = com::rdk::hal::PropertyValue{};

    com::rdk::hal::planecontrol::PropertyKVPair pairAlpha;
    pairAlpha.property = com::rdk::hal::planecontrol::Property::ALPHA;
    pairAlpha.propertyValue = com::rdk::hal::PropertyValue{};

    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList = {pairX, pairAlpha};
    ::android::binder::Status status = service->getPropertyMulti(planeResourceIndex, &propertyKVList,&result);
    EXPECT_EQ(status.exceptionCode(), ::android::binder::Status::EX_ILLEGAL_ARGUMENT);
    EXPECT_TRUE(propertyKVList.empty());
    EXPECT_TRUE(result);
}



TEST_F(PlaneControlServiceTest, PositiveCase_SetMultiplePlanePropertiesSuccessfully) {
    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
    bool result;
    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair1;
    propertyKVPair1.property = com::rdk::hal::planecontrol::Property::X;
    //propertyKVPair1.propertyValue.value.emplace<com::rdk::hal::PropertyValue::Value::Tag::intValue>(100);
    propertyKVPair1.propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(100);
    propertyKVList.push_back(propertyKVPair1);

    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair2;
    propertyKVPair2.property = com::rdk::hal::planecontrol::Property::Y;
    //propertyKVPair2.propertyValue.value.emplace<com::rdk::hal::PropertyValue::Value::Tag::intValue>(200);
    propertyKVPair1.propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(200);
    propertyKVList.push_back(propertyKVPair2);

    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair3;
    propertyKVPair3.property = com::rdk::hal::planecontrol::Property::WIDTH;
    //propertyKVPair3.propertyValue.value.emplace<com::rdk::hal::PropertyValue::Value::Tag::intValue>(800);
    propertyKVPair1.propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(800);
    propertyKVList.push_back(propertyKVPair3);

    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair4;
    propertyKVPair4.property = com::rdk::hal::planecontrol::Property::HEIGHT;
    //propertyKVPair4.propertyValue.value.emplace<com::rdk::hal::PropertyValue::Value::Tag::intValue>(600);
    propertyKVPair1.propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(600);
    propertyKVList.push_back(propertyKVPair4);

    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair5;
    propertyKVPair5.property = com::rdk::hal::planecontrol::Property::ALPHA;
    //propertyKVPair5.propertyValue.value.emplace<com::rdk::hal::PropertyValue::Value::Tag::byteValue>(static_cast<int8_t>(255));
    propertyKVPair1.propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::byteValue>(static_cast<int8_t>(255));
    propertyKVList.push_back(propertyKVPair5);

    for (const auto& planeResourceIndex : planeIndices) {
        android::binder::Status status = service->setPropertyMultiAtomic(planeResourceIndex, propertyKVList,&result);
        EXPECT_TRUE(status.isOk());
        EXPECT_TRUE(result);
    }
}

TEST_F(PlaneControlServiceTest, NegativeCase_NoPropertiesDefined) {
    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
    bool result;
    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair;
    propertyKVPair.property = com::rdk::hal::planecontrol::Property::WIDTH;
    propertyKVList.push_back(propertyKVPair); // No property value defined (nullopt)

    for (const auto& planeResourceIndex : planeIndices) {
        android::binder::Status status = service->setPropertyMultiAtomic(planeResourceIndex, propertyKVList,&result);
        EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
        EXPECT_TRUE(result);
    }
}

TEST_F(PlaneControlServiceTest, PositiveCase_SetSinglePropertySuccessfully) {
    bool result;
    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair;
    propertyKVPair.property = com::rdk::hal::planecontrol::Property::ZORDER;
    propertyKVPair.propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(1);
    propertyKVList.push_back(propertyKVPair);
    for (const auto& planeResourceIndex : planeIndices) {
        android::binder::Status status = service->setPropertyMultiAtomic(planeResourceIndex, propertyKVList,&result);
        EXPECT_TRUE(status.isOk());
        EXPECT_TRUE(result);
    }
}

TEST_F(PlaneControlServiceTest, NegativeCase_InvalidPlaneResourceIndex) {
    int32_t planeResourceIndex = -1;
     bool result;
    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair;
    propertyKVPair.property = com::rdk::hal::planecontrol::Property::WIDTH;
    propertyKVPair.propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(800);
    propertyKVList.push_back(propertyKVPair);
    for (const auto& planeResourceIndex : planeIndices) {
        android::binder::Status status = service->setPropertyMultiAtomic(planeResourceIndex, propertyKVList,&result);
        EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
        EXPECT_TRUE(result);
    }
}

TEST_F(PlaneControlServiceTest, NegativeCase_InvalidPropertyKey) {
    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
    bool result;
    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair;
    propertyKVPair.property = static_cast<com::rdk::hal::planecontrol::Property>(9999); // Invalid property key
    propertyKVPair.propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(800);
    propertyKVList.push_back(propertyKVPair);
    for (const auto& planeResourceIndex : planeIndices) {
        android::binder::Status status = service->setPropertyMultiAtomic(planeResourceIndex, propertyKVList,&result);
        EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
        EXPECT_TRUE(result);
    }
}


TEST_F(PlaneControlServiceTest, RegisterValidListener) {
    ::android::sp<ValidPlaneControlListener> listener = new ValidPlaneControlListener();
    bool _aidl_return;
    
    // Simulate interacting with the Android service
    Status status = service->registerListener(listener, &_aidl_return);

    // Verify the listener is registered successfully
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(_aidl_return);
    
    //cleanup
    status = service->unregisterListener(listener,&_aidl_return);
}

TEST_F(PlaneControlServiceTest, RegisterSameListenerTwice) {
    ::android::sp<ValidPlaneControlListener> listener = new ValidPlaneControlListener();
    bool _aidl_return;
    
    // Register the listener for the first time
    Status status = service->registerListener(listener, &_aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(_aidl_return);

    // Attempt to register the same listener again
    status = service->registerListener(listener, &_aidl_return);
    // Verify the registration fails the second time
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(_aidl_return);

    //cleanup
    status = service->unregisterListener(listener,&_aidl_return);
}


TEST_F(PlaneControlServiceTest, UnregisterListener_InValidListener) {
    ::android::sp<::com::rdk::hal::planecontrol::IPlaneControlListener> listener = new ValidPlaneControlListener();
    bool aidl_return = false;

    ::android::binder::Status status = service->unregisterListener(listener, &aidl_return);

    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(aidl_return);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    getService();
    return RUN_ALL_TESTS();
}
