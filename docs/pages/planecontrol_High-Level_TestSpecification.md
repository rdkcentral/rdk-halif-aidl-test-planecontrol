# Plane Control High Level Test Specification Document

## Table of Contents

- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Introduction](#introduction)
- [Module Description](#module-description)
- [L2 Test Specification](#l2-test-specification)
- [L3 Test Specification](#l3-test-specification)

## Acronyms, Terms and Abbreviations

- `HAL`    - Hardware Abstraction Layer
- `API`    - Application Programming Interface
- `AIDL`   - Android Interface Definition Language
- `L2`     - Level 2 Testing
- `L3`     - Level 3 Testing
- `NA`     - Not Applicable
- `Caller` - Any user of the interface via the `APIs`
- `CB`     - Call-back function (suffix)
- `RDK`    - Reference Design Kit

## Introduction

This document provides an overview of the testing requirements for the Plane Control module. It outlines the scope of testing, objectives, testing levels and approaches, specific test requirements.

Plane Control Interface Definitions - [Plane Control HAL](https://github.com/rdkcentral/rdk-halif-aidl/tree/0.7.0/planecontrol/current)

## Module Description

The Plane Control `HAL` service provides the interfaces for the video frame queue and deliver video frames to a video plane under the timing of the vendor AV sync.

## Stram Specification




## L2 Test Specification

|Test|Description|HAL APIs|
|----|-----------|--------|
|Set and Get Plane Control Properties|<ul><li>get platform capabilities using `getCapabilities` to get list of supported plane indices</li><li>Loop through all the plane indices</li><li>Use the `setProperty` method to set all the supported value provided in [configuration YAML](../../profiles/hfp-planecontrol.yaml) for all plane resource indices</li><li>Get the property value using `getProperty` of `IPlaneControl` and verify the HAL returns the set Value form the YAML file</li></ul>|<ul><li>IPlaneControl - getCapabilities,setProperty, getProperty </li></ul>|
|Set and Get Multiple Plane Control Properties|<ul><li>Prepare Key value pair of the available properties</li><li>Use the `setPropertyMultiAtomic` method to set all the supported value provided in [configuration YAML](../../profiles/hfp-planecontrol.yaml) for all plane resource indices</li><li>Get the property value using `getPropertyMulti` of `IPlaneControl` and verify the HAL returns the set Value form the YAML file</li></ul>|<ul><li>get platform capabilities using `getCapabilities` to get list of supported plane indices</li><li>Loop through all the plane indices</li><li>IPlaneControl - getCapabilities,setPropertyMultiAtomic, getPropertyMulti </li></ul>|
|Video Source-Plane Mapping verification|<ul><li>get platform capabilities using `getCapabilities` to get list of supported plane indices</li><li>Loop through all of the video plane indices</li><li>Set a Video source to the Video plane using `setVideoSourceDestinationPlaneMapping`</li><li>verify the video source and plane mapping using `getVideoSourceDestinationPlaneMapping`</li></ul>|<ul><li>IPlaneControl - getCapabilities, setVideoSourceDestinationPlaneMapping, getVideoSourceDestinationPlaneMapping </li></ul>|

## L3 Test Specification

|Test|Description|HAL APIs|
|----|-----------|--------|
|Video Source-Plane Mapping verification|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the video plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Set a Video source to the Video plane using `setVideoSourceDestinationPlaneMapping`</li><li>verify the video source and plane mapping using `getVideoSourceDestinationPlaneMapping`</li><li>Verify the Plane is showing the mapped video resource</li><li>Change the source to different plane and verify its reflected in the display </li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, getVideoSourceDestinationPlaneMapping </li></ul>|
|Video Source-Plane swapping|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the video plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Set a Video source to the Video plane 0 and another to VideoPlane 1 using `setVideoSourceDestinationPlaneMapping`</li><li>verify the video source and plane mapping using `getVideoSourceDestinationPlaneMapping`</li><li>Verify the Plane is showing the mapped video resource</li><li>Swap the source and plane and verify its reflected in the display </li> <li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, getVideoSourceDestinationPlaneMapping </li></ul>|
|Aspect Ratio Changes|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the video plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Set a Video source to the Video plane using `setVideoSourceDestinationPlaneMapping`</li><li>Set `ASPECT_RATIO` property using `setProperty` to supported values in configuration YAML</li><li>Verify teh Aspect ratio change is affected in the video plane</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, setProperty </li></ul>|
|Z order Changes|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Set a Video sources  to the Video planes using `setVideoSourceDestinationPlaneMapping`</li><li>Set `ZORDER` property using `setProperty` to supported values in configuration YAML</li><li>Check the Z order position of the Video plane w.r.t to other video plane and graphics plane.</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, setProperty </li></ul>|
|Visibility Changes|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Set a Video sources  to the Video planes using `setVideoSourceDestinationPlaneMapping`</li><li>Set `VISIBLE` property using `setProperty` to supported values in configuration YAML</li><li>Check the visibility  of the planes.</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, setProperty </li></ul>|
|Over scan Changes|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the video plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Set a Video sources  to the Video planes using `setVideoSourceDestinationPlaneMapping`</li><li>Set `OVERSCAN` property using `setProperty` to supported values in configuration YAML</li><li>Check the overscan % of the planes.</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, setProperty </li></ul>|
|Alpha Changes|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Set a Video sources  to the Video planes using `setVideoSourceDestinationPlaneMapping`</li><li>Set `ALPHA` property using `setProperty` to supported values in configuration YAML</li><li>Check the over scan % of the planes.</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, setProperty </li></ul>|
|Plane position changes|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Set a Video sources  to the Video planes using `setVideoSourceDestinationPlaneMapping`</li><li>Set `x` and `y` property using `setProperty` or `setPropertyMultiAtomic` to supported values in configuration YAML</li><li>Check the position of the plane w.r.t reference frame .</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, setProperty </li></ul>|
|Plane size changes|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Set a Video sources  to the Video planes using `setVideoSourceDestinationPlaneMapping`</li><li>Set `WIDTH` and `HEIGHT` property using `setProperty` or `setPropertyMultiAtomic` to supported values in configuration YAML</li><li>Check the size of the plane w.r.t reference frame .</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, setProperty </li></ul>|
|Verify Dynamic Range|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all the video plane indices</li><li> Register event listeners using `registerEventListener`</li><li>create a playback Pipeline or Video source pipe anc connect it with selected plane using `setVideoSourceDestinationPlaneMapping`</li><l1>Loop through all the dynamic range values supported</li><li>Playback the Stream specific to each dynamic range and verify the output</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping</li></ul>|
|Verify Supported pixel format|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all the video plane indices</li><li> Register event listeners using `registerEventListener`</li><li>create a playback Pipeline or Video source pipe anc connect it with selected plane using `setVideoSourceDestinationPlaneMapping`</li><l1>Loop through all the pixel format supported</li><li>Playback the Stream specific to each dynamic range and verify the output</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping</li></ul>|