#!/bin/sh

# *
# * If not stated otherwise in this file or this component's LICENSE file the
# * following copyright and licenses apply:
# *
# * Copyright 2024 RDK Management
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# * http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *

###########################################################
# Print info message
###########################################################
LOGI() {
    echo "${LOG_INFO} ${LOG_TAG} ${1}"
}

###########################################################
# Print error message
###########################################################
LOGE() {
    echo "${LOG_ERROR} ${LOG_TAG} ${1}"
}

# Basic commands
RM="rm -rf"
MKDIR="mkdir -p"
MAKE="make"
CMAKE="cmake"
GIT="git"
CD="cd"

LOG_TAG="<PLANE-CONTROL-SERVICE>"
LOG_INFO="<INFO>"
LOG_ERROR="<ERROR>"

# Define and export global variables
export WORK_DIR=$(pwd)
export HAL_DIR="${WORK_DIR}/../../hal"
export HAL_BRANCH="develop"
export HAL_COMP_DIR1="videodecoder"
export NEED_COMMON=true

export PLANECONTROL_AIDL_DIR="${WORK_DIR}/../../"
export VERSION="current"

export LINUX_BINDER_DIR="${WORK_DIR}/../../../rdk-halif-binder"
export INSTALL_DIR="${WORK_DIR}/output"

###########################################################
# Clone hal repo
###########################################################
clone_hal_repo() {
    LOGI "Cloning hal repo"
    ${CD} "${WORK_DIR}"

    ${RM} "${HAL_DIR}"
    ${GIT} clone https://github.com/rdk-e/hal.git -b "${HAL_BRANCH}" "${HAL_DIR}"
    if [ $? -ne 0 ]; then
        LOGE "Failed to clone hal repository."
        exit 1
    fi
}

#######################################################################
# Generate hal proxy & native interface using AIDL for common if needed
#######################################################################
build_aidl_for_common() {
    if [ "${NEED_COMMON}" = "true" ]; then
        LOGI "Building common dependency with AIDL"
        
	HAL_COMMON="${HAL_DIR}/common"
        
	${CD} "${HAL_COMMON}"

        ${LINUX_BINDER_DIR}/local/bin/aidl --lang=cpp --min_sdk_version=33 --structured --stability=vintf \
            -I${HAL_COMMON} -h ${HAL_COMMON}/output/h -o ${HAL_COMMON}/output/cpp \
	    ${HAL_COMMON}/${VERSION}/com/rdk/hal/*.aidl 

        if [ $? -ne 0 ]; then
            LOGE "Failed to generate proxy & native interface in: ${HAL_COMMON}"
            ${CD} "${WORK_DIR}"
            return 1
        fi

        LOGI "Successfully generated HAL component proxy & native interface in ${HAL_COMMON}"
        ${CD} "${WORK_DIR}"
    else
        LOGI "Skipping common dependency as it is not required"
    fi
}

###########################################################
# Generate hal proxy & native interface
###########################################################
build_aidl_hal() {
    LOGI "Generating hal proxy & native interface"

    # List of directories and their corresponding HAL versions
    HALS=(
        "${HAL_COMP_DIR1}"
    )

    # Iterate over each HAL directory
    for HAL_COMP_DIR in "${HALS[@]}"; do
        ${CD} ${HAL_DIR}
        ${CMAKE} -DAIDL_TARGET=${HAL_COMP_DIR} -DAIDL_BIN="${LINUX_BINDER_DIR}/local/bin/aidl" .


        if [ $? -ne 0 ]; then
            LOGE "Failed to build in directory: ${HAL_COMP_DIR}"
            ${CD} ${WORK_DIR}
            return 1
        fi

        LOGI "Successfully built HAL component  proxy & native interface in ${HAL_COMP_DIR}"
        ${CD} ${WORK_DIR}
    done

}

###########################################################
# Build PlaneControl Service
###########################################################
build_planecontrol_service() {
    LOGI "Building PlaneControl service"
    
    ${CMAKE} -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" -DLINUX_BINDER_OUT_DIR="${LINUX_BINDER_DIR}/local/" -DAIDL_EXAMPLE_GEN_DIR="${PLANECONTROL_AIDL_DIR}" -DPLANECONTROL_AIDL_VERSION="${PLANECONTROL_AIDL_DIR}/${VERSION}" \
    && ${MAKE} install -j"$(nproc)"

    if [ $? -ne 0 ]; then
        LOGE "Failed to build the linux binder device tool"
        ${CD} "${WORK_DIR}"
        return 1
    fi

    LOGI "Successfully built mockdevicecontrol service"
    ${CD} "${WORK_DIR}"
}

# Main Execution
clone_hal_repo
build_aidl_for_common
build_aidl_hal
build_planecontrol_service

