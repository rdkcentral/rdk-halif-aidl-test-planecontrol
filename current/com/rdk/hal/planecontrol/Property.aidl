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
  
/** 
 *  @brief     Plane properties definition.
 *  @author    Luc Kennedy-Lamb
 *  @author    Peter Stieglitz
 *  @author    Douglas Adler
 *  @copyright 
 */
 
@VintfStability
@Backing(type="int")
enum Property
{
    /**
	 * Left coordinate of the plane inside its frame coordinates. 
	 */
    X = 0,
 
    /**
	 * Top coordinate of the plane inside its frame coordinates. 
	 */
    Y = 1,
 
    /**
	 * Width of the plane inside its reference frame coordinates. 
	 */
    WIDTH = 2,
 
    /**
	 * Height of the plane inside its reference frame coordinates. 
	 */
    HEIGHT = 3,
 
    /**
	 * Alpha blending value for the whole plane. 0=transparent, 255=opaque. 
	 */
    ALPHA = 4,
 
    /**
	 * Z-order position of the plane. Higher z-order planes are composited above lower z-order planes. 
	 */
    ZORDER = 5,
 
    /**
	 * Visibility of the plane. 0=invisible, 1=visible. 
	 */
    VISIBLE = 6,
  
    /**
	 * Overscan applied to a video plane. Overscan is specified in percentage and applied equally to all edges.
     * 0=full (no overscan).
     * Overscan does not apply to graphics planes.
     */
    OVERSCAN = 7,
 
    /**
	 * Aspect ratio applied to a video plane.
     * Values are enumerated in AspectRatio.
     */
    ASPECT_RATIO = 8,
 
    /**
	 * Metrics 
	 */
    
    /**
	 * Count of frames received.  Not necessarily displayed.
	 * -1 means this metric is not yet implemented by the vendor.
	 */  
	METRIC_FRAMES_RECEIVED = 1000,
    
    /**
	 * Count of frames presented.
	 * -1 means this metric is not yet implemented by the vendor.
	 */  
	METRIC_FRAMES_PRESENTED = 1001,
	
    /**
	 * Count of frames dropped, received but never displayed due to late delivery.
	 * Not relevant for graphics plane.
	 * -1 means this metric is not yet implemented by the vendor.
	 */  
    METRIC_FRAMES_DROPPED_LATE = 1002,
	
    /**
	 * Count of frames dropped, due to frame rate conversion.
	 * Not relevant for graphics plane.
	 * -1 means this metric is not yet implemented by the vendor.
	 */  
    METRIC_FRAMES_DROPPED_FRC = 1003,
	
    /**
	 * Count of frames repeated, due to frame rate conversion.
	 * Not relevant for graphics plane.
	 * -1 means this metric is not yet implemented by the vendor.
	 */  
    METRIC_FRAMES_REPEATED_FRC = 1004,
	
    /**
	 * Count of frames repeated, due to a missing frame at expected presentation time.
	 * Not relevant for graphics plane.
	 * -1 means this metric is not yet implemented by the vendor.
	 */  
    METRIC_FRAMES_REPEATED_MISSING_FRAME = 1005,
	
    /**
	 * Count of underflow events, due to empty frame queue at expected presentation time.
	 * Not relevant for graphics plane.
	 * -1 means this metric is not yet implemented by the vendor.
	 */  
    METRIC_UNDERFLOWED = 1006,
}
