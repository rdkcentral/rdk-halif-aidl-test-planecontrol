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
package com.rdk.hal.planecontrol;
import com.rdk.hal.planecontrol.IPlaneControlListener;
import com.rdk.hal.planecontrol.PlaneResource;
import com.rdk.hal.planecontrol.SourcePlaneMapping;
import com.rdk.hal.planecontrol.Property;
import com.rdk.hal.planecontrol.PropertyKVPair;
import com.rdk.hal.PropertyValue;
 
/** 
 *  @brief     Plane Control HAL interface.
 *  @author    Luc Kennedy-Lamb
 *  @author    Peter Stieglitz
 *  @author    Douglas Adler
 */
 
@VintfStability
interface IPlaneControl
{
    /** The service name to publish. To be returned by getServiceName() in the derived class. */
    const @utf8InCpp String serviceName = "PlaneControl";

    /**
     * Gets the platform plane resources and their capabilities.
     * 
     * This function can be called at any time and is not dependant on any Plane Control state.
     * The returned value is not allowed to change between calls.
     * 
     * @returns PlaneResource[].
     */
    PlaneResource[] getResourceCapabilities();
 
    /**
     * Gets the native graphics window handle to couple with an EGL display.
     * 
     * This function can be called multiple times, but will return the same native graphics window
     * handle on successive calls.  No reference counting is expected.
     * 
     * @param[in] planeResourceIndex    The plane resource index.
     * 
     * @returns Native graphics window handle.
     * 
     * @exception binder::Status EX_ILLEGAL_ARGUMENT
     * 
     * @see releaseNativeGraphicsWindowHandle()
     */
    long getNativeGraphicsWindowHandle(in int planeResourceIndex);
 
    /**
     * Releases the native graphics window handle previously returned from a call to getNativeGraphicsWindowHandle().
     * 
     * @param[in] planeResourceIndex    The plane resource index.
     * @param[in] nativeWindowHandle    A native window handle.
     * 
     * @exception binder::Status EX_ILLEGAL_ARGUMENT
     * 
     * @see getNativeGraphicsWindowHandle()
     */
    void releaseNativeGraphicsWindowHandle(in int planeResourceIndex, in long nativeWindowHandle);

    /**
     * Flips the latest swapped graphics buffer (e.g. provided by eglSwapBuffers) to the display.
     * 
     * @param[in] planeResourceIndex        The plane resource index.
     * 
     * @returns HALResult enum value.
     * 
     * @exception binder::Status EX_ILLEGAL_ARGUMENT
     * 
     * @see getNativeGraphicsWindowHandle()
     */
    void flipGraphicsBuffer(in int planeResourceIndex);
 
    /**
     * Sets the destination plane for one or more video sources.
     * 
     * This function allows multiple video [source -> plane] mappings to be changed together.
     * Operations such as main and PIP video swaps can be performed using this call.
     * 
     * If a plane is already mapped and is not being remapped by this function then the call fails with EX_ILLEGAL_STATE.
     *
     * @param[in] listSourcePlaneMapping    An array of video source to video plane mappings.
     *
     * @exception binder::Status EX_ILLEGAL_ARGUMENT
     * @exception binder::Status EX_ILLEGAL_STATE 
     */
    void setVideoSourceDestinationPlane(in SourcePlaneMapping[] listSourcePlaneMapping);
  
    /**
     * Gets a property of a plane.
     * 
     * @param[in] planeResourceIndex    The plane resource index.
     * @param[in] property              Property enum.
     * 
     * @returns PropertyValue.
     * 
     * @exception binder::Status EX_ILLEGAL_ARGUMENT
     * 
     * @see setProperty(), getPropertyMulti()
     */
    PropertyValue getProperty(in int planeResourceIndex, in Property property);

    /**
     * Sets a property of a plane.
     * 
     * @param[in] planeResourceIndex    The plane resource index.
     * @param[in] property              Property enum.
     * @param[in] propertyValue         Property value.
     * 
     * @returns PropertyValue.
     * 
     * @exception binder::Status EX_ILLEGAL_ARGUMENT
     * 
     * @see getProperty(), setPropertyMultiAtomic()
     */
    void setProperty(in int planeResourceIndex, in Property property, in PropertyValue propertyValue);
 
    /**
     * Gets multiple properties for a plane.
     * 
     * When calling the getPropertyMulti() the propertyKVList parameter contains an array of
     * PropertyKVPair parcelables that have the property set.
     * On success the propertyValue is set in the returned array.
     * 
     * @param[in] planeResourceIndex    The plane resource index.
     * @param[in,out] propertyKVList    Array of property key-value pairs.
     * 
     * @exception binder::Status EX_ILLEGAL_ARGUMENT
     * 
     * @see getProperty(), setPropertyMultiAtomic()
     */
    void getPropertyMulti(in int planeResourceIndex, inout PropertyKVPair[] propertyKVList);
  
    /**
     * Sets multiple properties for a plane.
     * 
     * @param[in] planeResourceIndex    The plane resource index.
     * @param[in] propertyKVList        Array of property key-value pairs.
     * 
     * @exception binder::Status EX_ILLEGAL_ARGUMENT
     * 
     * @see setProperty(), getPropertyMulti()
     */
    void setPropertyMultiAtomic(in int planeResourceIndex, in PropertyKVPair[] propertyKVList);
 
    /**
     * Registers a callback listener for events generated by the Plane Control.
     *
     * An `IPlaneControlListener` can only be registered once and will fail on subsequent
     * registration attempts.
     *
     * @param[in] listener              Reference to IPlaneControlListener interface.
     * 
     * @return boolean
     * @retval true     The event listener was registered.
     * @retval false    The event listener is already registered.
     * 
     * @see unregisterListener()
     */
    boolean registerListener(in IPlaneControlListener listener);
 
    /**
     * Unregisters a callback listener for events generated by the Plane Control.
     *
     * @param[in] listener              Reference to IPlaneControlListener interface.
     *
     * @return boolean
     * @retval true     The event listener was unregistered.
     * @retval false    The event listener was not found registered.
     *
     * @see registerListener()
     */
    boolean unregisterListener(in IPlaneControlListener listener);
}
