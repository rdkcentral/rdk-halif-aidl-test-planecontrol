# Plane Control High Level Test Specification Document

## Table of Contents

- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Introduction](#introduction)
- [Module Description](#module-description)
- [Video Playback Specification](#video-playback-specification)
- [Graphics Plane Test Application](#graphics-plane-test-application)
- [vsyncDisplayLatency Measurement](#vsyncdisplaylatency-measurement)
- [Video Plane Geometry Specification](#video-plane-geometry-specification)
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

## Video Playback Specification

Plane control supports 3 types of Video sources

* HDMI
* Composite
* Video Sink

Follow the below steps to playback required video using each source

|Source|Achieve Video Playback|
|------|----------------------|
|HDMI  |Connect an HDMI capable device to the Sink port. Playback the required Video as per the test requirement e.g: HDR10, SD, HD etc|
|Composite|Connect a Composite capable device to the sink port. Play back the supported/capable videos as per the test requirement.|
|Video Sink| Create a Video pipeline using the AV HAL objects. Play back the streams as specified in the [streams info](https://github.com/rdkcentral/rdk-halif-binder-test-videodecoder/blob/feature/issues4_Create_HLT/doc/pages/VideoDecoder_TestStreamsInfo.md) document as required for the test.|

## Graphics Plane Test Application

To Validate the Graphics plane we need to create an EGL sample app which will provide the graphics plane data to VTS. This could be an application which  draws a simple triangle to the plane.

_TBD: EGL a separate Binary or Within VTS binary so that VTS has control over what it draws ?_

## vsyncDisplayLatency Measurement

_TBD: How to measure the time delay between presenting the frame to displayed_

## Video Plane Geometry Specification

Video Plane can be set to different size and shapes by setting following properties of the planes

<li>X</li><li>Y</li><li>WIDTH</li><li>HEIGHT</li>

|GEOMETRY|Property Values|
|--------|---------------|
|Full Scressn| Set `X=0`, `Y=0`, `HEIGHT = frame Height from capabilities`, `WIDTH = frame width from capabilities`|
|Side By Side| <li>1st Plane `X=0`, `Y=0`, `HEIGHT = frameHeight`, `WIDTH = frameWidth/2`</li><li>2nd Plane `X=frameWidth/2`, `Y=0`, `HEIGHT = frameHeight`, `WIDTH = frameWidth/2`</li>|
|PIP Window| <li>1st Plane `X=0`, `Y=0`, `HEIGHT = frameHeight`, `WIDTH = frameWidth`</li><li>2nd Plane `X=3*frameWidth/4`, `Y=frameHeight/4`, `HEIGHT = frameHeight/4`, `WIDTH = frameWidth/4`</li>|
|3X4 plane window|Set `X=frameHeight/8`, `Y=0`, `HEIGHT = frameHeight`, `WIDTH = 3*frameHeight/4`|
## L2 Test Specification

|Test|Description|HAL APIs|
|----|-----------|--------|
|Set and Get Plane Control Properties|<ul><li>get platform capabilities using `getCapabilities` to get list of supported plane indices</li><li>Loop through all the plane indices</li><li>Use the `setProperty` method to set all the supported value provided in [configuration YAML](../../profiles/hfp-planecontrol.yaml) for all plane resource indices</li><li>Get the property value using `getProperty` of `IPlaneControl` and verify the HAL returns the set Value form the YAML file</li></ul>|<ul><li>IPlaneControl - getCapabilities,setProperty, getProperty </li></ul>|
|Set and Get Multiple Plane Control Properties|<ul><li>get platform capabilities using `getCapabilities` to get list of supported plane indices</li><li>Loop through all the plane indices</li><li>Prepare Key value pair of the available properties</li><li>Use the `setPropertyMultiAtomic` method to set all the supported value provided in [configuration YAML](../../profiles/hfp-planecontrol.yaml) for all plane resource indices</li><li>Get the property value using `getPropertyMulti` of `IPlaneControl` and verify the HAL returns the set Value form the YAML file</li></ul>|<ul><li>get platform capabilities using `getCapabilities` to get list of supported plane indices</li><li>Loop through all the plane indices</li><li>IPlaneControl - getCapabilities,setPropertyMultiAtomic, getPropertyMulti </li></ul>|
|Video Source-Plane Mapping verification|<ul><li>get platform capabilities using `getCapabilities` to get list of supported plane indices</li><li>Loop through all of the video plane indices</li><li>Set a Video source to the Video plane using `setVideoSourceDestinationPlaneMapping`</li><li>verify the video source and plane mapping using `getVideoSourceDestinationPlaneMapping`</li></ul>|<ul><li>IPlaneControl - getCapabilities, setVideoSourceDestinationPlaneMapping, getVideoSourceDestinationPlaneMapping </li></ul>|

## L3 Test Specification

|Test|Description|HAL APIs|
|----|-----------|--------|
|Video Source-Plane Mapping verification|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the video plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Loop through all the video sources</li><li>Set the Video source to the Video plane using `setVideoSourceDestinationPlaneMapping`</li><li>verify the video source and plane mapping using `getVideoSourceDestinationPlaneMapping`</li><li>Play video using the selected Video source</li><li>Verify the Plane is showing the mapped video resource</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, getVideoSourceDestinationPlaneMapping </li></ul>|
|Video Source-Plane swapping|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the video plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Set a Video source to the Video plane 0 and another to VideoPlane 1 using `setVideoSourceDestinationPlaneMapping`</li><li>verify the video source and plane mapping using `getVideoSourceDestinationPlaneMapping`</li><li>Update each video plane geometry of each plane as per the geometry specification</li><li>PLay video using the selected source</li><li>Verify the Plane is showing the mapped video resources</li><li>Swap the source and plane and verify its reflected in the display </li> <li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, getVideoSourceDestinationPlaneMapping </li></ul>|
|Aspect Ratio Changes|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the video plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Loop through all the video sources</li><li>Set the Video source to the Video plane using `setVideoSourceDestinationPlaneMapping`</li><li>Update the video plane geometry of each plane as per the geometry specification</li><li>Set `ASPECT_RATIO` property using `setProperty` for all supported values </li><li>Verify the Aspect ratio change is affected in the video plane</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, setProperty </li></ul>|
|Z order Changes|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Set a Video sources  to the Video planes using `setVideoSourceDestinationPlaneMapping`</li><li>Set `ZORDER` property using `setProperty` to supported values in configuration YAML</li><li>Check the Z order position of the Video plane w.r.t to other video plane and graphics plane.</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, setProperty </li></ul>|
|Visibility Changes|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Set a Video sources  to the Video planes using `setVideoSourceDestinationPlaneMapping`</li><li>Loop through all the video sources</li><li>Set the Video source to the Video plane using `setVideoSourceDestinationPlaneMapping`</li><li>Update the video plane geometry of each plane as per the geometry specification</li><li>Set `VISIBLE` property using `setProperty` to supported values in configuration YAML</li><li>Check the visibility  of the planes.</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, setProperty </li></ul>|
|Over scan Changes|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the video plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Loop through all the video sources</li><li>Set the Video source to the Video plane using `setVideoSourceDestinationPlaneMapping`</li><li>Update the video plane geometry of each plane as per the geometry specification</li><li>Set `OVERSCAN` property using `setProperty` to supported values in configuration YAML</li><li>Check the overscan % of the planes.</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, setProperty </li></ul>|
|Alpha Changes|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Set a Video sources  to the Video planes using `setVideoSourceDestinationPlaneMapping`</li><li>Loop through all the video sources</li><li>Set the Video source to the Video plane using `setVideoSourceDestinationPlaneMapping`</li><li>Set `ALPHA` property using `setProperty` to supported values in configuration YAML</li><li>Check the over scan % of the planes.</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, setProperty </li></ul>|
|Plane position changes|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Set a Video sources  to the Video planes using `setVideoSourceDestinationPlaneMapping`</li><li>Loop through all the video sources</li><li>Set the Video source to the Video plane using `setVideoSourceDestinationPlaneMapping`</li><li>Set `x` and `y` property using `setProperty` or `setPropertyMultiAtomic` to supported values in configuration YAML</li><li>Check the position of the plane w.r.t reference frame .</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, setProperty </li></ul>|
|Plane size changes|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all of the plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Set a Video sources  to the Video planes using `setVideoSourceDestinationPlaneMapping`</li><li>Loop through all the video sources</li><li>Set the Video source to the Video plane using `setVideoSourceDestinationPlaneMapping`</li><li>Set `WIDTH` and `HEIGHT` property using `setProperty` or `setPropertyMultiAtomic` to supported values in configuration YAML</li><li>Check the size of the plane w.r.t reference frame .</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping, setProperty </li></ul>|
|Verify Dynamic Range|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all the video plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Loop through all the video sources</li><li>Set the Video source to the Video plane using `setVideoSourceDestinationPlaneMapping`</li><l1>Loop through all the dynamic range values supported</li><li>Playback the Stream specific to each dynamic range and verify the output</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping</li></ul>|
|Verify Supported pixel format|<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all the video plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Loop through all the video sources</li><li>Set the Video source to the Video plane using `setVideoSourceDestinationPlaneMapping`</li><li>Update the video plane geometry of each plane as per the geometry specification</li><l1>Loop through all the pixel format supported</li><li>Playback the Stream specific to each dynamic range and verify the output</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping</li></ul>|
|Verify graphics Window Handle |<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all the graphics plane indices</li><li> Register event listeners using `registerEventListener`</li><li>get the graphics plan handle for EGL application using `getNativeGraphicsWindowHandle`</li><li>Draw a sample graphics using the handle from HAL</li><li>Force the graphics draw using `flipGraphicsBuffer`</li><li>Verify the graphics plane is updated as intended </li><li>Release the Graphics window handle using `releaseNativeGraphicsWindowHandle`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping</li></ul>|
|Verify vsyncDisplayLatency |<ul><li>get platform capabilities using `getCapabilities`</li><li>Loop through all the video plane indices</li><li> Register event listeners using `registerEventListener`</li><li>Loop through all the video sources</li><li>Set the Video source to the Video plane using `setVideoSourceDestinationPlaneMapping`</li><li>Update the video plane geometry of each plane as per the geometry specification</li><li>Playback the Stream verify the output</li><li>Measure the time difference between the frame presented to viewed in the display is as per specified in the capability</li><li>Unregister event listeners using `unregisterEventListener`</li></ul>|<ul><li>IPlaneControl - registerEventListener, unregisterEventListener, getCapabilities, setVideoSourceDestinationPlaneMapping</li></ul>|
