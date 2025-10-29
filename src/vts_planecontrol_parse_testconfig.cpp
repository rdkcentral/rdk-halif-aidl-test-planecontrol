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
 * @addtogroup VIDEO_SINK Video Sink Module
 * @{
 */

/**
 * @addtogroup VIDEO_SINK_HALTEST Video Sink HAL Tests
 * @{
 */

/**
 * @defgroup VIDEO_SINK_HALTEST_PARSE_TESTCONFIG Video Sink HAL Tests parse test configuration file
 * @{
 * @parblock
 *
 * ### Parse test configuration functions for Video Sink :
 *
 * Parse configuration functions required for the module across all vendors.

 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_videosink_parse_testconfig.cpp
 *
 */

#include <filesystem>
#include "vts_planecontrol_parse_testconfig.h"
#include "vts_planecontrol.h"

using namespace std;
static ::com::rdk::hal::planecontrol::SourceType stringToSourceType(const std::string& typeStr) {
    if (typeStr == "NONE") return ::com::rdk::hal::planecontrol::SourceType::NONE;
    if (typeStr == "VIDEO_SINK") return ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
    if (typeStr == "HDMI") return ::com::rdk::hal::planecontrol::SourceType::HDMI;
    if (typeStr == "COMPOSITE") return ::com::rdk::hal::planecontrol::SourceType::COMPOSITE;
    throw std::invalid_argument("Unknown plane type: " + typeStr);
}
UT_status_t vts_planecontrol_parse_testconfig(void* pTestInstance, const char *module, const char *testGroup, const char *testName,std::vector<planeControlTestConfig_t>& testConfig)
{
    ut_kvp_status_t status;
    ut_kvp_instance_t* kvpTestInstance = (ut_kvp_instance_t*)pTestInstance;
    string testList = string(module) + "." + string(testGroup) + "." + string(testName);
    string mapList = testList + "." + "Inputs" + "." + "Mapping";
    int32_t mapCount = 0;

    mapCount = ut_kvp_getListCount(kvpTestInstance, mapList.c_str());
    if (mapCount <= 0)
    {
        UT_LOG_ERROR("No mapping found in %s", mapList.c_str());
        return UT_STATUS_FAILURE;
    }
    UT_LOG_INFO("Found %d streams", mapCount);

    for (int32_t i = 0; i < mapCount; i++)
    {

        planeControlTestConfig_t testDetails = {};
        string configList = mapList + "." + to_string(i) + "test";
        int32_t configCount = 0;
        // === Only parse Control Parameters ===
        testDetails.testName = std::string(testName ? testName : "");
        for(int32_t j=0;j<configCount;j++)
        {
            ::com::rdk::hal::planecontrol::SourcePlaneMapping map;
            char tempBuffer[UT_KVP_MAX_ELEMENT_SIZE] = {0};
            string planeIndex = configList + "." + to_string(j) + "plane_index";
            string sourceType = configList + "." + to_string(j) + "source";
            string sourceIndex = configList + "." + to_string(j) + "souce_index";
            string decoderId = configList + "." + to_string(j) + "decoderId";
            map.destinationPlaneIndex = ut_kvp_getUInt64Field(kvpTestInstance, planeIndex.c_str());
            status = ut_kvp_getStringField(kvpTestInstance, sourceType.c_str(), tempBuffer, UT_KVP_MAX_ELEMENT_SIZE);
            if (status == UT_KVP_STATUS_SUCCESS)
            {
                map.sourceType = stringToSourceType(string(tempBuffer));
            }
            map.sourceIndex = ut_kvp_getUInt64Field(kvpTestInstance, sourceIndex.c_str());
            uint32_t _decoderId = ut_kvp_getUInt64Field(kvpTestInstance, decoderId.c_str());
            testDetails.decoderIds.push_back(_decoderId);
            testDetails.planeMappings.push_back(map);
        }
        testConfig.push_back(testDetails);

    }
    return UT_STATUS_OK;
}





/** @} */ // End of VIDEO_SINK_HALTEST_PARSE_TESTCONFIG
/** @} */ // End of VIDEO_SINK_HALTEST
/** @} */ // End of VIDEO_SINK
/** @} */ // End of HPK
