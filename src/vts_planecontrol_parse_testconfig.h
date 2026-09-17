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
 * @addtogroup PLANE_CONTROL Planecontrol Module
 * @{
 */

/**
 * @addtogroup PLANE_CONTROL_HALTEST Planecontrol HAL Tests
 * @{
 */

/**
 * @defgroup PLANE_CONTROL_HALTEST_PARSE_TESTCONFIG_HEADER Planecontrol HAL Tests parse test configuration Header File
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

struct planeControlTestConfig_t{
    std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping> planeMappings;
    std::vector<uint32_t> decoderIds;
    std::vector<std::pair<uint32_t,uint32_t>> positions;
    std::vector<std::pair<uint32_t,uint32_t>> size;
    std::string testName;
    uint32_t delay_between_steps;
};

/**
 * @brief Creates a KVP (Key-Value Pair) test instance from the specified configuration file.
 *
 * @param[in] fileName Path to the configuration file to be loaded.
 * @return Pointer to the created test instance, or NULL on failure.
 *
 * @note The returned instance should be destroyed using vts_kvp_destroy_instance().
 */
void* vts_kvp_create_instance(char *fileName);


/**
 * @brief Parses the test configuration for the PlaneControl module.
 *
 * @param[in] pTestInstance Pointer to the test instance created by vts_kvp_create_instance().
 * @param[in] module Name of the module to parse configuration for.
 * @param[in] testGroup Name of the test group.
 * @param[in] testName Name of the specific test.
 * @param[out] testConfigs Vector to be populated with parsed test configuration data.
 * @return UT_status_t Status of the parsing operation (e.g., UT_SUCCESS on success, error code otherwise).
 *
 * @note The function populates the testConfigs vector with the parsed configuration.
 */
UT_status_t vts_planecontrol_parse_testconfig(void* pTestInstance, const char *module, const char *testGroup, const char *testName,std::vector<planeControlTestConfig_t>& testConfigs);

/**
 * @brief Destroys a previously created KVP test instance and releases associated resources.
 *
 * @param[in] instance Pointer to the test instance to be destroyed.
 * @return void
 *
 * @note After calling this function, the instance pointer must not be used.
 */
void vts_kvp_destroy_instance(void *instance);

#endif //__VTS_PLANECONTROL_PARSE_TESTCONFIG_H__


/** @} */ // End of PLANECONTROL_HALTEST_PARSE_TESTCONFIG_HEADER
/** @} */ // End of PLANECONTROL_HALTEST
/** @} */ // End of PLANECONTROL
/** @} */ // End of HPK
