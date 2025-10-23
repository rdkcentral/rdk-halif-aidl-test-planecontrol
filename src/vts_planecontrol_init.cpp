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

/**
 * @addtogroup Plane_Control Plane Control Module
 * @{
 */

/**
 * @addtogroup Plane_Control_Halif_Test Plane Control HALIF Tests
 * @{
 */

/**
 * @defgroup Plane_Control_Halif_Test_Init Plane Control HALIF Tests Init File
 * @{
 * @parblock
 *
 * ### Initialization functions for Plane Control VTS HALIF AIDL tests :
 *
 *
 *
 * **Pre-Conditions:**  None@n
 * **Dependencies:** None@n
 *
 * Ref to API Definition specification documentation :
 *
 * @endparblock
 */


/**
 * @file vts_planecontrol_init.cpp
 *
 */
#include "vts_planecontrol.h"
#include "vts_planecontrol_parse_testconfig.h"

sp<IPlaneControl> gPlaneControlservice;
void *gPlaneControlTestConfiguration;

UT_status_t VTS_PlaneControl_init()
{

    if(getPlaneControlService() != UT_STATUS_OK)
    {
        return UT_STATUS_FAILURE;
    }
    if(vts_planecontrol_parsePlaneControlHFP() != UT_STATUS_OK)
    {
        UT_LOG_ERROR("Failed to process the HFP file");
        return UT_STATUS_FAILURE;
    }
    gPlaneControlTestConfiguration = vts_kvp_create_instance(PLANE_CONTROL_TEST_CONFIG_PATH);

    return UT_STATUS_OK;
}

 /** @} */ // End of Plane_Control_Halif_Test_Init
 /** @} */ // End of Plane_Control_Halif_Test
 /** @} */ // End of Plane_Control
 /** @} */ // End of HPK
