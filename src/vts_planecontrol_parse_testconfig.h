/**
 *  If not stated otherwise in this file or this component's LICENSE
 *  file the following copyright and licenses apply:
 *
 *  Copyright 2024 RDK Management
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
 * @addtogroup HPK Hardware Porting Kit
 * @{
 * @par The Hardware Porting Kit
 * HPK is the next evolution of the well-defined Hardware Abstraction Layer
 * (HAL), but augmented with more comprehensive documentation and test suites
 * that OEM or SOC vendors can use to self-certify their ports before taking
 * them to RDKM for validation or to an operator for final integration and
 * deployment. The Hardware Porting Kit effectively enables an OEM and/or SOC
 * vendor to self-certify their own Video Accelerator devices, with minimal RDKM
 * assistance.
 *
 */

/**
 * @addtogroup PLANECONTROL Planecontrol Module
 * @{
 */

/**
 * @addtogroup PLANECONTROL_HALTEST Planecontrol HAL Tests
 * @{
 */

/**
 * @defgroup PLANECONTROL_HALTEST_PARSE_TESTCONFIG_HEADER Planecontrol HAL Tests parse test configuration Header File
 * @{
 * @parblock
 *
 * ### Parse test configuration functions for Plane Control :
 *
 * Parse test configuration functions required for the module across all vendors.
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_planecontrol_parse_testconfig.h
 *
 */
#ifndef __VTS_PLANECONTROL_PARSE_TESTCONFIG_H__
#define __VTS_PLANECONTROL_PARSE_TESTCONFIG_H__

#include <ut.h>
#include <ut_log.h>
#include <vector>
#include "ut_kvp_profile.h"
#include "vts_videosink_helper_functions.h"
#include "vts_videosink_parse_hfp.h"
#include "vts_planecontrol.h"

#define PLANE_CONTROL_TEST_CONFIG_PATH "./planecontrol_test_configuration/vts_l3_PlaneControl_test_config.yaml"

struct planeControlTestConfig_t{
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> planeMappings;
    std::vector<uint32_t> decoderIds;
    std::vector<std::pair<uint32_t,uint32_t>> positions;
    std::vector<std::pair<uint32_t,uint32_t>> size;
    uint32_t graphicsPlaneIndex;
    std::string testName;
    uint32_t stepDelay;
};

void* vts_kvp_create_instance(char *fileName);
UT_status_t vts_planecontrol_parse_testconfig(void* pTestInstance, const char *module, const char *testGroup, const char *testName,std::vector<planeControlTestConfig_t>& testConfigs);
void vts_kvp_destroy_instance(void *instance);

#endif //__VTS_PLANECONTROL_PARSE_TESTCONFIG_H__


/** @} */ // End of PLANECONTROL_HALTEST_PARSE_TESTCONFIG_HEADER
/** @} */ // End of PLANECONTROL_HALTEST
/** @} */ // End of PLANECONTROL
/** @} */ // End of HPK
