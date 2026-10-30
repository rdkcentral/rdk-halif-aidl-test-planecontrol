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

#include "vts_planecontrol_test.h"
#include "vts_planecontrol.h"
#include "vts_input_path_helpers.h"

#include <algorithm>

bool PlaneControlTest::startCompositeInputPath(int32_t compositeInputIndex)
{
    const auto input = vts::input_path::findInputPathEntry(
        mCompositeInputList, compositeInputIndex,
        [](const auto& entry) { return entry.portId; });
    UT_ASSERT_TRUE(input != mCompositeInputList.end())
        << "No opened composite input found for source index " << compositeInputIndex;
    if (input == mCompositeInputList.end())
    {
        return false;
    }

    COMPOSITEINPUT_START(input->controller);
    return true;
}

void PlaneControlTest::stopCompositeInputPath(int32_t compositeInputIndex)
{
    const auto input = vts::input_path::findInputPathEntry(
        mCompositeInputList, compositeInputIndex,
        [](const auto& entry) { return entry.portId; });
    if (input != mCompositeInputList.end())
    {
        COMPOSITEINPUT_STOP(input->controller);
    }
}