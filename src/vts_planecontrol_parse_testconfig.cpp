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
 * @addtogroup PLANE_CONTROL Plane Control Module
 * @{
 */

/**
 * @addtogroup PLANE_CONTROL_HALTEST Plane Control HAL Tests
 * @{
 */

/**
 * @defgroup PLANE_CONTROL_HALTEST_PARSE_TESTCONFIG Plane Control HAL Tests parse test configuration file
 * @{
 * @parblock
 *
 * ### Parse test configuration functions for Plane Control :
 *
 * Parse configuration functions required for the module across all vendors.

 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_planecontrol_parse_testconfig.cpp
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
    throw std::invalid_argument("Unknown source type: " + typeStr);
}

static ::com::rdk::hal::planecontrol::AspectRatio stringToAspectRatio(const std::string& ratioStr) {
    if (ratioStr == "FULL_WITH_ASPECT") return ::com::rdk::hal::planecontrol::AspectRatio::FULL_WITH_ASPECT;
    if (ratioStr == "FULL_STRETCH") return ::com::rdk::hal::planecontrol::AspectRatio::FULL_STRETCH;
    if (ratioStr == "ZOOM_WITH_ASPECT") return ::com::rdk::hal::planecontrol::AspectRatio::ZOOM_WITH_ASPECT;
    throw std::invalid_argument("Unknown aspect ratio mode: " + ratioStr);
}

UT_status_t vts_planecontrol_parse_testconfig(void* pTestInstance, const char *module, const char *testGroup, const char *testName,std::vector<planeControlTestConfig_t>& testConfig)
{
    UT_LOG_INFO("Parsing test configuration for module=%s group=%s test=%s",
                module ? module : "<null>",
                testGroup ? testGroup : "<null>",
                testName ? testName : "<null>");
    ut_kvp_status_t status;
    if (pTestInstance == nullptr)
    {
        UT_LOG_ERROR("Invalid test configuration handle (null) for module=%s group=%s test=%s",
                     module ? module : "<null>",
                     testGroup ? testGroup : "<null>",
                     testName ? testName : "<null>");
        return UT_STATUS_FAILURE;
    }
    ut_kvp_instance_t* kvpTestInstance = (ut_kvp_instance_t*)pTestInstance;
    string testList = string(module) + "." + string(testGroup) + "." + string(testName);
    string mapList = testList + "." + "Inputs" + "." + "mapping";
    string stepDelay = testList+ "." + "Controls"+ "." + "delay_between_steps";
    int32_t mapCount = 0;
    uint32_t _stepDelay = ut_kvp_getUInt64Field(kvpTestInstance, stepDelay.c_str());

    mapCount = ut_kvp_getListCount(kvpTestInstance, mapList.c_str());
    if (mapCount <= 0)
    {
        UT_LOG_ERROR("No mapping found in %s", mapList.c_str());
        return UT_STATUS_FAILURE;
    }
    UT_LOG_INFO("Found %d mapping configurations", mapCount);

    for (int32_t i = 0; i < mapCount; i++)
    {

        planeControlTestConfig_t testDetails = {};
        string stepList = mapList + "." + to_string(i) + "." + "step";
        int32_t stepCount = ut_kvp_getListCount(kvpTestInstance, stepList.c_str());
        // === Only parse Control Parameters ===
        testDetails.testName = std::string(testName ? testName : "");
        testDetails.delay_between_steps = _stepDelay;
        if (testDetails.testName == "Planecontrol_Aspectratio")
        {
            const string aspectRatioList = testList + ".Inputs.aspectRatios";
            const int32_t aspectRatioCount = ut_kvp_getListCount(
                kvpTestInstance, aspectRatioList.c_str());
            for (int32_t ratioIndex = 0; ratioIndex < aspectRatioCount; ++ratioIndex)
            {
                char ratioBuffer[UT_KVP_MAX_ELEMENT_SIZE] = {0};
                const string ratioPath = aspectRatioList + "." + to_string(ratioIndex);
                status = ut_kvp_getStringField(kvpTestInstance, ratioPath.c_str(),
                                               ratioBuffer, UT_KVP_MAX_ELEMENT_SIZE);
                if (status == UT_KVP_STATUS_SUCCESS)
                {
                    testDetails.aspectRatios.push_back(stringToAspectRatio(ratioBuffer));
                }
            }
        }
        for(int32_t j=0;j<stepCount;j++)
        {
            ::com::rdk::hal::planecontrol::SourcePlaneMapping map;
            char tempBuffer[UT_KVP_MAX_ELEMENT_SIZE] = {0};
            string planeIndex = stepList + "." + to_string(j) + "." + "plane_index";
            string sourceType = stepList + "." + to_string(j) + "." + "source";
            string sourceIndex = stepList + "." + to_string(j) + "." + "source_index";
            string decoderId = stepList + "." + to_string(j) + "." + "decoderId";
            string positionX = stepList + "." + to_string(j) + "." + "X";
            string positionY = stepList + "." + to_string(j) + "." + "Y";
            string height = stepList + "." + to_string(j) + "." + "height";
            string width = stepList + "." + to_string(j) + "." + "width";
            map.destinationPlaneIndex = ut_kvp_getUInt64Field(kvpTestInstance, planeIndex.c_str());
            status = ut_kvp_getStringField(kvpTestInstance, sourceType.c_str(), tempBuffer, UT_KVP_MAX_ELEMENT_SIZE);
            if (status == UT_KVP_STATUS_SUCCESS)
            {
                map.sourceType = stringToSourceType(string(tempBuffer));
            }
            map.sourceIndex = ut_kvp_getUInt64Field(kvpTestInstance, sourceIndex.c_str());
            testDetails.decoderIds.push_back(ut_kvp_getUInt64Field(kvpTestInstance, decoderId.c_str()));
            testDetails.planeMappings.push_back(map);
            uint32_t _x = ut_kvp_getUInt64Field(kvpTestInstance, positionX.c_str());
            uint32_t _y = ut_kvp_getUInt64Field(kvpTestInstance, positionY.c_str());
            uint32_t _height = ut_kvp_getUInt64Field(kvpTestInstance, height.c_str());
            uint32_t _width = ut_kvp_getUInt64Field(kvpTestInstance, width.c_str());
            testDetails.positions.push_back(std::make_pair(_x,_y ));
            testDetails.size.push_back(std::make_pair(_height,_width));
        }
        testConfig.push_back(testDetails);

    }
    return UT_STATUS_OK;
}





/** @} */ // End of PLANE_CONTROL_HALTEST_PARSE_TESTCONFIG
/** @} */ // End of PLANE_CONTROL_HALTEST
/** @} */ // End of PLANE_CONTROL
/** @} */ // End of HPK
