# IPLANECONTROL L2 Low Level Test Specification and Procedure Documentation
 
## Table of Contents

       - [IPLANECONTROL L2 Low Level Test Specification and Procedure Documentation](#iplanecontrol-l2-low-level-test-specification-and-procedure-documentation)

         - [Table of Contents](#table-of-contents)
         - [Overview](#overview)
           - [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
           - [Definitions](#definitions)
           - [References](#references)
         - [Level 2 Test Procedure](#level-2-test-procedure)
 
## Overview

       This document describes the level 2 testing suite for the IPLANECONTROL module.
 
### Acronyms, Terms and Abbreviations

       - `HAL` \- Hardware Abstraction Layer, may include some common components
       - `UT`  \- Unit Test(s)
       - `OEM`  \- Original Equipment Manufacture
       - `SoC`  \- System on a Chip
 
### Definitions

         - `ut-core` \- Common Testing Framework <https://github.com/rdkcentral/ut-core>, which wraps a open-source framework that can be expanded to the requirements for future framework.
 
### References
- `High Level Test Specification` - [planecontrol_High-Level_TestSpecification.md](planecontrol_High-Level_TestSpecification.md)
 
## Level 2 Test Procedure

       The following functions are expecting to test the module operates correctly.
 
### Test 1

| Title | Details |
| -- | -- |
| Function Name | `test_l2_PlaneControlBaseTest_TestGetSetProperty` |
| Description | Get platform capabilities using `getCapabilities` to get list of supported plane indices. Loop through all the plane indices. Use the `setProperty` method to set all the supported value provided in configuration YAML for all plane resource indices. Get the property value using `getProperty` of `IPlaneControl` and verify the HAL returns the set Value from the YAML file. |
| Test Group | Module : 02 |
| Test Case ID | 001 |
| Priority | High |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Steps | Description | Test Data | Expected Result | Notes |
| -- | --------- | ---------- | -------------- | ----- |
| 01 | Get platform capabilities using `getCapabilities` | capabilities = valid buffer | Status is OK | Should be successful |
| 02 | Loop through all plane indices and set property WIDTH using `setProperty` | planeIndex = capability.planeIndex, property = WIDTH, propertyValue = frame_width | Status is OK, setResult is true | Should be successful |
| 03 | Get property WIDTH using `getProperty` and verify | planeIndex = capability.planeIndex, property = WIDTH | Status is OK, getPropertyValue matches set value | Should be successful |
| 04 | Set property HEIGHT using `setProperty` | planeIndex = capability.planeIndex, property = HEIGHT, propertyValue = frame_height | Status is OK, setResult is true | Should be successful |
| 05 | Get property HEIGHT using `getProperty` and verify | planeIndex = capability.planeIndex, property = HEIGHT | Status is OK, getPropertyValue matches set value | Should be successful |
| 06 | Set property X using `setProperty` | planeIndex = capability.planeIndex, property = X, propertyValue = 0 | Status is OK, setResult is true | Should be successful |
| 07 | Get property X using `getProperty` and verify | planeIndex = capability.planeIndex, property = X | Status is OK, getPropertyValue matches set value | Should be successful |
| 08 | Set property Y using `setProperty` | planeIndex = capability.planeIndex, property = Y, propertyValue = 0 | Status is OK, setResult is true | Should be successful |
| 09 | Get property Y using `getProperty` and verify | planeIndex = capability.planeIndex, property = Y | Status is OK, getPropertyValue matches set value | Should be successful |
| 10 | If supportsAlpha, set property ALPHA using `setProperty` | planeIndex = capability.planeIndex, property = ALPHA, propertyValue = 0 | Status is OK, setResult is true | Should be successful |
| 11 | If supportsAlpha, get property ALPHA using `getProperty` and verify | planeIndex = capability.planeIndex, property = ALPHA | Status is OK, getPropertyValue matches set value | Should be successful |
| 12 | If supportsZOrder, set property ZORDER using `setProperty` | planeIndex = capability.planeIndex, property = ZORDER, propertyValue = 1 | Status is OK, setResult is true | Should be successful |
| 13 | If supportsZOrder, get property ZORDER using `getProperty` and verify | planeIndex = capability.planeIndex, property = ZORDER | Status is OK, getPropertyValue matches set value | Should be successful |
| 14 | Set property VISIBLE using `setProperty` | planeIndex = capability.planeIndex, property = VISIBLE, propertyValue = 1 | Status is OK, setResult is true | Should be successful |
| 15 | Get property VISIBLE using `getProperty` and verify | planeIndex = capability.planeIndex, property = VISIBLE | Status is OK, getPropertyValue matches set value | Should be successful |
| 16 | If planeType is VIDEO, set property OVERSCAN using `setProperty` | planeIndex = capability.planeIndex, property = OVERSCAN, propertyValue = 0 | Status is OK, setResult is true | Should be successful |
| 17 | If planeType is VIDEO, get property OVERSCAN using `getProperty` and verify | planeIndex = capability.planeIndex, property = OVERSCAN | Status is OK, getPropertyValue matches set value | Should be successful |
| 18 | If planeType is VIDEO, set property ASPECT_RATIO using `setProperty` | planeIndex = capability.planeIndex, property = ASPECT_RATIO, propertyValue = 1 | Status is OK, setResult is true | Should be successful |
| 19 | If planeType is VIDEO, get property ASPECT_RATIO using `getProperty` and verify | planeIndex = capability.planeIndex, property = ASPECT_RATIO | Status is OK, getPropertyValue matches set value | Should be successful |

### Test 2

| Title | Details |
| -- | -- |
| Function Name | `test_l2_PlaneControlBaseTest_TestGetSetPropertyMultiAtomic` |
| Description | Get platform capabilities using `getCapabilities` to get list of supported plane indices. Loop through all the plane indices. Prepare Key value pair of the available properties. Use the `setPropertyMultiAtomic` method to set all the supported value provided in configuration YAML for all plane resource indices. Get the property value using `getPropertyMulti` of `IPlaneControl` and verify the HAL returns the set Value from the YAML file. |
| Test Group | Module : 02 |
| Test Case ID | 002 |
| Priority | High |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Steps | Description | Test Data | Expected Result | Notes |
| -- | --------- | ---------- | -------------- | ----- |
| 01 | Get platform capabilities using `getCapabilities` | capabilities = valid buffer | status.isOk() = true | Should be successful |
| 02 | Loop through all the plane indices and prepare key-value pairs for properties | planeIndex = capability.planeIndex, propertyKVList = list of PropertyKVPair | N/A | Should be successful |
| 03 | Set properties using `setPropertyMultiAtomic` for each plane index | planeIndex = capability.planeIndex, propertyKVList = list of PropertyKVPair | status.isOk() = true, setResult = true | Should be successful |
| 04 | Get properties using `getPropertyMulti` for each plane index and verify values | planeIndex = capability.planeIndex, getPropertyKVList = list of PropertyKVPair | status.isOk() = true, getResult = true | Should be successful |
| 05 | Verify each property value matches the expected value from YAML | property = Property::WIDTH, Property::HEIGHT, etc. | UT_ASSERT_EQUAL(value, expected_value) | Should be successful |

### Test 3

| Title | Details |
| -- | -- |
| Function Name | `test_l2_planecontrol_TestGetSetVideoSourceDestinationPlaneMapping` |
| Description | Get platform capabilities using `getCapabilities` to get list of supported plane indices. Loop through all of the video plane indices. Set a Video source to the Video plane using `setVideoSourceDestinationPlaneMapping`. Verify the video source and plane mapping using `getVideoSourceDestinationPlaneMapping`. |
| Test Group | Module : 02 |
| Test Case ID | 003 |
| Priority | High |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Steps | Description | Test Data | Expected Result | Notes |
| -- | --------- | ---------- | -------------- | ----- |
| 01 | Get platform capabilities using `getCapabilities` | capabilities = valid buffer | status.isOk() = true | Should be successful |
| 02 | Loop through capabilities to find video plane indices | capability.type = PlaneType::VIDEO | plane_index.size() >= 2 | Should be successful |
| 03 | Set video source to video plane using `setVideoSourceDestinationPlaneMapping` | sourcePlaneMappings = list of mappings, mappingResult = valid buffer | status.isOk() = true, mappingResult = true | Should be successful |
| 04 | Verify video source and plane mapping using `getVideoSourceDestinationPlaneMapping` | retrievedMappings = valid buffer | status.isOk() = true | Should be successful |
| 05 | Validate each mapping in retrievedMappings matches sourcePlaneMappings | mapping.destinationPlaneIndex = valid index, mapping.sourceType = valid type, mapping.sourceIndex = valid index | mapping found in retrievedMappings | Should be successful |

