
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

/**
* @file PlaneControlClient.cpp
* @page PlaneControl hal Level 1 Tests
*
* ## Module's Role
* This module includes Level 1 functional tests (success and failure scenarios).
* This is to ensure that the PlaneControl hal APIs meet the requirements across all vendors.
*
*/
#include <ut_kvp_profile.h>
#include <ut.h>
#include <ut_log.h>
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
#include "com/rdk/hal/planecontrol/BnPlaneControlListener.h"

using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;

using com::rdk::hal::planecontrol::IPlaneControl;
using ::com::rdk::hal::PropertyValue;
using ::com::rdk::hal::planecontrol::Property;
using namespace com::rdk::hal::planecontrol;

struct Plane {
    int id;
    std::string type;
    std::vector<std::string> pixelFormats;
    int maxWidth;
    int maxHeight;
    int frameWidth;
    int frameHeight;
    std::vector<std::string> supportedDynamicRanges;
    int colorDepth;
    int maxFrameRate;
    bool supportsAlpha;
    bool supportsZOrder;
    int vsyncDisplayLatency;
};

sp<IPlaneControl> service;
int _planeResourceIndex;
int _maxwidth;
int _maxheight;
int number_of_planes = 0;
int videosink_count = 0;
int videosink_id;
bool videosink_available_flag = false;
bool multiple_videosink_source_flag = false;
std::vector<Plane> planes;
std::vector<int> videoplanes;
std::vector<int> graphicplanes;
std::vector<int> video_and_graphic_planes;
std::vector<int> videosink_ids;

int getService() {
    service = android::interface_cast<IPlaneControl>(
            defaultServiceManager()->getService(String16(IPlaneControl::serviceName().c_str())));
    if (service == nullptr) {
        printf("Service not available\n");
        return -1;
    }
    return 0;
}


class ValidPlaneControlListener : public BnPlaneControlListener {
public:

    ValidPlaneControlListener() {};
    virtual ~ValidPlaneControlListener() = default;
};

class L1PlaneControlUnregisterTest : public UTCore
{
    protected:
        ::android::sp<IPlaneControlListener> listener;
        void SetUp() override
        {
            listener = new ValidPlaneControlListener();
            bool _aidl_return;
            // Register the listener for the first time
            Status status = service->registerListener(listener, &_aidl_return);
            EXPECT_TRUE(status.isOk());
            EXPECT_TRUE(_aidl_return);
        }
};

class L1PlaneControlPropertyTest : public virtual UTCore
{
    protected:
    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> initialpropertyKVList;
    void SetUp() override
    {
        com::rdk::hal::planecontrol::PropertyKVPair kvPair1;
        kvPair1.property = com::rdk::hal::planecontrol::Property::X;
        kvPair1.propertyValue = {}; 
        initialpropertyKVList.push_back(kvPair1);       

        com::rdk::hal::planecontrol::PropertyKVPair kvPair2;
        kvPair2.property = com::rdk::hal::planecontrol::Property::Y;
        kvPair2.propertyValue = {}; 
        initialpropertyKVList.push_back(kvPair2);  

        com::rdk::hal::planecontrol::PropertyKVPair kvPair3;
        kvPair3.property = com::rdk::hal::planecontrol::Property::WIDTH;
        kvPair3.propertyValue = {}; 
        initialpropertyKVList.push_back(kvPair3);  

        com::rdk::hal::planecontrol::PropertyKVPair kvPair4;
        kvPair4.property = com::rdk::hal::planecontrol::Property::HEIGHT;
        kvPair4.propertyValue = {}; 
        initialpropertyKVList.push_back(kvPair4);  

        com::rdk::hal::planecontrol::PropertyKVPair kvPair5;
        kvPair5.property = com::rdk::hal::planecontrol::Property::VISIBLE;
        kvPair5.propertyValue = {}; 
        initialpropertyKVList.push_back(kvPair5);  


        bool aidl_return = false;
        Status status = service->getPropertyMulti(_planeResourceIndex, &initialpropertyKVList, &aidl_return);
        EXPECT_TRUE(status.isOk());
    }

    void TearDown() override
    {
        bool aidl_return = false;
        Status status = service->setPropertyMultiAtomic(_planeResourceIndex, initialpropertyKVList, &aidl_return);
        EXPECT_TRUE(status.isOk());
    }
};


//helper function
bool populateExpectedPropertyValue(const com::rdk::hal::planecontrol::PropertyKVPair& kvPair, com::rdk::hal::planecontrol::Property expectedProperty) 
{
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

//currently Hal Feature Profile contains string value instead of enum for the parameter PixelFormat
com::rdk::hal::videodecoder::PixelFormat StringToPixelFormat(const std::string& format) 
{
    static const std::unordered_map<std::string, com::rdk::hal::videodecoder::PixelFormat> formatMap = {
        {"YCBCR422", com::rdk::hal::videodecoder::PixelFormat::YCBCR422},
        {"YCBCR444", com::rdk::hal::videodecoder::PixelFormat::YCBCR444},
        {"YCBCR420", com::rdk::hal::videodecoder::PixelFormat::YCBCR420},
        {"RGB", com::rdk::hal::videodecoder::PixelFormat::RGB},
        {"NATIVE", com::rdk::hal::videodecoder::PixelFormat::NATIVE}
    };

    auto it = formatMap.find(format);
    if (it != formatMap.end()) 
    {
        return it->second;
    }
    return static_cast<com::rdk::hal::videodecoder::PixelFormat>(-1);
}

//currently Hal Feature Profile contains string value instead of enum for the parameter DynamicRange
com::rdk::hal::videodecoder::DynamicRange StringtoDynamicRange(const std::string& dynamicrange)
{
    static const std::unordered_map<std::string, com::rdk::hal::videodecoder::DynamicRange> formatMap = {
        {"UNKNOWN", com::rdk::hal::videodecoder::DynamicRange::UNKNOWN},
        {"SDR", com::rdk::hal::videodecoder::DynamicRange::SDR},
        {"HLG", com::rdk::hal::videodecoder::DynamicRange::HLG},
        {"HDR10", com::rdk::hal::videodecoder::DynamicRange::HDR10},
        {"HDR10PLUS", com::rdk::hal::videodecoder::DynamicRange::HDR10PLUS},  
        {"DOLBY_VISION", com::rdk::hal::videodecoder::DynamicRange::DOLBY_VISION}  
    };

    auto it = formatMap.find(dynamicrange);
    if (it != formatMap.end()) 
    {
        return it->second;
    }
    return static_cast<com::rdk::hal::videodecoder::DynamicRange>(-1);
}

// TODO: PlaneResource got renamed in commit `3b32784f33bf8140bfd3dcf3e7239013583c5566` in hal.git
#if 0
/**
* @brief Test the positive scenario of getting resource capabilities in L1PlaneControlService.
*
* This test validates the proper retrieval of plane resource capabilities via the PlaneControlService, ensuring that the data obtained matches the expected values.
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
* | 01 | Retrieve resource capabilities from the service | PlaneResource = valid pointer | Resource capabilities list should be retrieved, status.isOk() should be true | Should Pass |
* | 02 | Validate that plane resources are not empty | planeResources | planeResources is not empty | Should Pass |
* | 04 | Validate size of plane resources | planeResources.size() from api , planes.size() from profile | planeResources.size() equals planes.size() | Should Pass |
* | 05 | Validate each plane's id matches | id from profile, planeIndex from api | Values should be equal | Should Pass |
* | 06 | Validate each plane's maxWidth matches | maxWidth from profile, maxWidth from api | Values should be equal | Should Pass |
* | 07 | Validate each plane's maxHeight matches | maxHeight from profile, maxHeight from api| Values should be equal | Should Pass |
* | 08 | Validate each plane's frameWidth matches | frameWidth from profile, frameWidth  from api | Values should be equal | Should Pass |
* | 09 | Validate each plane's frameHeight matches | frameHeight from profile, frameHeight  from api| Values should be equal | Should Pass |
* | 10 | Validate each plane's colorDepth matches | colorDepth from profile , colorDepth  from api | Values should be equal | Should Pass |
* | 11 | Validate each plane's maxFrameRate matches | maxFrameRate from profile, maxFrameRate   from api | Values should be equal | Should Pass |
* | 12 | Validate each plane's supportsAlpha matches | supportsAlpha from profile, supportsAlpha  from api | Values should be equal | Should Pass |
* | 13 | Validate each plane's supportsZOrder matches |supportsZOrder from profile, supportsZOrder  from api | Values should be equal | Should Pass |
* | 14 | Validate each plane's vsyncDisplayLatency matches | vsyncDisplayLatency from profile, vsyncDisplayLatency  from api| Values should be equal | Should Pass |
* | 15 | Validate size of each plane's pixel formats | pixelFormats.size() from profile, pixelFormats.size()  from api | Sizes should be equal | Should Pass |
* | 16 | Validate each plane's pixel formats by converting string to PixelFormat | pixelFormats from profile, pixelFormats  from api| Values should be equal after conversion | Should Pass |
* | 17 | Validate size of each plane's supported dynamic ranges | supportedDynamicRanges.size() from profile, supportedDynamicRanges.size()  from api | Sizes should be equal | Should Pass |
* | 18 | Validate each plane's supported dynamic ranges by converting string to DynamicRange | supportedDynamicRanges from profile, supportedDynamicRanges  from api | Values should be equal after conversion | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_1_getResourceCapabilities) 
{

    std::vector<::com::rdk::hal::planecontrol::PlaneResource> planeResources;
    ::android::binder::Status status = service->getResourceCapabilities(&planeResources);
    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(planeResources.empty());
    ASSERT_EQ(planeResources.size(),planes.size());

    for(int i=0; i<planes.size(); ++i)
    {
        EXPECT_EQ(planes[i].id,planeResources[i].planeIndex);
        EXPECT_EQ(planes[i].maxWidth,planeResources[i].maxWidth); 
        EXPECT_EQ(planes[i].maxHeight,planeResources[i].maxHeight); 
        EXPECT_EQ(planes[i].frameWidth,planeResources[i].frameWidth); 
        EXPECT_EQ(planes[i].frameHeight,planeResources[i].frameHeight); 
        EXPECT_EQ(planes[i].colorDepth,planeResources[i].colorDepth); 
        EXPECT_EQ(planes[i].maxFrameRate,planeResources[i].maxFrameRate); 
        EXPECT_EQ(planes[i].supportsAlpha,planeResources[i].supportsAlpha); 
        EXPECT_EQ(planes[i].supportsZOrder,planeResources[i].supportsZOrder);
        EXPECT_EQ(planes[i].vsyncDisplayLatency,planeResources[i].vsyncDisplayLatency);        
        ASSERT_EQ(planes[i].pixelFormats.size(),planeResources[i].pixelFormats.size());
        for(int j=0; j<planes[i].pixelFormats.size(); ++j)
        {
            EXPECT_EQ(StringToPixelFormat(planes[i].pixelFormats[j]), planeResources[i].pixelFormats[j]);
        }

        ASSERT_EQ(planes[i].supportedDynamicRanges.size(),planeResources[i].supportedDynamicRanges.size());
        for(int k=0; k<planes[i].supportedDynamicRanges.size(); ++k)
        {
            EXPECT_EQ(StringtoDynamicRange(planes[i].supportedDynamicRanges[k]), planeResources[i].supportedDynamicRanges[k]);
        }     
    }
}
#endif

/**
* @brief Tests the successful retrieval of native graphics window handle from the service.
*
* This test ensures that the `getNativeGraphicsWindowHandle` function consistently retrieves valid window handles from the service for each available graphic plane defined in `graphicplanes`. This is crucial for managing graphics and rendering behavior.
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
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Check if `graphicplanes` from profile is not empty | graphicplanes = vector contain graphic plane index from profile | graphicplanes vector should contain data | Should be successful |
* | 02| Call `getNativeGraphicsWindowHandle` for each graphic plane index | graphicplaneindex = valid index , &_aidl_return = valid pointer | status.isOk() should be true, _aidl_return should be valid graphic window handle | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_1_getNativeGraphicsWindowHandle) 
{ 
    int64_t _aidl_return;
    Status status;
    ASSERT_FALSE(graphicplanes.empty()) << "NO GRAPHIC PLANES AVAILABLE";
    for (const auto& graphicplaneindex : graphicplanes)
    {
        status = service->getNativeGraphicsWindowHandle(graphicplaneindex, &_aidl_return);
        EXPECT_TRUE(status.isOk());
    }
}

/**
* @brief Negative test case for getNativeGraphicsWindowHandle API
*
* Tests the behavior of getNativeGraphicsWindowHandle API when called with video planes that does not exist which should return an illegal argument exception@n
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
* | 01 | Check that the collection of video planes from profile is not empty | videoplanes | Assertion should pass | Should be successful |
* | 02 | Iterate through each video plane index and invoke getNativeGraphicsWindowHandle | videoplaneindex, _aidl_return | status.exceptionCode() == EX_ILLEGAL_ARGUMENT | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_1_getNativeGraphicsWindowHandle) 
{
    int64_t _aidl_return;
    Status status;
    ASSERT_FALSE(videoplanes.empty()) << "NO VIDEO PLANES AVAILABLE";
    for (const auto& videoplaneindex : videoplanes)
    {    
        status = service->getNativeGraphicsWindowHandle(videoplaneindex, &_aidl_return);
        EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }

}

/**
* @brief Test for verifying positive scenarios of getNativeGraphicsWindowHandle with L1PlaneControlService.
*
* This test ensures that the function getNativeGraphicsWindowHandle is able to successfully retrieve the native graphics window handle 
* for each graphic plane in the list twice and confirms that the retrieved handles match, thereby validating the consistency of the service.
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
* | 01 | Check if graphicplanes list is not empty | None | Assertion should succeed if there are graphic planes | Should be successful |
* | 02 | Loop through each graphic plane index from profile and call getNativeGraphicsWindowHandle for the first time | graphicplaneindex from graphicplanes, _aidl_return1 | status.isOk() should be true and _aidl_return1 should have valid handle | Should be successful |
* | 03 | Call getNativeGraphicsWindowHandle for the second time for the same graphic plane index | graphicplaneindex from graphicplanes, _aidl_return2 | status.isOk() should be true and _aidl_return2 should have valid handle | Should be successful |
* | 04 | Compare both retrieved handles | _aidl_return1, _aidl_return2 | Both handles should be equal | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_2_getNativeGraphicsWindowHandle) 
{
    int64_t _aidl_return1, _aidl_return2;
    Status status;

    ASSERT_FALSE(graphicplanes.empty()) << "NO GRAPHIC PLANES AVAILABLE";
    for (const auto& graphicplaneindex : graphicplanes)
    {
        status = service->getNativeGraphicsWindowHandle(graphicplaneindex, &_aidl_return1);
        EXPECT_TRUE(status.isOk());

        status = service->getNativeGraphicsWindowHandle(graphicplaneindex, &_aidl_return2);
        EXPECT_TRUE(status.isOk());

        EXPECT_EQ(_aidl_return1, _aidl_return2);
    }
}

/**
* @brief This test function validates the positive scenario of releasing Native Graphics Window Handle.
*
* This test ensures that the native graphics window handle can be correctly retrieved and subsequently released for all available graphic planes.
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
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Verify that `graphicplanes` from the profile is not empty to ensure there are planes available for testing. | | graphicplanes.empty() should return false | Should be successful |
* | 02| For each graphic plane index, call `getNativeGraphicsWindowHandle` to obtain the native window handle. | graphicplaneindex = valid value from profile, &nativeWindowHandle = valid pointer | status.isOk() should be true | Should Pass |
* | 03| For each graphic plane index, call `releaseNativeGraphicsWindowHandle` to release the native window handle. | graphicplaneindex = valid value, nativeWindowHandle = valid value | status.isOk() should be true | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_1_releaseNativeGraphicsWindowHandle) 
{ 
    int64_t nativeWindowHandle;
    Status status;

    ASSERT_FALSE(graphicplanes.empty()) << "NO GRAPHIC PLANES AVAILABLE";
    for (const auto& graphicplaneindex : graphicplanes)
    {
        status = service->getNativeGraphicsWindowHandle(graphicplaneindex, &nativeWindowHandle);
        EXPECT_TRUE(status.isOk());

        status = service->releaseNativeGraphicsWindowHandle(graphicplaneindex, nativeWindowHandle);
        EXPECT_TRUE(status.isOk()); 
    }      

}

/**
* @brief This test is designed to check the handling of invalid parameters in the `releaseNativeGraphicsWindowHandle` function.
*
* This test focuses on providing a negative test case to ensure that the service correctly identifies and handles 
* illegal arguments when trying to release a native graphics window handle. The importance of this test lies in 
* verifying robustness and error handling of the service for invalid input scenarios. 
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
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Ensure there are available graphic planes from the profile | graphicplanes = vector containig graphicplanes from profile  | Assertion pass if graphic planes are available | Should be successful |
* | 02| For each graphic plane index in the list, attempt to release a negative window handle (-10) | graphicplaneindex = Index from list, nativeWindowHandle = -10 | status.transactionError() should be EX_ILLEGAL_ARGUMENT | Should fail |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_1_releaseNativeGraphicsWindowHandle) 
{ 
    int64_t nativeWindowHandle = -10; //random number for negative test
    Status status;

    ASSERT_FALSE(graphicplanes.empty()) << "NO GRAPHIC PLANES AVAILABLE";
    for (const auto& graphicplaneindex : graphicplanes)
    {
        status = service->releaseNativeGraphicsWindowHandle(graphicplaneindex, nativeWindowHandle);
        EXPECT_EQ(status.transactionError(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }
}

/**
* @brief Test the releaseNativeGraphicsWindowHandle function with an invalid plane resource index
*
* This test verifies the behavior of the API releaseNativeGraphicsWindowHandle when given an invalid plane resource index. This is important to ensure that the API handles improper inputs gracefully and returns appropriate errors.
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
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Verify graphicplanes is not empty | N/A | Should be successful | |
* | 02 | Get native graphics window handle | graphicplaneindex = value, nativeWindowHandle = value | status = Ok, Should Pass | |
* | 03 | Invoke releaseNativeGraphicsWindowHandle with invalid plane resource index | planeResourceIndex = -5, nativeWindowHandle = valid value | status.transactionError() = EX_ILLEGAL_ARGUMENT | Should fail |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_2_releaseNativeGraphicsWindowHandle) 
{ 
    int64_t nativeWindowHandle;
    Status status;
    int InvalidplaneResourceIndex = -5;

    ASSERT_FALSE(graphicplanes.empty()) << "NO GRAPHIC PLANES AVAILABLE";
    for (const auto& graphicplaneindex : graphicplanes)
    {
        status = service->getNativeGraphicsWindowHandle(graphicplaneindex, &nativeWindowHandle);
        EXPECT_TRUE(status.isOk()); 
        //API invocation with invalid planeResourceIndex and valid nativeWindowHandle
        status = service->releaseNativeGraphicsWindowHandle(InvalidplaneResourceIndex, nativeWindowHandle);
        EXPECT_EQ(status.transactionError(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }
}

/**
* @brief This test verifies the positive scenario for flipping graphics buffers in L1PlaneControlService.
*
* This test iterates over a list of graphic plane indices and calls the flipGraphicsBuffer function for each index to ensure the function works correctly for all provided indices.
* @n
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 008 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Call flipGraphicsBuffer with each graphic plane index | graphicplaneindex = valid index from profile | status.isOk() = true | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_1_flipGraphicsBuffer) 
{
    Status status;
    bool aidl_return = false;
    for (const auto& graphicplaneindex : graphicplanes)
    {
        Status status = service->flipGraphicsBuffer(graphicplaneindex, &aidl_return);
        EXPECT_TRUE(status.isOk());
    }
}

/**
* @brief This test case validates the flipGraphicsBuffer method in a negative scenario.
*
* Tests if the flipGraphicsBuffer method handles illegal arguments correctly by checking whether it returns the EX_ILLEGAL_ARGUMENT exception code, by inputing videoplane index. This ensures robustness in input validation.@n
* @n
* **Test Group ID:** Basic (for L1): 01@n
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
* | 01 | Check if videoplanes are listed in the profile, by checking the vector  videoplanes | videoplanes.empty() | Assertion passes if not empty | Should Pass |
* | 02 | Iterate over each videoplane index from profile and call flipGraphicsBuffer | videoplaneindex = value from profile | status.exceptionCode() = EX_ILLEGAL_ARGUMENT | Should fail |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_1_flipGraphicsBuffer) 
{
    Status status;

    ASSERT_FALSE(videoplanes.empty()) << "NO VIDEO PLANES AVAILABLE";
    for (const auto& videoplaneindex : videoplanes)
    {          
        bool aidl_return = false;
        Status status = service->flipGraphicsBuffer(videoplaneindex, &aidl_return);
        EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }   
}

/**
* @brief This test validates the setting of video source to destination plane mappings.
*
* This test checks the functionality of the setVideoSourceDestinationPlaneMapping API by setting valid video sources to available destination planes and ensuring the status is OK. It also resets the mappings as part of clean-up.
* @n
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 010 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None @n
* **Dependencies:** None @n
* **User Interaction:** None @n
*
* **Test Procedure:**
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Ensure that there is mulitple videosink and videoplane are available| multiple_videosi:%snk_source_flag, videoplanes.size()  | multiple_videosink_source_flag=true, videoplanes.size()>1 |  |
* | 02| Initialize listSourcePlaneMapping and set sourceType| listSourcePlaneMapping[0].sourceType=VIDEO_SINK | Should be successful |  |
* | 03| Loop through videosink_count and videoplanes| i=0 to videosink_count, j=0 to videoplanes.size() |  | Should be successful |
* | 04| Set sourceIndex and destinationPlaneIndex in listSourcePlaneMapping and call setVideoSourceDestinationPlaneMapping| listSourcePlaneMapping = valid data | status.isOk()=true | Should Pass |
* | 05| Clean-up by resetting sourceIndex and destinationPlaneIndex| sourceIndex= each video sink id from profile, destinationPlaneIndex=-1 | status.isOk()=true | Should Pass |
*/
// setVideoSourceDestinationPlane() is refactored to setVideoSourceDestinationPlaneMapping() in
// fac3ca8b19af81d2fb1f2ae56e754e73a1efb28f in hal.git
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_1_setVideoSourceDestinationPlaneMapping)
{
    if(multiple_videosink_source_flag && (videoplanes.size()>1))
    {
        std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(1);
        listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
        bool aidl_return = false;
        for(int i=0; i<videosink_count; ++i)
        {
            for(int j=0; j<videoplanes.size(); ++j)
            {
                listSourcePlaneMapping[0].sourceIndex = videosink_ids[i];
                listSourcePlaneMapping[0].destinationPlaneIndex = videoplanes[j];  
                Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping, &aidl_return);
                EXPECT_TRUE(status.isOk());
            }
        }
        //clean up
        for(int k=0; k<videosink_count; ++k)
        {
            listSourcePlaneMapping[0].sourceIndex = videosink_ids[k];
            listSourcePlaneMapping[0].destinationPlaneIndex = -1;
            Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping, &aidl_return);
            EXPECT_TRUE(status.isOk());
        }
    }
    else
    {
        GTEST_SKIP() << "Skipping this test due to multiple videosink unavailability OR multiple videoplane unavailability in the profile";
    }
}

/**
* @brief This test verifies that the setVideoSourceDestinationPlaneMapping function sets the correct video source to destination planes.
*
* This test checks the functionality of setting and then resetting video source to destination plane mapping. It ensures that the service correctly handles multiple video sinks and planes by mapping them and then cleaning up.
* @n
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 011 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None @n
* **Dependencies:** None @n
* **User Interaction:** None @n
* @n 
* **Test Procedure:**
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Check if multiple videosink source and videoplane are available | multiple_videosink_source_flag, videoplanes.size() | multiple_videosink_source_flag = true, videoplanes.size() > 1 | Should be successful |
* | 02 | Create a list of SourcePlaneMapping for two videosinks | listSourcePlaneMapping initialized with 2 mappings | Should be successful | Should be successful |
* | 03 | Set the first sourceType to VIDEO_SINK,  sourceIndex = first video sink id , and destinationPlaneIndex = first video plane index| listSourcePlaneMapping[0]  | setting listSourcePlaneMapping[0] Should be successful | Should be successful |
* | 04 | Set the second sourceType to VIDEO_SINK, second sourceIndex = second video sink id, and second destinationPlaneIndex = second video plane index | listSourcePlaneMapping[1] | setting listSourcePlaneMapping[1]  Should be successful | Should be successful |
* | 05 | Call setVideoSourceDestinationPlaneMapping with the mapping list | listSourcePlaneMapping with 2 mappings | status.isOk() should be true | Should Pass |
* | 06 | Reset first destinationPlaneIndex to -1 | listSourcePlaneMapping[0].destinationPlaneIndex = -1 | Should be successful | Should be successful |
* | 07 | Reset second destinationPlaneIndex to -1 | listSourcePlaneMapping[1].destinationPlaneIndex = -1 | Should be successful | Should be successful |
* | 08 | Call setVideoSourceDestinationPlaneMapping with the reset mapping list | listSourcePlaneMapping with reset mappings | status.isOk() should be true | Should Pass |
* | 09 | Skip test if multiple videosinks or videoplanes are unavailable | Condition check and GTEST_SKIP() call | Test should be skipped | Should be successful |
*/
// setVideoSourceDestinationPlaneMapping() is refactored to setVideoSourceDestinationPlaneMapping() in
// fac3ca8b19af81d2fb1f2ae56e754e73a1efb28f in hal.git
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_2_setVideoSourceDestinationPlaneMapping) 
{
    if(multiple_videosink_source_flag && (videoplanes.size()>1))
    {
        std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(2);
        listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
        listSourcePlaneMapping[0].sourceIndex = videosink_ids[0];
        listSourcePlaneMapping[0].destinationPlaneIndex = videoplanes[0];

        listSourcePlaneMapping[1].sourceType = ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
        listSourcePlaneMapping[1].sourceIndex = videosink_ids[1];
        listSourcePlaneMapping[1].destinationPlaneIndex = videoplanes[1];

        bool aidl_return = false;
        Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping, &aidl_return);
        EXPECT_TRUE(status.isOk());

        //clean up
        listSourcePlaneMapping[0].destinationPlaneIndex = -1;
        listSourcePlaneMapping[1].destinationPlaneIndex = -1;

        status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping, &aidl_return);
        EXPECT_TRUE(status.isOk());
    }
    else
    {
        GTEST_SKIP() << "Skipping this test due to multiple videosink unavailability OR multiple videoplane unavailability in the profile";
    }
}

/**
* @brief This test case verifies the behavior of the setVideoSourceDestinationPlaneMapping API when given an invalid sourceType.
*
* This test ensures that the setVideoSourceDestinationPlaneMapping API handles the case where the sourceType is set to NONE, which should be an invalid argument.
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
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Setup input data| listSourcePlaneMapping: sourceType = NONE, sourceIndex = 0, destinationPlaneIndex = 0 | SourcePlaneMapping setup successful | Should be successful |
* | 02 | Call setVideoSourceDestinationPlaneMapping API | listSourcePlaneMapping: sourceType = NONE, sourceIndex = 0, destinationPlaneIndex = 0 | status.exceptionCode() = android::binder::Status::EX_ILLEGAL_ARGUMENT | Should fail |
*/
// setVideoSourceDestinationPlaneMapping() is refactored to setVideoSourceDestinationPlaneMapping() in
// fac3ca8b19af81d2fb1f2ae56e754e73a1efb28f in hal.git
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_1_setVideoSourceDestinationPlaneMapping) 
{
    // Setup input
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(1);
    listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::NONE;
    listSourcePlaneMapping[0].sourceIndex = 0;
    listSourcePlaneMapping[0].destinationPlaneIndex = 0;

    // Call API
    bool aidl_return = false;
    Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping, &aidl_return);
    EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

/**
* @brief Test the setVideoSourceDestinationPlaneMapping function with invalid inputs
*
* This test case aims to validate whether the `setVideoSourceDestinationPlaneMapping` function handles invalid input correctly. 
* It checks for the appropriate error response when `listSourcePlaneMapping` is provided with incorrect parameters.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 013 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None @n
* **Dependencies:** None @n
* **User Interaction:** None @n
* @n
* **Test Procedure:** @n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Check if videosink and graphic planes are available | videosink_available_flag, graphicplanes.size() | Should proceed with test or skip | Should be successful |
* | 02 | Initialize SourcePlaneMapping object with one element | listSourcePlaneMapping = 1 | Should be successful | |
* | 03 | Set listSourcePlaneMapping sourceType to VIDEO_SINK | listSourcePlaneMapping[0].sourceType = VIDEO_SINK | Should be successful | |
* | 04 | Set listSourcePlaneMapping sourceIndex to   first videosink id from profile | listSourcePlaneMapping[0].sourceIndex = videosink_ids[0] | Should be successful | |
* | 05 | Set listSourcePlaneMapping destinationPlaneIndex to first graphic plane id from profile | listSourcePlaneMapping[0].destinationPlaneIndex = graphicplanes[0] | Should be successful | |
* | 06 | Invoke setVideoSourceDestinationPlaneMapping with listSourcePlaneMapping | listSourcePlaneMapping = SourcePlaneMapping object  | status.exceptionCode() should be EX_ILLEGAL_ARGUMENT | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_2_setVideoSourceDestinationPlaneMapping) 
{
    if(videosink_available_flag && graphicplanes.size())
    {
        std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> listSourcePlaneMapping(1);
        listSourcePlaneMapping[0].sourceType = ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
        listSourcePlaneMapping[0].sourceIndex = videosink_ids[0];
        listSourcePlaneMapping[0].destinationPlaneIndex = graphicplanes[0];

        bool aidl_return = false;
        Status status = service->setVideoSourceDestinationPlaneMapping(listSourcePlaneMapping, &aidl_return);
        EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }
    else
    {
        GTEST_SKIP() << "Skipping this test due to  videosink unavailability OR  graphic plane unavailability in the profile";
    }
}

/**
* @brief Test to verify the functionality of getProperty()
*
* This test verifies the behavior of the getProperty() function by fetching
* the property value of various plane resources and asserts that the status is OK 
* and the resulting property value is valid.
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
* | Variation / Step | Description                          | Test Data                                   |Expected Result       |Notes               |
* | :--------------: | ------------------------------------ | ------------------------------------------ |---------------------|--------------------|
* | 01               | Initialize variables and lists       | property = Property::X, result = valid object of PropertyValue, status     | Should be successful|                    |
* | 02               | Start loop through plane resources   | video_and_graphic_planes from profile                            | Should be successful|                    |
* | 03               | Get property for planeResourceIndex  | planeResourceIndex = valid index from profile, property = X , &result = valid pointer      |  status.isOk() should be true        |     Should Pass     |
* | 04               | Assert result has value              | result.value.has_value()                    | Should Pass         |                    |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_1_getProperty) 
{
    Property property = Property::X;
    std::optional<::com::rdk::hal::PropertyValue> result;
    Status status;
    for (const auto& planeResourceIndex : video_and_graphic_planes)
    {
        status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value().value.has_value());
    }
}

/**
* @brief Test the getProperty method of the L1PlaneControlService for positive scenarios
*
* This test validates the successful retrieval of a property value of the property Y for each plane in the video_and_graphic_planes list using the getProperty method of the L1PlaneControlService. The test ensures that the method call returns an OK status and that the result contains a valid value.
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
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Setup the necessary variables and objects for testing | property = Property::Y, result = valid object of PropertyValue, status | Variables initialized successfully | Should be successful |
* | 02| Iterate over each plane in the video_and_graphic_planes list | planeResourceIndex from video_and_graphic_planes | Each iteration processes a plane | Should be successful |
* | 03| Call the getProperty method with the current planeResourceIndex and property | planeResourceIndex = value from profile, property = Property::Y, &result | status.isOk() should be true | Should Pass |
* | 04| Verify that the resulting property value has a valid value | result.value | result.value.has_value() is true | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_2_getProperty) 
{
    Property property = Property::Y;
    std::optional<::com::rdk::hal::PropertyValue> result;
    Status status;
    for (const auto& planeResourceIndex : video_and_graphic_planes)
    {
        status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value().value.has_value());
    }
}

/**
* @brief Test case to validate the negative scenario of the getProperty function when an invalid planeResourceIndex is passed.
*
* This test case is designed to verify the response of the getProperty function in scenarios where the provided 
* planeResourceIndex is outside the valid range. The purpose is to ensure that the function handles such cases
* gracefully without crashing and returns the appropriate error code.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 016 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description                     | Test Data                              | Expected Result                                                             | Notes       |
* | :----:           | -----------                     | ----------                             | --------------                                                              | -----       |
* | 01               | Setup and initialize variables  | planeResourceIndex = -10, property = Property::X, result = nullptr  | Variables initialized and ready                                           | Should be successful |
* | 02               | Call getProperty API            | planeResourceIndex = -10, property = Property::X, result = nullptr  | status.exceptionCode() should match EX_ILLEGAL_ARGUMENT | Should Fail |
*/

TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_1_getProperty) 
{
    int32_t planeResourceIndex = -10;
    Property property = Property::X;
    std::optional<::com::rdk::hal::PropertyValue> result;

    Status status = service->getProperty(planeResourceIndex, property, &result);
    ASSERT_FALSE(status.isOk());
    EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

/**
* @brief This test verifies the successful retrieval of a property from the plane control service.
*
* This test is designed to check the correct functionality of retrieving a property value (WIDTH) from various plane resources. It ensures that the service responds with valid data.
* @n
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 017 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Iterate through each video and graphic plane resource index from profile | planeResourceIndex from video_and_graphic_planes, Property::WIDTH, result | status.isOk() = true, result.value.has_value() = true | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_3_getProperty) 
{
    Property property = Property::WIDTH;
    std::optional<::com::rdk::hal::PropertyValue> result;

    Status status;
    for (const auto& planeResourceIndex : video_and_graphic_planes)
    {
        status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value().value.has_value());
    }
}

/**
* @brief Tests the getProperty function with an invalid property value.
*
* This test verifies that the getProperty function of the L1PlaneControlService returns an error when called with an invalid property value. 
* Specifically, it checks that the function fails with an appropriate status code when the property value is set to -1, which is not a valid property.
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
* | 01 | Initialize Property variable with an invalid value (-1) | property = -1 | Should be successful | Should be successful |
* | 02 | Declare result and status variables | result, status initialized | Variables initialization success | Should be successful|
* | 03 | Loop through each plane resource index and call getProperty | planeResourceIndex = valid id from profile, property = -1 , &result = valid pointer for holding PropertyValue| status.exceptionCode() = EX_ILLEGAL_ARGUMENT | Should fail |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_2_getProperty) 
{
    Property property = static_cast<Property>(-1);
    std::optional<::com::rdk::hal::PropertyValue> result;
    Status status;
    for (const auto& planeResourceIndex : video_and_graphic_planes)
    {
        status = service->getProperty(planeResourceIndex, property, &result);
        EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }
}

/**
* @brief Validate getting the height property of video and graphic planes
*
* This test verifies the successful retrieval of the height property for various video and graphic planes using the getProperty function.
* @n
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 019 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None @n
* **Dependencies:** None @n
* **User Interaction:** None @n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |----------- | ----- |
* | 01 | Initialize property to Property::HEIGHT and create result and status variables. | property = Property::HEIGHT, result = ::com::rdk::hal::PropertyValue(), status = Status() | Should be successful | Should be successful |
* | 02 | Loop through each planeResourceIndex in video_and_graphic_planes and call getProperty(). | planeResourceIndex = video_and_graphic_planes from profile, property = Property::HEIGHT, &result = valid pointer | status.isOk() should be true | Should Pass |
* | 03 | Assert that the result's value has a value. | result.value | result.value.has_value() == true | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_4_getProperty) 
{
    Property property = Property::HEIGHT;
    std::optional<::com::rdk::hal::PropertyValue> result;
    Status status;
    for (const auto& planeResourceIndex : video_and_graphic_planes)
    {
        status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value().value.has_value());
    }
}

/**
* @brief Tests the getProperty function to validate obtaining properties from video and graphic planes.
*
* This test seeks to retrieve the Property::ALPHA from various plane resources and verify the returned property value using the getProperty API. Ensuring that the function call succeeds and the expected properties are retrieved.
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
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Initialize the `Property` object with `Property::ALPHA` and declare `::com::rdk::hal::PropertyValue` result| Property property = Property::ALPHA; ::com::rdk::hal::PropertyValue result | Should be successful | |
* | 02| Iterate through the `video_and_graphic_planes` index from profile and call the `getProperty` function for each plane index to retrieve the ALPHA property value| planeResourceIndex from video_and_graphic_planes, property = Property::ALPHA, &result | status.isOk() should be true and `result.value` should have the property value | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_5_getProperty) 
{
    Property property = Property::ALPHA;
    std::optional<::com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : video_and_graphic_planes)
    {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value().value.has_value());
    }
}

/**
* @brief Test to check the positive scenario for getting property value in L1PlaneControlService
*
* This test verifies that the getProperty function correctly retrieves the property valuecof ZORDER for each plane resource index available.
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
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Iterate over each plane resource index in video_and_graphic_planes from profile, For each plane resource index, call getProperty with property and result | planeResourceIndex, property = ZORDER, &result | status.isOk() should be true, result.value.has_value() should be true | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_6_getProperty) 
{
    Property property = Property::ZORDER;
    std::optional<::com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : video_and_graphic_planes)
    {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value().value.has_value());
    }
}

/**
* @brief Test to check the 'getProperty' function of L1PlaneControlService.
*
* This test checks the functionality of the 'getProperty' method in the L1PlaneControlService class by verifying that the correct properties are returned for given plane resources.
*@n
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 022@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
*
* **Test Procedure:**
* | Variation / Step | Description | Test Data                                   | Expected Result                           | Notes                           |
* | :--------------: | ----------- | ------------------------------------------- | ----------------------------------------- | ------------------------------- |
* | 01               | Iterate through video_and_graphic_planes index from profile and call getProperty  | planeResourceIndex = valid value, property = VISIBLE, &result = valid pointer | status.isOk() should be true and result has value | Should Pass                    |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_7_getProperty) 
{
    Property property = Property::VISIBLE;
    std::optional<::com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : video_and_graphic_planes)
    {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value().value.has_value());
    }
}

/**
* @brief Unit test for L1PlaneControlService that verifies positive scenario for getProperty function.
*
* This test checks the functionality of the getProperty method in the L1PlaneControlServiceTest class by verifying that it correctly retrieves the Property::OVERSCAN property for different video planes and that the result value is present.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 023@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Retrieve Property::OVERSCAN for each video plane | planeResourceIndex = video plane index from profile, property = Property::OVERSCAN, &result | status.isOk() should be true and result.value has value | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_8_getProperty) 
{
    Property property = Property::OVERSCAN;
    std::optional<::com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : videoplanes) 
    {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value().value.has_value());
    }
}

/**
* @brief Test the getProperty function of L1PlaneControlService
*
* This function tests the functionality of the getProperty API to ensure 
* it correctly retrieves the specified property (ASPECT_RATIO) and fills 
* the result argument appropriately. This test validates that the property retrieval 
* succeeds for each video plane resource index specified in videoplanes.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 024@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set the property to ASPECT_RATIO | property = Property::ASPECT_RATIO | Should be successful |  |
* | 02 | Initialize the result object | result = uninitialized | Should be successful |  |
* | 03 | Iterate through each plane resource in videoplanes | videoplanes from profile | Should be successful |  |
* | 04 | Call getProperty function for each videoplanes from profile  | planeResourceIndex = valid video plane index, property ASPECT_RATIO , &result = valid pointer  | status.isOk() should be true| Should be successful |
* | 06 | Verify that result value is set correctly | result.value |  result.value.has_value() should be true | Should be successful |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_9_getProperty) 
{
    Property property = Property::ASPECT_RATIO;
    std::optional<::com::rdk::hal::PropertyValue> result;
   for (const auto& planeResourceIndex : videoplanes) 
   {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value().value.has_value());
    }
}

// METRIC_FRAMES_RECEIVED is removed in 3b32784f33bf8140bfd3dcf3e7239013583c5566 in hal.git
#if 0
/**
* @brief This test checks the functionality of the getProperty method for verifying if the metric frames received are correctly retrieved for each plane resource index.
*
* The test validates whether the getProperty method correctly fetches the METRIC_FRAMES_RECEIVED property for each of the video and graphic planes, and ensures that the operation completes successfully with valid results.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 025@n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Iterate through each plane resource index | planeResourceIndex from video_and_graphic_planes container | None | Should be successful |
* | 02| For each planeResourceIndex, call getProperty method to fetch METRIC_FRAMES_RECEIVED property | planeResourceIndex=current index, property=METRIC_FRAMES_RECEIVED, result=output container | Status.isOK() should be true and result.value has_value is true | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_10_getProperty) 
{
    Property property = Property::METRIC_FRAMES_RECEIVED;
    std::optional<::com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : video_and_graphic_planes)
    {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value().value.has_value());
    }
}
#endif

// METRIC_FRAMES_PRESENTED are removed in 3b32784f33bf8140bfd3dcf3e7239013583c5566 in hal.git
#if 0
/**
* @brief Test to verify the retrieval of a specific property from the plane control service.
*
* This test checks if the L1PlaneControlService is able to successfully retrieve the METRIC_FRAMES_PRESENTED property from video and graphic planes. It verifies that the status returned is OK and the retrieved property value is present.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 026@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Retrieve the METRIC_FRAMES_PRESENTED property for each plane resource index. | planeResourceIndex = value from video_and_graphic_planes, property = METRIC_FRAMES_PRESENTED | Status.isOK() should be true, result.value should have a value | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_11_getProperty) 
{
    Property property = Property::METRIC_FRAMES_PRESENTED;
    std::optional<::com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : video_and_graphic_planes)
    {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value().value.has_value());
    }
}
#endif

// METRIC_FRAMES_DROPPED_LATE is removed in 3b32784f33bf8140bfd3dcf3e7239013583c5566 in hal.git
#if 0
/**
* @brief Test to verify the getProperty function in PlaneControl
*
* This test checks the positive case for the getProperty using the Property::METRIC_FRAMES_DROPPED_LATE to ensure it returns the expected result.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 027 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None @n
* **Dependencies:** None @n
* **User Interaction:** None @n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set property to METRIC_FRAMES_DROPPED_LATE | property = Property::METRIC_FRAMES_DROPPED_LATE | Should be successful | Should be successful |
* | 02 | Loop through each planeResourceIndex in videoplanes and call getProperty | planeResourceIndex = valid video plane index, property = Property::METRIC_FRAMES_DROPPED_LATE, result = <PropertyValue> | Status.isOk should be true, result.value should have value | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_12_getProperty) 
{
    Property property = Property::METRIC_FRAMES_DROPPED_LATE;
    std::optional<::com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : videoplanes) 
    {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value().value.has_value());
    }
}
#endif

// METRIC_FRAMES_DROPPED_FRC is removed in 3b32784f33bf8140bfd3dcf3e7239013583c5566 in hal.git
#if 0
/**
* @brief Test the function to get property of plane control service
*
* This test checks the functionality of the getProperty method in the plane control service for retrieving the metric frames dropped property.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 028@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Retrieve the value of Property METRIC_FRAMES_DROPPED_FRC for each video plane resource available. | planeResourceIndex = valid  plane index, property = METRIC_FRAMES_DROPPED_FRC | status.isOk() = true, result.value.has_value() = true | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_13_getProperty) 
{
    Property property = Property::METRIC_FRAMES_DROPPED_FRC;
    std::optional<::com::rdk::hal::PropertyValue> result;
    for (const auto& planeResourceIndex : videoplanes) 
    {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value().value.has_value());
    }
}
#endif

// METRIC_FRAMES_REPEATED_FRC is removed in 3b32784f33bf8140bfd3dcf3e7239013583c5566 in hal.git
#if 0
/**
* @brief Test the `getProperty` function of the L1PlaneControlService
*
* This test checks the functionality of the `getProperty` method in the L1PlaneControlService by ensuring it returns a valid property value for different video plane resources.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 029@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Assign the property to Property::METRIC_FRAMES_REPEATED_FRC. | property = Property::METRIC_FRAMES_REPEATED_FRC | Initialization should be correct | Should be successful |
* | 02| Iterate over each plane resource index in videoplanes. | planeResourceIndex = video plane index from profile | Loop should iterate over all elements | Should be successful |
* | 03| Call the service's getProperty method for each plane resource. | planeResourceIndex = valid videoplanes index, property = Property::METRIC_FRAMES_REPEATED_FRC, result = valid PropertyValue object | status.isok() should be true and result value should be present | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_14_getProperty) 
{
    Property property = Property::METRIC_FRAMES_REPEATED_FRC;
    ::com::rdk::hal::PropertyValue result;
    for (const auto& planeResourceIndex : videoplanes) 
    {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value.has_value());
    }
}
#endif

// METRIC_FRAMES_REPEATED_MISSING_FRAME is removed in 3b32784f33bf8140bfd3dcf3e7239013583c5566 in hal.git
#if 0
/**
* @brief Tests whether the 'getProperty' function correctly retrieves the PropertyValue for various videoplane resources.
*
* This test iterates over various videoplane resources and checks if the getProperty function correctly returns the
* expected PropertyValue for each resource. The function specifically tests the Property::METRIC_FRAMES_REPEATED_MISSING_FRAME property.@n
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 030@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Initialize Property and result variables | Property = Property::METRIC_FRAMES_REPEATED_MISSING_FRAME, result = default initialized | Initialization should be successful | Should be successful |
* | 02| Iterate over each videoplane resource and invoke getProperty function | planeResourceIndex = iterated value, property = Property::METRIC_FRAMES_REPEATED_MISSING_FRAME, result = reference to result variable | status.isOk() should be true, result value should have value | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_15_getProperty) 
{
    Property property = Property::METRIC_FRAMES_REPEATED_MISSING_FRAME;
    ::com::rdk::hal::PropertyValue result;
    for (const auto& planeResourceIndex : videoplanes) 
    {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value.has_value());
    }
}
#endif

// METRIC_UNDERFLOWED is removed in 3b32784f33bf8140bfd3dcf3e7239013583c5566 in hal.git
#if 0
/**
* @brief Test L1PlaneControlService functionality for retrieving a specific property.
*
* This test checks the ability of the L1PlaneControlService to successfully retrieve the 
* property `METRIC_UNDERFLOWED` for all video planes defined in the `videoplanes` collection.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 031@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Initialize the test data for the `property` and `result` variables. | property = Property::METRIC_UNDERFLOWED, result = PropertyValue() | Should be successful |  Should be successful |
* | 02 | Loop through each `planeResourceIndex` in `videoplanes`. | planeResourceIndex = Each index in videoplanes collection from profile | Should be successful | Should be successful |
* | 03 | Call `getProperty` for each `video plane index` with `property` and assert the status. | planeResourceIndex = Each index, property = Property::METRIC_UNDERFLOWED, result = valid pointer | status.isOk() should be true | Should Pass |
* | 04 | Assert that the result has a value if the status is OK. | result.value.has_value() | result.value.has_value() Should be True | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_16_getProperty) 
{
    Property property = Property::METRIC_UNDERFLOWED;
    ::com::rdk::hal::PropertyValue result;
    for (const auto& planeResourceIndex : videoplanes) 
    {
        Status status = service->getProperty(planeResourceIndex, property, &result);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(result.value.has_value());
    }
}
#endif

/**
 * @brief Test the setProperty function of L1PlaneControlService for positive scenario.
 *
 * This test checks the setProperty method by first retrieving the initial property
 * value of a plane's property X, setting a new property value, and then restoring the initial value.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 032@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Get the initial property value for each plane | planeResourceIndex = valid value, property = X, &initialvalue | status.isOk() should be true | Should be successful |
 * | 02 | Set the new property value to 100 for each plane | planeResourceIndex  = valid value, property = X, value = 100 | status.isOk() should be true | Should Pass |
 * | 03 | Clean up: Restore the initial property value for each plane | planeResourceIndex = valid value, property = X, initialvalue | status.isOk() should be true | Should be successful | 
 */
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_1_setProperty) 
{
    Property property = Property::X;
    PropertyValue value;
    std::optional<PropertyValue> initialvalue;
	bool aidl_return = false;
    for (const auto& planeResourceIndex : video_and_graphic_planes)
    {
        Status status = service->getProperty(planeResourceIndex, property, &initialvalue);
        ASSERT_TRUE(status.isOk());

        value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(100);
        status = service->setProperty(planeResourceIndex, property, value, &aidl_return);
        ASSERT_TRUE(status.isOk());

        //clean up
        status = service->setProperty(planeResourceIndex, property, initialvalue.value(), &aidl_return);
        ASSERT_TRUE(status.isOk());        
    }
}

/**
* @brief This test validates setting a property in L1 Plane Control Service positively.
*
* This test ensures that a property 'Y' can be correctly set and reset on different plane resources. 
* The initial value of the property is obtained and stored, then set to a new value. After validating 
* the set operation, the property is reset to its initial value.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 033@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- |  ----- |
* | 01 | Get initial value of the Property::Y of each plane resource | planeResourceIndex = value from profile, property = Property::Y | status.isOk() should be true | Should be successful |
* | 02 | Set the property value to 200 for each plane resource | planeResourceIndex = value from profile, property = Property::Y, value = 200 | status.isOk() should be true, ASSERT_TRUE passes | Should Pass |
* | 03 | Reset the property value to its initial value for each plane resource | planeResourceIndex = value from profile, property = Property::Y, value = initialvalue | status.isOk() should be true, ASSERT_TRUE passes | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_2_setProperty) 
{
    Property property = Property::Y;
    PropertyValue value;
    std::optional<PropertyValue> initialvalue;
	bool aidl_return = false;
    for (const auto& planeResourceIndex : video_and_graphic_planes) 
    {
        Status status = service->getProperty(planeResourceIndex, property, &initialvalue);
        ASSERT_TRUE(status.isOk());

        value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(200);
        status = service->setProperty(planeResourceIndex, property, value, &aidl_return);
        ASSERT_TRUE(status.isOk());

        //clean up
        status = service->setProperty(planeResourceIndex, property, initialvalue.value(), &aidl_return);
        ASSERT_TRUE(status.isOk());  
    }
}

/**
* @brief Test the setting of the property WIDTH for a plane control service
*
* This test verifies the functionality of setting the WIDTH property for different plane IDs and ensures it can be set and reverted successfully.
@n
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 034@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Initialize variables | property = Property::WIDTH, value = {}, initialvalue = {} | Variables should be initialized | Should be successful |
* | 02| Iterate over each plane in planes from profile| plane object contains { id, maxWidth } | Iteration starts | Should be successful |
* | 03| Get the initial property value |  _plane.id = valid id from profile, property = Property::WIDTH, initialvalue = {} | status.isOk() should be true | Should be successful |
* | 05| Set the width property to maxWidth |  _plane.id = valid id from profile, property = Property::WIDTH, value = _plane.maxWidth | status.isOk() should be true | Should be successful |
* | 07| Revert the property to its initial value | _plane.id = valid id from profile, property = Property::WIDTH, initialvalue = {initial value} | status.isOk() should be true | Should be successful |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_3_setProperty) 
{
    Property property = Property::WIDTH;
    PropertyValue value;
    std::optional<PropertyValue> initialvalue;
	bool aidl_return = false;
   
    for (const auto& _plane : planes)      
    {
        Status status = service->getProperty(_plane.id, property, &initialvalue);
        ASSERT_TRUE(status.isOk());

        //value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(1280);
        value.value = _plane.maxWidth;
        status = service->setProperty(_plane.id, property, value, &aidl_return);
        ASSERT_TRUE(status.isOk());

        //clean up
        status = service->setProperty(_plane.id, property, initialvalue.value(), &aidl_return);
        ASSERT_TRUE(status.isOk()); 
    }
}

/**
* @brief Tests the setProperty functionality of the L1 Plane Control Service.
*
* This test verifies the ability of the L1 Plane Control Service to correctly set and restore the property value for the plane control. The test ensures that the service can successfully set a property value for each plane and then revert it back to the initial value.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 035@n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Retrieve initial property value of the plane | _plane.id = plane index from profile, property = Property::HEIGHT, &initialvalue = valid pointer | status.isOk() should be true | Should be successful |
* | 02 | Set the property value of the plane to maxHeight | _plane.id = plane index from profile, property = Property::HEIGHT, value = maxHeight from profile | status.isOk() should be true | Should Pass |
* | 03 | Restore the initial property value of the plane | _plane.id = plane index from profile, property = Property::HEIGHT, value = initialvalue | status.isOk() should be true | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_4_setProperty) 
{
    PropertyValue value;
    std::optional<PropertyValue> initialvalue;
    Property property = Property::HEIGHT;
	bool aidl_return = false;
    for (const auto& _plane : planes) 
    {
        Status status = service->getProperty(_plane.id, property, &initialvalue);
        ASSERT_TRUE(status.isOk());

        value.value = _plane.maxHeight;
        status = service->setProperty(_plane.id, property, value, &aidl_return);
        ASSERT_TRUE(status.isOk());

        //clean up
        status = service->setProperty(_plane.id, property, initialvalue.value(), &aidl_return);
        ASSERT_TRUE(status.isOk()); 
    }
}


/**
* @brief Test the positive setting of the ALPHA Property of planes that support ALPHA.
*
* This test checks if the service can correctly set and reset the ALPHA property of each plane
* which supports the ALPHA property. It ensures that the plane's ALPHA property is set, verified and cleaned up correctly.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 036@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Get the initial ALPHA property value of the plane | _plane.id = valid plane index from profile, property = Property::ALPHA, output initialvalue | status.isOk() should be true | Should be successful |
* | 02| Set the ALPHA property of the plane to 128 | _plane.id = valid plane index from profile, property = Property::ALPHA, value = 128 | status.isOk() should be true | Should Pass |
* | 03| Clean up: Reset the ALPHA property to its initial value | _plane.id = valid plane index from profile, property = Property::ALPHA, value = initialvalue | status.isOk() should be true | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_5_setProperty) 
{
    std::optional<PropertyValue> initialvalue;
    PropertyValue value;
    Property property = Property::ALPHA;
	bool aidl_return = false;
    for (const auto& _plane : planes) 
    {
        if(_plane.supportsAlpha)
        {
            Status status = service->getProperty(_plane.id, property, &initialvalue);
            ASSERT_TRUE(status.isOk());

            value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(128);
            status = service->setProperty(_plane.id, property, value, &aidl_return);
            ASSERT_TRUE(status.isOk());

            //clean up
            status = service->setProperty(_plane.id, property, initialvalue.value(), &aidl_return);
            ASSERT_TRUE(status.isOk()); 
        }
        else
        {
            std::cout <<"plane Index : " <<_plane.id <<" doesnot support ALPHA property";
        }
    }
}

/**
* @brief Test the setProperty functionality for negative scenario with invalid index
*
* This test checks the behavior of the setProperty function when provided an invalid planeResourceIndex
*@n
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 037@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Set property with invalid planeResourceIndex | planeResourceIndex = -1, property = Property::X, value.value = 100 | status.exceptionCode() = EX_ILLEGAL_ARGUMENT | Should Fail |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_1_setProperty) 
{
    Property property = Property::X;
    int planeResourceIndex = -1;
    PropertyValue value;
	bool aidl_return = false;
    value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(100);
    Status status = service->setProperty(planeResourceIndex, property, value, &aidl_return);
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

/**
* @brief Test to verify behavior of setProperty when an invalid property ID is set
*
* This test checks if the setProperty function handles invalid property IDs correctly by returning an illegal argument exception.
* @n
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 038 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None
* **Dependencies:** None
* **User Interaction:** None
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* |------------------|-------------|-----------|------------------|-------|
* | 01               | Create PropertyValue object and set its intValue to 100 | value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(100) | Value object created with intValue 100 | Should be successful |
* | 02               | Iterate through each planeResourceIndex in video_and_graphic_planes | planeResourceIndex from video_and_graphic_planes | Iteration setup | Should be successful |
* | 03               | Call setProperty with planeResourceIndex, invalid property (-1) and value | planeResourceIndex = valid, Property = -1, value = 100 | Should return status with exceptionCode EX_ILLEGAL_ARGUMENT | Should fail |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_2_setProperty) 
{
    PropertyValue value;
	bool aidl_return = false;
    value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(100);
    for (const auto& planeResourceIndex : video_and_graphic_planes) 
    {
        Status status = service->setProperty(planeResourceIndex, static_cast<Property>(-1), value, &aidl_return);
        ASSERT_FALSE(status.isOk());
        ASSERT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }
}

/**
* @brief Test the `setProperty` function with an invalid value type 'bool', expecting an illegal argument exception.
*
* This test checks the `setProperty` function in a negative scenario by setting the property `WIDTH` to an invalid type `bool` which should trigger an illegal argument exception.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 039@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Get the `Property` value `WIDTH` | property = Property::WIDTH | Initialized property to WIDTH | Should be successful |
* | 02 | Initialize the `PropertyValue` object | value = true | Initialized value to boolean true | Should be successful |
* | 03 | Loop through each plane and set invalid property value `true` | value.value = true | status.exceptionCode() == android::binder::Status::EX_ILLEGAL_ARGUMENT | Should fail |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_3_setProperty) 
{
    Property property = Property::WIDTH;
    PropertyValue value;
	bool aidl_return = false;
    value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::booleanValue>(true);
    for (const auto& _plane : planes) 
    {
        Status status = service->setProperty(_plane.id, property, value, &aidl_return);
        ASSERT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }
}

/**
* @brief Tests setting a property for a given plane to a new value and then resetting it to its initial value.
*
* Validates whether the OVERSCAN property of a plane can be set to a new value and then reverted back to the original value successfully.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 040@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Verify getting the initial property value for each plane | planeResourceIndex = video plane index from profile, property = Property::OVERSCAN | status.isOk() should be true | Should be successful |
* | 02| Set the property value to 5 for each plane | planeResourceIndex = video plane index from profile, property = Property::OVERSCAN, value = 5 | status.isOk() should be true | Should Pass |
* | 03| Cleanup by resetting the property to its initial value for each plane | planeResourceIndex = video plane index from profile, property = Property::OVERSCAN, initialvalue | status.isOk() should be true | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_6_setProperty) 
{
    Property property = Property::OVERSCAN;
    PropertyValue value;
    std::optional<PropertyValue> initialvalue;
	bool aidl_return = false;
    for (const auto& planeResourceIndex : videoplanes) 
    {
        Status status = service->getProperty(planeResourceIndex, property, &initialvalue);
        ASSERT_TRUE(status.isOk());

        value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(5);
        status = service->setProperty(planeResourceIndex, property, value, &aidl_return);
        ASSERT_TRUE(status.isOk());

        //clean up
        status = service->setProperty(planeResourceIndex, property, initialvalue.value(), &aidl_return);
        ASSERT_TRUE(status.isOk());  
    }
}

/**
* @brief Test L1 plane control service for setting a property with invalid arguments.
*
* This test checks the behavior of the setProperty method when attempting to set an invalid property value on a graphics plane. The test aims to ensure that the service correctly handles illegal argument cases.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 041 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description                                                    | Test Data                                                                           | Expected Result                                      | Notes                         |
* | :--------------: | -------------------------------------------------------------- | ----------------------------------------------------------------------------------- | --------------------------------------------------- | ----------------------------- |
* | 01               | Initialize PropertyValue and Property with invalid data.       | property = Property::OVERSCAN, value = 5                                           | Should be successful                                 | Should be successful          |
* | 02               | Check if graphics planes are available.                        | graphicplanes.empty()                                                              | Should skip if no graphics plane available           | Should be successful          |
* | 03               | Attempt to set property on the first available graphics plane. | graphicplanes[0], property = Property::OVERSCAN, value = 5                         | status.exceptionCode() == android::binder::Status::EX_ILLEGAL_ARGUMENT | Should fail |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_4_setProperty) 
{
	bool aidl_return = false;
    PropertyValue value;
    Property property = Property::OVERSCAN;
    value.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(5);

    if(graphicplanes.empty())
    {
        GTEST_SKIP() << "Skipping the test due to no graphics plane available in the profile";
    }
    else
    {
        Status status = service->setProperty(graphicplanes[0], property, value, &aidl_return);
        ASSERT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }
}

/**
* @brief Test case for validating getting multiple plane control properties in a positive scenario
*
* This test validates the retrieval of multiple plane control properties for video and graphic planes.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 042@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Initialize PropertyKVPair for Property X | property = Property::X, propertyValue = None | PropertyKVPair initialized with property X and its default value | Should be successful |
* | 02 | Initialize PropertyKVPair for Property Y | property = Property::Y, propertyValue = None | PropertyKVPair initialized with property Y and its default value | Should be successful |
* | 03 | Setup vector with PropertyKVPair list | propertyKVList = {pairX, pairY} | Vector initialized with PropertyKVPair list | Should be successful |
* | 04 | Call getPropertyMulti for each planeResourceIndex from profile | planeResourceIndex, propertyKVList |status.isOk() should be true | Should Pass |
* | 06 | Validate propertyKVList size | propertyKVList.size() = 2u | Vector size is 2 | Should Pass |
* | 07 | Validate Property X | propertyKVList[0], Property::X | Expected value for Property X is set | Should Pass |
* | 08 | Validate Property Y | propertyKVList[1], Property::Y | Expected value for Property Y is set | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_1_getPropertyMulti) 
{
	bool aidl_return = false;
    com::rdk::hal::planecontrol::PropertyKVPair pairX;
    pairX.property = com::rdk::hal::planecontrol::Property::X;
    pairX.propertyValue = com::rdk::hal::PropertyValue{};

    com::rdk::hal::planecontrol::PropertyKVPair pairY;
    pairY.property = com::rdk::hal::planecontrol::Property::Y;
    pairY.propertyValue = com::rdk::hal::PropertyValue{};

    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList = {pairX, pairY};
    for (const auto& planeResourceIndex : video_and_graphic_planes) 
    {
        ::android::binder::Status status = service->getPropertyMulti(planeResourceIndex, &propertyKVList, &aidl_return);
        // Validate
        EXPECT_TRUE(status.isOk());
        EXPECT_EQ(propertyKVList.size(), 2u);

        EXPECT_TRUE(populateExpectedPropertyValue(propertyKVList[0], com::rdk::hal::planecontrol::Property::X));
        EXPECT_TRUE(populateExpectedPropertyValue(propertyKVList[1], com::rdk::hal::planecontrol::Property::Y)); // Example value
    }
}

/**
* @brief Test the functionality of retrieving multiple properties with invalid plane resource index.
*
* This test aims to verify the behavior of the getPropertyMulti function when called with an invalid plane resource index. It checks if the function handles the invalid input appropriately and returns the expected error code.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 043@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Initialize plane resource index to an invalid value | planeResourceIndex = -1 | Should be successful | |
* | 02 | Prepare PropertyKVPair for property X | pairX.property = Property::X, pairX.propertyValue = PropertyValue{} | Should be successful | |
* | 03 | Prepare PropertyKVPair for property Y | pairY.property = Property::Y, pairY.propertyValue = PropertyValue{} | Should be successful | |
* | 04 | Combine properties into a list | propertyKVList = {pairX, pairY} | Should be successful | |
* | 05 | Call getPropertyMulti with invalid plane resource index | planeResourceIndex = -1, propertyKVList | status.exceptionCode() == EX_ILLEGAL_ARGUMENT | should fail |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_1_getPropertyMulti) 
{
	bool aidl_return = false;
    int32_t planeResourceIndex = -1;

    com::rdk::hal::planecontrol::PropertyKVPair pairX;
    pairX.property = com::rdk::hal::planecontrol::Property::X;
    pairX.propertyValue = com::rdk::hal::PropertyValue{};

    com::rdk::hal::planecontrol::PropertyKVPair pairY;
    pairY.property = com::rdk::hal::planecontrol::Property::Y;
    pairY.propertyValue = com::rdk::hal::PropertyValue{};

    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList = {pairX, pairY};
    ::android::binder::Status status = service->getPropertyMulti(planeResourceIndex, &propertyKVList, &aidl_return);
    EXPECT_EQ(status.exceptionCode(), ::android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

// METRIC_UNDERFLOWED is removed in 3b32784f33bf8140bfd3dcf3e7239013583c5566 in hal.git
#if 0
/**
* @brief This test validates the positive scenario for the `getPropertyMulti` method of the `L1PlaneControlService`.
*
* This test examines if the `getPropertyMulti` method correctly retrieves multiple properties for video plane resources and returns an OK status.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 044@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Initialize property key-value pair and add it to a list | Property = METRIC_UNDERFLOWED, PropertyValue = default | Property key-value pair initialized and added to list | Should be successful |
* | 02 | Iterate through each video plane resource index from profile and call getPropertyMulti method | planeResourceIndex list, propertyKVList | status.isOk() should be true | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_2_getPropertyMulti) 
{
	bool aidl_return = false;
    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
    com::rdk::hal::planecontrol::PropertyKVPair kvPair;
    kvPair.property = com::rdk::hal::planecontrol::Property::METRIC_UNDERFLOWED;
    kvPair.propertyValue = {}; // Or initialize with appropriate default value

    propertyKVList.push_back(kvPair);
    for (const auto& planeResourceIndex : videoplanes) 
    {
        ::android::binder::Status status = service->getPropertyMulti(planeResourceIndex, &propertyKVList, &aidl_return);
        EXPECT_TRUE(status.isOk());
    }
}
#endif

// METRIC_FRAMES_RECEIVED and METRIC_FRAMES_PRESENTED are removed in 3b32784f33bf8140bfd3dcf3e7239013583c5566 in hal.git
#if 0
/**
* @brief This test validates the `getPropertyMulti` function with valid property keys.
*
* The objective is to test the `getPropertyMulti` function using valid property keys `METRIC_FRAMES_RECEIVED` and `METRIC_FRAMES_PRESENTED`, ensuring that the status received after invoking the function is OK.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 045@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description                                                        | Test Data                                   | Expected Result | Notes                           |
* | :--------------: | ------------------------------------------------------------------ | ------------------------------------------ | --------------- | ------------------------------- |
* | 01               | Create a property key-value pair with `METRIC_FRAMES_RECEIVED`    | kvPair1.property = METRIC_FRAMES_RECEIVED  | Should be successful | Should be successful |
* | 02               | Set property value to an empty object for `kvPair1`               | kvPair1.propertyValue = {}                | Should be successful | Should be successful |
* | 03               | Push `kvPair1` into `propertyKVList`                              | propertyKVList.push_back(kvPair1)          | Should be successful | Should be successful |
* | 04               | Create a property key-value pair with `METRIC_FRAMES_PRESENTED`   | kvPair2.property = METRIC_FRAMES_PRESENTED | Should be successful | Should be successful |
* | 05               | Set property value to an empty object for `kvPair2`               | kvPair2.propertyValue = {}                 | Should be successful | Should be successful |
* | 06               | Push `kvPair2` into `propertyKVList`                              | propertyKVList.push_back(kvPair2)          | Should be successful | Should be successful |
* | 07               | Loop over `video_and_graphic_planes` and invoke `getPropertyMulti`| planeResourceIndex, &propertyKVList = valid pointer | status.isOk() should be true | Should Pass |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_3_getPropertyMulti) 
{
    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;

    com::rdk::hal::planecontrol::PropertyKVPair kvPair1;
    kvPair1.property = com::rdk::hal::planecontrol::Property::METRIC_FRAMES_RECEIVED;
    kvPair1.propertyValue = {}; 
    propertyKVList.push_back(kvPair1);

    com::rdk::hal::planecontrol::PropertyKVPair kvPair2;
    kvPair2.property = com::rdk::hal::planecontrol::Property::METRIC_FRAMES_PRESENTED;
    kvPair2.propertyValue = {}; 
    propertyKVList.push_back(kvPair2);

    for (const auto& planeResourceIndex : video_and_graphic_planes) 
    {
        ::android::binder::Status status = service->getPropertyMulti(planeResourceIndex, &propertyKVList);
        EXPECT_TRUE(status.isOk());
    }
}
#endif

/**
* @brief This test verifies the functionality of setting multiple atomic properties for a plane control.
*
* This test is designed to set multiple properties for a plane control, including X, Y, Width, and Height. 
* It ensures that the `setPropertyMultiAtomic` function works correctly by comparing the expected and actual operation status@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 046@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Retreiving initail value of properties using getPropertyMulti | None | status.isOk() should be true | done by Pre-requisite SETUP function |
* | 02 | Set up initial properties by invoking the SetUp method | initial propertyKVList with empty values | Should succeed | Done by Pre-requisite SetUp function |
* | 03 | Create the propertyKVPair1 and set property X to 100 and insert to the propertyKVList | X = 100 | Should succeed | Should be successful |
* | 04 | Create the propertyKVPair2 with property Y set to 200 and insert to the propertyKVList | Y = 200 | Should succeed | Should be successful |
* | 05 | Create the propertyKVPair3 with WIDTH to max width from profile and insert to the propertyKVList | WIDTH = _maxwidth | Should succeed | Should be successful |
* | 06 | Create the propertyKVPair4 with HEIGHT to max height from profile and insert to the propertyKVList | HEIGHT = _maxheight | Should succeed | Should be successful |
* | 07 | Invoke setPropertyMultiAtomic with valid planeResourceIndex and propertyKVList | planeResourceIndex = valid, propertyKVList = valid | status.isOk() should be true | Should Pass |
* | 08 | Clean up by invoking TearDown method to reset properties using setPropertyMultiAtomic | initialpropertyKVList | status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1PlaneControlPropertyTest, test_l1_planecontrol_positive_1_setPropertyMultiAtomic) 
{
    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;

    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair1;
    propertyKVPair1.property = com::rdk::hal::planecontrol::Property::X;
    propertyKVPair1.propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(100);
    propertyKVList.push_back(propertyKVPair1);

    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair2;
    propertyKVPair2.property = com::rdk::hal::planecontrol::Property::Y;
    propertyKVPair2.propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(200);
    propertyKVList.push_back(propertyKVPair2);

    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair3;
    propertyKVPair3.property = com::rdk::hal::planecontrol::Property::WIDTH;
    // propertyKVPair3.propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(800);
    propertyKVPair3.propertyValue.value =  _maxwidth;   
    propertyKVList.push_back(propertyKVPair3);

    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair4;
    propertyKVPair4.property = com::rdk::hal::planecontrol::Property::HEIGHT;
    //propertyKVPair4.propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(600);
    propertyKVPair4.propertyValue.value = _maxheight;
    propertyKVList.push_back(propertyKVPair4);

	bool aidl_return = false;
    android::binder::Status status = service->setPropertyMultiAtomic(_planeResourceIndex, propertyKVList, &aidl_return);
    EXPECT_TRUE(status.isOk());
}

/**
* @brief Test the negative scenario of setPropertyMultiAtomic with missing property values.
*
* This test checks the behavior of setPropertyMultiAtomic when property value is not provided (nullopt).
* The test iterates over the planeResourceIndices and invokes setPropertyMultiAtomic with incomplete propertyKVPair.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 047@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :--------------: | ----------- | --------- | --------------- | ----- |
* | 01               | Create an empty vector of PropertyKVPair | propertyKVList = [] | Should be successful |  |
* | 02               | Initialize PropertyKVPair with property WIDTH and no value | propertyKVPair.property = WIDTH, propertyKVPair.value = nullopt | Should be successful |  |
* | 03               | Add PropertyKVPair to propertyKVList | propertyKVList.push_back(propertyKVPair) |Should be successful |  |
* | 04               | Iterate over the planeResourceIndices from profile irrespective of video/graphic plane and call setPropertyMultiAtomic with the property list | input: planeResourceIndex, propertyKVList | status.exceptionCode() == android::binder::Status::EX_ILLEGAL_ARGUMENT | Should Fail |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_1_setPropertyMultiAtomic) 
{
	bool aidl_return = false;
    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair;
    propertyKVPair.property = com::rdk::hal::planecontrol::Property::WIDTH;
    propertyKVList.push_back(propertyKVPair); // No property value defined (nullopt)

    for (const auto& planeResourceIndex : video_and_graphic_planes) 
    {
        android::binder::Status status = service->setPropertyMultiAtomic(planeResourceIndex, propertyKVList, &aidl_return);
        EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
    }
}

/**
* @brief This test verifies the positive scenario of setting multiple plane control properties atomically.
*
* This test ensures that multiple plane control properties can be set atomically using the `setPropertyMultiAtomic` API.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 048@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Get initial properties using getPropertyMulti API | None | status.isOk() should be true |Done by Pre-requisite SetUp function |
* | 02| Prepare property key-value list with VISIBLE property set to 1 | property = VISIBLE, propertyValue = 1 | Should be successful | Should be successful |
* | 03| Invoke setPropertyMultiAtomic API | planeResourceIndex = valid index from profile, propertyKVList = valid list | status.isOk() should be true | Should Pass |
* | 04| Restore the initial property key-value list | None | Should be successful |Done by Pre-requisite TearDown function |
*/
TEST_F(L1PlaneControlPropertyTest, test_l1_planecontrol_positive_2_setPropertyMultiAtomic) 
{
	bool aidl_return  = false;
    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair;
    propertyKVPair.property = com::rdk::hal::planecontrol::Property::VISIBLE;
    propertyKVPair.propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(1);
    propertyKVList.push_back(propertyKVPair);

    android::binder::Status status = service->setPropertyMultiAtomic(_planeResourceIndex, propertyKVList, &aidl_return);
    EXPECT_TRUE(status.isOk());
}

/**
* @brief Test setPropertyMultiAtomic with invalid plane resource index (-1)
*
* This test checks the behavior of the setPropertyMultiAtomic function when
* called with an invalid plane resource index (-1), which is expected to
* fail as an illegal argument.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 049@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Initialize planeResourceIndex to -1 and create propertyKVList with one propertyKVPair. | planeResourceIndex = -1, property = WIDTH, propertyValue = 800 | | Should be successful |
* | 02| Call setPropertyMultiAtomic with planeResourceIndex and propertyKVList | planeResourceIndex = -1, propertyKVList = valid | status.exceptionCode() == android::binder::Status::EX_ILLEGAL_ARGUMENT | Should Fail |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_2_setPropertyMultiAtomic) 
{
	bool aidl_return = false;
    int32_t planeResourceIndex = -1;
    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair;
    propertyKVPair.property = com::rdk::hal::planecontrol::Property::WIDTH;
    propertyKVPair.propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(800);
    propertyKVList.push_back(propertyKVPair);

    android::binder::Status status = service->setPropertyMultiAtomic(planeResourceIndex, propertyKVList, &aidl_return);
    EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

/**
* @brief This test verifies that the setPropertyMultiAtomic function correctly handles an invalid property key.
*
* This test is designed to check the response of the setPropertyMultiAtomic function when it is given an invalid property key (9999), which does not exist. This will help us ensure that the function handles invalid input arguments properly by throwing the appropriate exception, in this case, EX_ILLEGAL_ARGUMENT.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 050@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Create an instance of PropertyKVPair and set an invalid property key (9999). Set property value to 800. | propertyKVPair.property = 9999, propertyKVPair.propertyValue.value = 800 | PropertyKVPair created with invalid key and value 800 | Should be successful |
* | 02 | Add the PropertyKVPair to propertyKVList. | propertyKVPair = invalid  | PropertyKVPair added to the list | Should be successful |
* | 03 | Call setPropertyMultiAtomic with valid plane index and invalid propertyKVList as arguments. | video_and_graphic_planes[0], propertyKVList = invalid | status.exceptionCode() == android::binder::Status::EX_ILLEGAL_ARGUMENT | Should fail |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_3_setPropertyMultiAtomic) 
{
	bool aidl_return = false;
    std::vector<com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;
    com::rdk::hal::planecontrol::PropertyKVPair propertyKVPair;

    propertyKVPair.property = static_cast<com::rdk::hal::planecontrol::Property>(9999); // Invalid property key
    propertyKVPair.propertyValue.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::Tag::intValue>(800);
    propertyKVList.push_back(propertyKVPair);

    android::binder::Status status = service->setPropertyMultiAtomic(video_and_graphic_planes[0], propertyKVList, &aidl_return);
    EXPECT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);    
}

/**
* @brief Tests the registration of a valid listener to the Plane Control Service.
*
* This test checks if a valid Plane Control Listener can be successfully registered to the Plane Control Service.
* @n
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 051 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Create a valid Plane Control Listener object | N/A | Listener object should be created successfully | Should be successful |
* | 02| Check if the listener object is not nullptr | listener=A valid listener object | Listener object should not be nullptr | Should be successful |
* | 03| Register the listener with the Plane Control Service | listener, _aidl_return | Status.isok() should be true, _aidl_return should be true | Should Pass |
* | 04| Unregister the listener from the Plane Control Service for cleanup | listener, _aidl_return | Status.isok() should be true, _aidl_return should be true | Should be successful, Done by Pre-requisite TearDown function |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_positive_1_registerListener) 
{
    ::android::sp<IPlaneControlListener> listener = new ValidPlaneControlListener();
    bool _aidl_return;
    ASSERT_NE(listener, nullptr);
    Status status = service->registerListener(listener, &_aidl_return);
    // Verify the listener is registered successfully
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(_aidl_return);   
    //cleanup
    status = service->unregisterListener(listener,&_aidl_return);
}

/**
* @brief Test to verify duplicate registration of plane control listener
*
* This test ensures that an attempt to register the same listener twice will fail.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 052@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Call SetUp() in the Pre-requisite class and register the listener | listener = valid listener, &_aidl_return | status.isOk(), _aidl_return = true | Done by Pre-requisite SetUp function |
* | 02 | Attempt to register the same listener again using registerListener API | listener = previously registered listener | status.isOk(), _aidl_return = false | Should fail |
* | 03 | Call unregisterListener to clean up | listener = valid listener ( registered from the SETUP) | status.isOk(), _aidl_return should be true | Should be successful |
*/
TEST_F(L1PlaneControlUnregisterTest, test_l1_planecontrol_negative_1_registerListener) 
{
    bool _aidl_return;
    // Attempt to register the same listener again
    Status status = service->registerListener(listener, &_aidl_return);
    // Verify the registration fails the second time
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(_aidl_return);
    //cleanup
    status = service->unregisterListener(listener,&_aidl_return);
}

/**
* @brief Test for unregistering a valid listener for plane control
*
* This test checks the unregisterListener functionality. It ensures that a previously registered listener 
* is successfully unregistered by verifying the status and return values after the operation.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 053@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description                                    | Test Data                                | Expected Result                 | Notes                              |
* | :--------------: | ---------------------------------------------- | ---------------------------------------- | -----------------------------   | -----------------------------------|
* | 01               | Call SetUp to register the listener            | listener = new ValidPlaneControlListener | Registration successful         | Done by Pre-requisite SetUp function|
* | 02               | Invoke unregisterListener with the same listener | listener = valid registered listener   | status.isOk(), aidl_return = true            | Should Pass            |
*/
TEST_F(L1PlaneControlUnregisterTest, test_l1_planecontrol_positive_1_unregisterListener) 
{
    bool aidl_return = false;
    ::android::binder::Status status = service->unregisterListener(listener, &aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(aidl_return);
}

/**
* @brief Test the unregisterListener function in negative scenarios
*
* Verifies the behavior of the unregisterListener function when the specified listener has not been previously registered.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 054@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01    | Create a valid IPlaneControlListener object | listener = new ValidPlaneControlListener() | Listener object created successfully | Should be successful |
* | 02    | Call unregisterListener with the created listener and capture the return value and status | input: listener = ValidPlaneControlListener instance, output: aidl_return = false | status.isOk() should be true, aidl_return should be false | Should fail |
*/
TEST(L1PlaneControlServiceTest, test_l1_planecontrol_negative_1_unregisterListener) 
{
    bool aidl_return = false;
    ::android::sp<::com::rdk::hal::planecontrol::IPlaneControlListener> listener = new ValidPlaneControlListener();

    ::android::binder::Status status = service->unregisterListener(listener, &aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(aidl_return);
}


std::vector<std::string> splitPath(const std::string& path, char delimiter) {
        std::vector<std::string> segments;
        size_t start = 0;
        size_t end = path.find(delimiter);

        while (end != std::string::npos) {
                segments.push_back(path.substr(start, end - start));
                start = end + 1;
                end = path.find(delimiter, start);
        }

        segments.push_back(path.substr(start));
        return segments;
}


int createComponentMap( ut_kvp_instance_t * p_instance, std::map<std::string, int> &l_compMap){

        char result_kvp[UT_KVP_MAX_ELEMENT_SIZE];
        std::string key;
        int total_componentes = ut_kvp_getListCount(p_instance ,"components");
        if(total_componentes == 0){
                std::cerr << "components node not found or not a list." << std::endl;
                return -1;
        }
        for(int i = 0; i < total_componentes;i++){
                key.clear();
                key = "components/"+ std::to_string(i) + "/name";
                if(0 == ut_kvp_getStringField(p_instance, key.c_str(), result_kvp, UT_KVP_MAX_ELEMENT_SIZE)){
                        l_compMap.insert({std::string(result_kvp),i});
                }
        }
        return 0;
}

int updatedPath(std::string &path, const std::map<std::string, int> &l_compMap){

        std::vector<std::string> segments = splitPath(path, '.');
        // Check if we have enough segments to perform the update
        if (segments.size() < 2) {
                std::cerr << "Error: Path has insufficient segments for update." << std::endl;
                return -1;
        }
        // Check if the componentMap contains the second segment
        auto it = l_compMap.find(segments[1]);
        if (it == l_compMap.end()) {
                std::cerr << "Error: Component " << segments[1] << " not found in component map." << std::endl;
                return -1;
        }

        std::string rtn;
        segments[1] = std::to_string(l_compMap.at(segments[1]));
        rtn += segments[0];
        for(int j = 1;j< segments.size();j++){
                rtn += "/"+ segments[j];
        }
        path.clear();
        path = rtn;
        return 0;
}

int main(int argc, char **argv) 
{
    Plane temporary;
    int result1, result2;
    int number_of_planes;
    int pixelformatsCount;
    int Dynamicrangescount;
    std::map<std::string , int> componentMap;
    char result_kvp[UT_KVP_MAX_ELEMENT_SIZE];
    std::string planeControlpath = "components.IPlaneControl.planes";
    std::string videosink_count_path = "components.IVideoSinkManager.IVideoSink";

    android::ProcessState::self()->startThreadPool();

    UT_init(argc, argv);
    ut_kvp_instance_t *config = ut_kvp_profile_getInstance();

    if (config != nullptr) 
    {
        
        if(-1 == createComponentMap(config, componentMap)){
        	return -1;
        }
        if(-1 == updatedPath(planeControlpath, componentMap)){
        	std::cerr << "Error: yaml not have videodecoder config\n";
         	return -1;
        }
            updatedPath(videosink_count_path, componentMap);
    	if(getService() == -1)
    	{
        	return -1;
    	}


	number_of_planes = ut_kvp_getListCount(config,planeControlpath.c_str());
        std::cout << "number of planes available: " << number_of_planes<<std::endl;
        for(int i = 0; i < number_of_planes; i++)
        {
            std::string planeindexpath = planeControlpath+"/"+std::to_string(i) + "/id";
            std::string planetypepath = planeControlpath+"/"+std::to_string(i) + "/type";
            std::string planemaxwidthpath = planeControlpath+"/"+std::to_string(i) + "/maxWidth";
            std::string planemaxHeightpath = planeControlpath+"/"+std::to_string(i) + "/maxHeight";
            std::string planeframeWidthpath = planeControlpath+"/"+std::to_string(i) + "/frameWidth";
            std::string planeframeHeightpath = planeControlpath+"/"+std::to_string(i) + "/frameHeight";
            std::string planecolorDepthpath = planeControlpath+"/"+std::to_string(i) + "/colorDepth";
            std::string planemaxFrameRatepath = planeControlpath+"/"+std::to_string(i) + "/maxFrameRate";
            std::string planesupportsAlphapath = planeControlpath+"/"+std::to_string(i) + "/supportsAlpha";
            std::string planesupportsZOrderpath = planeControlpath+"/"+std::to_string(i) + "/supportsZOrder";
            std::string planevsyncDisplayLatencypath = planeControlpath+"/"+std::to_string(i) + "/vsyncDisplayLatency";
            std::string planepixelformatcountpath = planeControlpath+"/"+std::to_string(i) + "/pixelFormats";
            std::string planesupportedDynamicRangescountpath = planeControlpath+"/"+std::to_string(i) + "/supportedDynamicRanges";

            temporary.id = ut_kvp_getUInt32Field(config,planeindexpath.c_str());
            temporary.type =  (0 == ut_kvp_getStringField(config,planetypepath.c_str(), result_kvp, UT_KVP_MAX_ELEMENT_SIZE)) ? std::string(result_kvp) : "";
            video_and_graphic_planes.push_back(temporary.id);
            if(temporary.type == "VIDEO")
            {
                videoplanes.push_back(temporary.id);
            }
            if(temporary.type == "GRAPHICS")
            {
                graphicplanes.push_back(temporary.id);
            }
            temporary.maxWidth = ut_kvp_getUInt32Field(config,planemaxwidthpath.c_str());
            temporary.maxHeight = ut_kvp_getUInt32Field(config,planemaxHeightpath.c_str());
            temporary.frameWidth = ut_kvp_getUInt32Field(config,planeframeWidthpath.c_str());
            temporary.frameHeight = ut_kvp_getUInt32Field(config, planeframeHeightpath.c_str());
            temporary.colorDepth = ut_kvp_getUInt32Field(config, planecolorDepthpath.c_str());
            temporary.maxFrameRate = ut_kvp_getUInt32Field(config, planemaxFrameRatepath.c_str());
            temporary.supportsAlpha = ut_kvp_getBoolField(config, planesupportsAlphapath.c_str());
            temporary.supportsZOrder = ut_kvp_getBoolField(config, planesupportsZOrderpath.c_str());
            temporary.vsyncDisplayLatency = ut_kvp_getUInt32Field(config, planevsyncDisplayLatencypath.c_str());

            pixelformatsCount = ut_kvp_getListCount(config,planepixelformatcountpath.c_str());
            for(int j=0 ; j<pixelformatsCount ; j++)
            {
                std::string planepixelformatpath = planepixelformatcountpath+"/"+std::to_string(j);
                std::string pixelformat =  (0 == ut_kvp_getStringField(config, planepixelformatpath.c_str(), result_kvp, UT_KVP_MAX_ELEMENT_SIZE)) ? std::string(result_kvp) : "";
                temporary.pixelFormats.push_back(pixelformat.c_str());
            }

            Dynamicrangescount = ut_kvp_getListCount(config,planesupportedDynamicRangescountpath.c_str());
            for(int k=0 ; k<Dynamicrangescount ; k++)
            {
                std::string planesupportedDynamicRangespath = planesupportedDynamicRangescountpath+"/"+std::to_string(k);
                std::string Dynamicrange =  (0 == ut_kvp_getStringField(config, planesupportedDynamicRangespath.c_str(), result_kvp, UT_KVP_MAX_ELEMENT_SIZE)) ? std::string(result_kvp) : "";
                temporary.supportedDynamicRanges.push_back(Dynamicrange.c_str());
            }

            planes.push_back(temporary);
            temporary.pixelFormats.clear();
            temporary.supportedDynamicRanges.clear();

         }

         videosink_count = ut_kvp_getListCount(config,videosink_count_path.c_str());
         std::cout<< "videosink_count is : " <<videosink_count <<std::endl;
         if(videosink_count > 0)
         {
            videosink_available_flag = true;
            for(int i=0; i<videosink_count; ++i)
            {
                std::string videosinkidpath = videosink_count_path+"/"+std::to_string(i) + "/id";
                videosink_id = ut_kvp_getUInt32Field(config,videosinkidpath.c_str());
                videosink_ids.push_back(videosink_id);
            }
         }
         if(videosink_count > 1)
         {
            multiple_videosink_source_flag = true;
         }

        //  for(int index: video_and_graphic_planes)
         for (const auto& _plane: planes)
         {
            _planeResourceIndex = _plane.id;
            _maxwidth = _plane.maxWidth;
            _maxheight = _plane.maxHeight;
            ::testing::GTEST_FLAG(filter) = "L1PlaneControlPropertyTest.*";
            result1 = UT_run_tests();
         }
         
        ::testing::GTEST_FLAG(filter) = "L1PlaneControlServiceTest.*:L1PlaneControlUnregisterTest.*";
        result2 = UT_run_tests();

        return (result1 || result2);
    }
     
    else
    {
        std::cout << "USAGE: " << argv[0] <<  "-p <config_file>" <<std::endl;
        return -1;
    }
}

