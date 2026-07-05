#!/bin/bash
#* ******************************************************************************
#*  If not stated otherwise in this file or this component's LICENSE
#*  file the following copyright and licenses apply:
#*
#*  Copyright 2026 RDK Management
#*
#*  Licensed under the Apache License, Version 2.0 (the License);
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*  http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an AS IS BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#*
#* ******************************************************************************

set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

declare -r TOP="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
declare -r BUILD_DIR="${TOP}/build"
declare -r BUILD_INSTALL_PREFIX="${BUILD_DIR}/usr"
declare -r MOCK_SERVICE_DIR="${TOP}/mockservice"
declare -r RDK_HAL_DIR="${TOP}/rdk-halif-aidl"
declare -r DEFAULT_AIDL_MANIFEST="versions_released.yaml"
declare -r DEF_LINUX_BINDER="${TOP}/linux_binder_idl"

AIDL_VERSION_MANIFEST="${AIDL_VERSION_MANIFEST:-${DEFAULT_AIDL_MANIFEST}}"
RDK_HALIF_AIDL_VERSION="${RDK_HALIF_AIDL_VERSION:-main}"
PREBUILT_AIDL_ROOT="${PREBUILT_AIDL_ROOT:-}"
LINUX_BINDER_DIR="${LINUX_BINDER_DIR:-${DEF_LINUX_BINDER}}"
BINDER_VERSION="${BINDER_VERSION:-}"
TARGET_ARCH="${TARGET_ARCH:-linux}"
LINUX_BINDER_OUT_DIR=""

AIDL_BIN="${AIDL_BIN:-}"

mkdir -p "${BUILD_DIR}"

resolve_manifest_file()
{
    if [[ "${AIDL_VERSION_MANIFEST}" = /* ]]; then
        echo "${AIDL_VERSION_MANIFEST}"
    else
        echo "${RDK_HAL_DIR}/${AIDL_VERSION_MANIFEST}"
    fi
}

manifest_component_version()
{
    local component="$1"
    local manifest_file
    local default_version

    manifest_file="$(resolve_manifest_file)"
    if [ ! -f "${manifest_file}" ]; then
        echo "current"
        return 0
    fi

    default_version="$(awk '/^default:/{print $2; exit}' "${manifest_file}")"
    default_version="${default_version:-current}"

    awk -v comp="${component}" -v def="${default_version}" '
        /^components:/      { inmap=1; next }
        inmap && /^[^[:space:]#]/ { inmap=0 }
        inmap && $1 == comp":" {
            print ($2 == "" ? def : $2)
            found=1
            exit
        }
        END { if (!found) print def }
    ' "${manifest_file}"
}

ensure_rdk_hal_repo()
{
    if [ ! -d "${RDK_HAL_DIR}" ]; then
        git clone -b "${RDK_HALIF_AIDL_VERSION}" https://github.com/rdkcentral/rdk-halif-aidl.git "${RDK_HAL_DIR}"
        return 0
    fi

    pushd "${RDK_HAL_DIR}" >/dev/null
    git fetch -q --tags origin
    git checkout "${RDK_HALIF_AIDL_VERSION}"
    popd >/dev/null
}

resolve_binder_version()
{
    if [ -n "${BINDER_VERSION}" ]; then
        echo "${BINDER_VERSION}"
        return 0
    fi

    local version_file="${RDK_HAL_DIR}/binder_sdk.version"
    if [ -f "${version_file}" ]; then
        local pinned
        pinned="$(grep -vE '^[[:space:]]*(#|$)' "${version_file}" | head -1 | tr -d '[:space:]')"
        if [ -n "${pinned}" ]; then
            echo "${pinned}"
            return 0
        fi
    fi

    echo "2.3.0"
}

ensure_linux_binder_repo()
{
    local target_ref="$1"

    if [ ! -d "${LINUX_BINDER_DIR}" ]; then
        git clone https://github.com/rdkcentral/linux_binder_idl.git "${LINUX_BINDER_DIR}"
    fi

    if [ ! -d "${LINUX_BINDER_DIR}/.git" ]; then
        echo -e "${BOLD}${RED}ERROR: ${LINUX_BINDER_DIR} exists but is not a git repository${RESET}"
        exit 1
    fi

    pushd "${LINUX_BINDER_DIR}" >/dev/null
    if [ -n "$(git status --porcelain)" ]; then
        echo -e "${YELLOW}Cleaning uncommitted changes in ${LINUX_BINDER_DIR}...${RESET}"
        git checkout . || {
            git clean -fd || true
        }
    fi

    git fetch -q --tags origin
    if ! git checkout "${target_ref}"; then
        echo -e "${BOLD}${RED}ERROR: unable to checkout linux_binder_idl ref ${target_ref}${RESET}"
        popd >/dev/null
        exit 1
    fi
    popd >/dev/null
}

build_linux_binder_target()
{
    local make_cmd

    if [ ! -x "${LINUX_BINDER_DIR}/build-linux-binder-aidl.sh" ]; then
        echo -e "${BOLD}${RED}ERROR: missing build script ${LINUX_BINDER_DIR}/build-linux-binder-aidl.sh${RESET}"
        exit 1
    fi

    if command -v gmake >/dev/null 2>&1; then
        make_cmd="$(command -v gmake)"
    elif command -v make >/dev/null 2>&1; then
        make_cmd="$(command -v make)"
    else
        echo -e "${BOLD}${RED}ERROR: neither gmake nor make was found on PATH${RESET}"
        exit 1
    fi

    echo -e "${BOLD}${CYAN}Building linux_binder_idl target artifacts (TARGET_ARCH=${TARGET_ARCH})...${RESET}"
    echo -e "${CYAN}Using make program for CMake: ${make_cmd}${RESET}"

    # Remove stale CMake state that can pin an invalid make binary (e.g. /usr/bin/gmake).
    rm -rf "${LINUX_BINDER_DIR}/build-host" "${LINUX_BINDER_DIR}/build-target"

    pushd "${LINUX_BINDER_DIR}" >/dev/null
    if [ "${TARGET_ARCH}" = "arm" ]; then
        if [ -z "${CC:-}" ] || [ -z "${CXX:-}" ]; then
            echo -e "${BOLD}${RED}ERROR: Target=arm specified but CC/CXX cross-compiler not set in environment${RESET}"
            echo "Please source the ARM toolchain setup script first."
            popd >/dev/null
            exit 1
        fi
        echo -e "${GREEN}Using ARM cross-compiler: CC=${CC} CXX=${CXX}${RESET}"
    fi

    # Some SDK environments force CMAKE_MAKE_PROGRAM=/usr/bin/gmake even when gmake is absent.
    # Override it here so host tool configure checks can always invoke an existing make binary.
        PATH="/usr/bin:/bin:${PATH}" \
    MAKE="${make_cmd}" \
    CMAKE_MAKE_PROGRAM="${make_cmd}" \
        ./build-linux-binder-aidl.sh no-host-aidl

        # Build host AIDL tools via upstream script, while preventing inherited invalid make settings.
        PATH="/usr/bin:/bin:${PATH}" \
        MAKE="${make_cmd}" \
        BUILD_TYPE="${BUILD_TYPE:-Release}" \
        env -u CMAKE_MAKE_PROGRAM ./build-aidl-generator-tool.sh
    popd >/dev/null

    LINUX_BINDER_OUT_DIR="${LINUX_BINDER_DIR}/out/target"
    if [ ! -d "${LINUX_BINDER_OUT_DIR}/lib" ] || [ ! -d "${LINUX_BINDER_OUT_DIR}/include" ]; then
        echo -e "${BOLD}${RED}ERROR: linux_binder_idl output not found at ${LINUX_BINDER_OUT_DIR}${RESET}"
        exit 1
    fi
}

resolve_aidl_bin()
{
    local candidate
    for candidate in \
        "${LINUX_BINDER_DIR}/out/host/bin/aidl" \
        "${LINUX_BINDER_DIR}/out/target/bin/aidl" \
        "${LINUX_BINDER_DIR}/out/target/bin/aidl-cpp" \
        "${LINUX_BINDER_DIR}/out/aidl" \
        "${LINUX_BINDER_DIR}/build-target/aidl" \
        "${LINUX_BINDER_DIR}/build-host/aidl" \
        "${RDK_HAL_DIR}/out/target/bin/aidl"; do
        if [ -x "${candidate}" ]; then
            AIDL_BIN="${candidate}"
            return 0
        fi
    done

    AIDL_BIN=""
    return 1
}

ensure_colorama_for_system_python()
{
    local sys_python="/usr/bin/python3"
    if [ ! -x "${sys_python}" ]; then
        sys_python="python3"
    fi

    if ! "${sys_python}" -c 'import colorama' >/dev/null 2>&1; then
        echo -e "${CYAN}Installing colorama for ${sys_python}...${RESET}"
        "${sys_python}" -m pip install --quiet --user colorama >/dev/null 2>&1 || true
    fi

    if ! "${sys_python}" -c 'import colorama' >/dev/null 2>&1; then
        echo -e "${BOLD}${RED}ERROR: colorama is not available for ${sys_python}${RESET}"
        echo "Install it with: ${sys_python} -m pip install --user colorama"
        exit 1
    fi
}

install_linux_binder()
{
    echo -e "${BOLD}${CYAN}Installing linux binder runtime and headers...${RESET}"
    install -d "${BUILD_INSTALL_PREFIX}/bin" "${BUILD_INSTALL_PREFIX}/lib" "${BUILD_INSTALL_PREFIX}/include"
    if [ -d "${LINUX_BINDER_OUT_DIR}/bin" ]; then
        install -t "${BUILD_INSTALL_PREFIX}/bin" "${LINUX_BINDER_OUT_DIR}/bin"/*
    fi
    install -t "${BUILD_INSTALL_PREFIX}/lib" "${LINUX_BINDER_OUT_DIR}/lib"/*
    cp -apr "${LINUX_BINDER_OUT_DIR}/include"/* "${BUILD_INSTALL_PREFIX}/include/"

    # Stage binder SDK where rdk-halif-aidl expects it for CMake-based interface builds.
    local rdk_binder_out="${RDK_HAL_DIR}/out/target"
    local rdk_build_out="${RDK_HAL_DIR}/out/build"
    install -d "${rdk_binder_out}/lib" "${rdk_binder_out}/lib/binder" "${rdk_binder_out}/include" "${rdk_binder_out}/bin"
    install -d "${rdk_build_out}/include/binder_sdk"

    cp -apr "${LINUX_BINDER_OUT_DIR}"/lib/* "${rdk_binder_out}/lib/" 2>/dev/null || true
    cp -apr "${LINUX_BINDER_OUT_DIR}"/lib/* "${rdk_binder_out}/lib/binder/" 2>/dev/null || true
    cp -apr "${LINUX_BINDER_OUT_DIR}"/include/* "${rdk_binder_out}/include/" 2>/dev/null || true
    cp -apr "${LINUX_BINDER_OUT_DIR}"/include/* "${rdk_build_out}/include/binder_sdk/" 2>/dev/null || true
    if [ -d "${LINUX_BINDER_OUT_DIR}/bin" ]; then
        cp -apr "${LINUX_BINDER_OUT_DIR}"/bin/* "${rdk_binder_out}/bin/" 2>/dev/null || true
    fi
    touch "${rdk_binder_out}/.sdk_ready"
}

stage_linux_binder_source_for_rdk_hal()
{
    local expected_root="${RDK_HAL_DIR}/build-tools"
    local expected_src="${expected_root}/linux_binder_idl"

    install -d "${expected_root}"
    ln -sfn "${LINUX_BINDER_DIR}" "${expected_src}"

    echo -e "${CYAN}Binder source for rdk-halif-aidl: ${expected_src} -> ${LINUX_BINDER_DIR}${RESET}"
}

prepare_aidl_binder_sdk()
{
    if [ ! -f "${RDK_HAL_DIR}/out/target/.sdk_ready" ]; then
        echo -e "${BOLD}${CYAN}Preparing Binder SDK for AIDL generation...${RESET}"
        (
            cd "${RDK_HAL_DIR}"
            BINDER_VERSION="${BINDER_VERSION}" \
            BINDER_SOURCE_DIR="${LINUX_BINDER_DIR}" \
            ./build_binder.sh
        )
    fi
}

find_host_cmake()
{
    local candidates=("cmake" "cmake3" "/usr/bin/cmake" "/usr/local/bin/cmake")
    local c
    for c in "${candidates[@]}"; do
        if command -v "${c}" &>/dev/null || [ -x "${c}" ]; then
            local ver
            ver=$(${c} --version 2>/dev/null | awk 'NR==1{print $3}')
            local major minor
            IFS='.' read -r major minor _ <<< "${ver}"
            if [ "${major:-0}" -gt 3 ] || { [ "${major:-0}" -eq 3 ] && [ "${minor:-0}" -ge 19 ]; }; then
                echo "${c}"
                return 0
            fi
        fi
    done
    echo "cmake"
}

generate_hal_interface()
{
    local interface_target="$1"
    local interface_version="$2"
    local build_subdir="$3"
    local host_cmake="$4"

    local hal_build_dir="${BUILD_DIR}/${build_subdir}"
    local gen_dir="${BUILD_DIR}/${interface_target}_gen/${interface_version}"
    local interface_roots="${RDK_HAL_DIR}"

    rm -rf "${hal_build_dir}"

    "${host_cmake}" \
        -DAIDL_BIN="${AIDL_BIN}" \
        -DAIDL_GEN_DIR="${gen_dir}" \
        -DAIDL_SRC_VERSION="${interface_version}" \
        -DINTERFACE_TARGET="${interface_target}" \
        -DINTERFACES_ROOT_DIRS="${interface_roots}" \
        -DBINDER_SDK_DIR="${RDK_HAL_DIR}/out/target" \
        -DBINDER_SDK_INCLUDE_DIR="${RDK_HAL_DIR}/out/build" \
        -DLINUX_BINDER_AIDL_ROOT="${RDK_HAL_DIR}/build-tools/linux_binder_idl" \
        -DHOST_AIDL_DIR="${RDK_HAL_DIR}/build-tools/linux_binder_idl/host" \
        -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
        -B "${hal_build_dir}" \
        "${RDK_HAL_DIR}"
}

stage_generated_aidl_libs()
{
    local plane_ver video_ver common_ver
    local plane_lib video_lib common_lib
    local interface_headers_dir
    local hal_lib_dir

    hal_lib_dir="${RDK_HAL_DIR}/out/target/lib/halif"

    plane_ver="$(manifest_component_version planecontrol)"
    video_ver="$(manifest_component_version videodecoder)"
    common_ver="$(manifest_component_version common)"

    plane_lib=$(find "${hal_lib_dir}" -type f -name "libplanecontrol-vcurrent-cpp.so" 2>/dev/null | head -1)
    [ -n "${plane_lib}" ] || plane_lib=$(find "${hal_lib_dir}" -type f -name "libplanecontrol-v${plane_ver}-cpp.so" 2>/dev/null | head -1)
    [ -n "${plane_lib}" ] || plane_lib=$(find "${hal_lib_dir}" -type f -name "libplanecontrol-v*-cpp.so" 2>/dev/null | head -1)

    video_lib=$(find "${hal_lib_dir}" -type f -name "libvideodecoder-vcurrent-cpp.so" 2>/dev/null | head -1)
    [ -n "${video_lib}" ] || video_lib=$(find "${hal_lib_dir}" -type f -name "libvideodecoder-v${video_ver}-cpp.so" 2>/dev/null | head -1)
    [ -n "${video_lib}" ] || video_lib=$(find "${hal_lib_dir}" -type f -name "libvideodecoder-v*-cpp.so" 2>/dev/null | head -1)

    common_lib=$(find "${BUILD_DIR}" -type f -path "*_aidl/*" -name "libcommon-vcurrent-cpp.so" 2>/dev/null | head -1)
    [ -n "${common_lib}" ] || common_lib=$(find "${hal_lib_dir}" -type f -name "libcommon-vcurrent-cpp.so" 2>/dev/null | head -1)
    [ -n "${common_lib}" ] || common_lib=$(find "${BUILD_DIR}" -type f -path "*_aidl/*" -name "libcommon-v${common_ver}-cpp.so" 2>/dev/null | head -1)
    [ -n "${common_lib}" ] || common_lib=$(find "${BUILD_DIR}" -type f -path "*_aidl/*" -name "libcommon-v*-cpp.so" 2>/dev/null | head -1)

    if [ -z "${plane_lib}" ] || [ -z "${video_lib}" ] || [ -z "${common_lib}" ]; then
        echo -e "${BOLD}${RED}ERROR: required AIDL libs were not found after generation${RESET}"
        exit 1
    fi

    cp -f "${plane_lib}" "${BUILD_INSTALL_PREFIX}/lib/"
    cp -f "${video_lib}" "${BUILD_INSTALL_PREFIX}/lib/"
    cp -f "${common_lib}" "${BUILD_INSTALL_PREFIX}/lib/"

    install -d "${BUILD_INSTALL_PREFIX}/include"
    for interface_headers_dir in \
        "${RDK_HAL_DIR}/common/current/include" \
        "${RDK_HAL_DIR}/out/build/include/planecontrol/current/include" \
        "${RDK_HAL_DIR}/out/build/include/videodecoder/current/include"; do
        if [ -d "${interface_headers_dir}" ]; then
            cp -apr "${interface_headers_dir}"/* "${BUILD_INSTALL_PREFIX}/include/"
        fi
    done

    if [ "$(basename "${plane_lib}")" != "libplanecontrol-vcurrent-cpp.so" ]; then
        ln -sf "$(basename "${plane_lib}")" "${BUILD_INSTALL_PREFIX}/lib/libplanecontrol-vcurrent-cpp.so"
    fi
    if [ "$(basename "${video_lib}")" != "libvideodecoder-vcurrent-cpp.so" ]; then
        ln -sf "$(basename "${video_lib}")" "${BUILD_INSTALL_PREFIX}/lib/libvideodecoder-vcurrent-cpp.so"
    fi
    if [ "$(basename "${common_lib}")" != "libcommon-vcurrent-cpp.so" ]; then
        ln -sf "$(basename "${common_lib}")" "${BUILD_INSTALL_PREFIX}/lib/libcommon-vcurrent-cpp.so"
    fi
    ln -sf "libplanecontrol-vcurrent-cpp.so" "${BUILD_INSTALL_PREFIX}/lib/libplanecontrolaidl.so"
    ln -sf "libvideodecoder-vcurrent-cpp.so" "${BUILD_INSTALL_PREFIX}/lib/libvideodecoderaidl.so"
}

stage_prebuilt_aidl_artifacts()
{
    local prebuilt_root prebuilt_include prebuilt_lib
    local plane_ver video_ver common_ver
    local plane_lib video_lib common_lib

    if [ -z "${PREBUILT_AIDL_ROOT}" ]; then
        return 1
    fi

    prebuilt_root="$(realpath "${PREBUILT_AIDL_ROOT}" 2>/dev/null || true)"
    prebuilt_include="${prebuilt_root}/usr/include"
    prebuilt_lib="${prebuilt_root}/usr/lib"

    if [ ! -d "${prebuilt_include}/com/rdk/hal/planecontrol" ] || [ ! -d "${prebuilt_include}/com/rdk/hal/videodecoder" ] || [ ! -d "${prebuilt_lib}" ]; then
        return 1
    fi

    plane_ver="$(manifest_component_version planecontrol)"
    video_ver="$(manifest_component_version videodecoder)"
    common_ver="$(manifest_component_version common)"

    plane_lib="libplanecontrol-v${plane_ver}-cpp.so"
    video_lib="libvideodecoder-v${video_ver}-cpp.so"
    common_lib="libcommon-v${common_ver}-cpp.so"

    [ -f "${prebuilt_lib}/${plane_lib}" ] || return 1
    [ -f "${prebuilt_lib}/${video_lib}" ] || return 1
    [ -f "${prebuilt_lib}/${common_lib}" ] || return 1

    cp -apr "${prebuilt_include}/com" "${BUILD_INSTALL_PREFIX}/include/"
    cp -f "${prebuilt_lib}/${plane_lib}" "${BUILD_INSTALL_PREFIX}/lib/"
    cp -f "${prebuilt_lib}/${video_lib}" "${BUILD_INSTALL_PREFIX}/lib/"
    cp -f "${prebuilt_lib}/${common_lib}" "${BUILD_INSTALL_PREFIX}/lib/"

    ln -sf "${plane_lib}" "${BUILD_INSTALL_PREFIX}/lib/libplanecontrol-vcurrent-cpp.so"
    ln -sf "${video_lib}" "${BUILD_INSTALL_PREFIX}/lib/libvideodecoder-vcurrent-cpp.so"
    ln -sf "${common_lib}" "${BUILD_INSTALL_PREFIX}/lib/libcommon-vcurrent-cpp.so"
    ln -sf "${plane_lib}" "${BUILD_INSTALL_PREFIX}/lib/libplanecontrolaidl.so"
    ln -sf "${video_lib}" "${BUILD_INSTALL_PREFIX}/lib/libvideodecoderaidl.so"

    return 0
}

build_mockservice()
{
    make -C "${MOCK_SERVICE_DIR}" clean || true
    make -j"$(nproc)" -C "${MOCK_SERVICE_DIR}"
}

validate_and_bundle_artifacts()
{
    local required
    for required in \
        "${BUILD_DIR}/bin/EmulatePlaneControlService" \
        "${BUILD_INSTALL_PREFIX}/lib/libplanecontrolaidl.so"; do
        if [ ! -e "${required}" ]; then
            echo -e "${BOLD}${RED}Mock-only build failed: missing artifact ${required}${RESET}"
            exit 1
        fi
    done

    mkdir -p "${BUILD_DIR}/mock_bundle"
    cp -f "${BUILD_DIR}/bin/EmulatePlaneControlService" "${BUILD_DIR}/mock_bundle/"
    cp -f "${BUILD_INSTALL_PREFIX}/lib/libplanecontrolaidl.so" "${BUILD_DIR}/mock_bundle/"
    [ -f "${BUILD_INSTALL_PREFIX}/lib/libvideodecoderaidl.so" ] && cp -f "${BUILD_INSTALL_PREFIX}/lib/libvideodecoderaidl.so" "${BUILD_DIR}/mock_bundle/"

    echo -e "${BOLD}${GREEN}Mock-only build completed. Artifacts: ${BUILD_DIR}/mock_bundle${RESET}"
}

build_mock_only()
{
    BINDER_VERSION="$(resolve_binder_version)"
    echo -e "${BOLD}${GREEN}RDK_HALIF_AIDL_VERSION : ${RESET}${YELLOW}${RDK_HALIF_AIDL_VERSION}${RESET}"
    echo -e "${BOLD}${GREEN}BINDER_VERSION         : ${RESET}${YELLOW}${BINDER_VERSION}${RESET}"
    echo -e "${BOLD}${GREEN}TARGET_ARCH            : ${RESET}${YELLOW}${TARGET_ARCH}${RESET}"
    echo -e "${BOLD}${GREEN}AIDL_MANIFEST          : ${RESET}${YELLOW}$(resolve_manifest_file)${RESET}"

    ensure_rdk_hal_repo
    ensure_linux_binder_repo "${BINDER_VERSION}"
    build_linux_binder_target

    install_linux_binder
    stage_linux_binder_source_for_rdk_hal

    if ! stage_prebuilt_aidl_artifacts; then
        if ! resolve_aidl_bin; then
            echo -e "${BOLD}${RED}ERROR: aidl binary not found after linux_binder build${RESET}"
            exit 1
        fi

        ensure_colorama_for_system_python
        prepare_aidl_binder_sdk

        local host_cmake
        host_cmake="$(find_host_cmake)"

        local plane_ver video_ver
        plane_ver="$(manifest_component_version planecontrol)"
        video_ver="$(manifest_component_version videodecoder)"

        generate_hal_interface "planecontrol" "${plane_ver}" "planecontrol_aidl" "${host_cmake}"
        generate_hal_interface "videodecoder" "${video_ver}" "videodecoder_aidl" "${host_cmake}"

        "${host_cmake}" --build "${BUILD_DIR}/planecontrol_aidl" -j"$(nproc)"
        "${host_cmake}" --build "${BUILD_DIR}/videodecoder_aidl" -j"$(nproc)"

        stage_generated_aidl_libs
    fi

    build_mockservice
    validate_and_bundle_artifacts
}

clean_mock()
{
    make -C "${MOCK_SERVICE_DIR}" clean || true
    rm -rf "${BUILD_DIR}/mock_bundle"
}

help()
{
    echo -e "${BOLD}${CYAN}Usage:${RESET}"
    echo "  $0                Build mock service and required artifacts"
    echo "  $0 Target=arm     Build mock service for ARM (requires CC/CXX set)"
    echo "  $0 clean          Clean mock service outputs"
    echo "  $0 help|-h|--help Show this message"
    echo
    echo "Environment overrides:"
    echo "  RDK_HALIF_AIDL_VERSION (default: main)"
    echo "  AIDL_VERSION_MANIFEST (default: versions_released.yaml)"
    echo "  BINDER_VERSION (default: binder_sdk.version pin)"
    echo "  LINUX_BINDER_DIR (default: ${DEF_LINUX_BINDER})"
    echo "  PREBUILT_AIDL_ROOT (optional prebuilt root with usr/include + usr/lib)"
    echo "  TARGET_ARCH (default: linux, set via Target= parameter)"
}

ACTION="build"
TARGET_ARG=""

for arg in "$@"; do
    case "${arg}" in
        help|-h|--help)
            ACTION="help"
            ;;
        clean)
            ACTION="clean"
            ;;
        TARGET=*|Target=*)
            TARGET_ARG="${arg#*=}"
            TARGET_ARCH="${TARGET_ARG}"
            ;;
        "")
            ;;
        *)
            echo -e "${BOLD}${RED}Invalid argument: ${arg}${RESET}"
            help
            exit 1
            ;;
    esac
done

if [ -n "${TARGET_ARG}" ]; then
    echo -e "${BOLD}${YELLOW}Note:${RESET} TARGET=${TARGET_ARG} accepted for compatibility; mock build uses current toolchain/environment settings."
fi

case "${ACTION}" in
    help)
        help
        ;;
    clean)
        clean_mock
        ;;
    build)
        build_mock_only
        ;;
esac
