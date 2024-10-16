cmake_minimum_required(VERSION 3.8)

# Set the project name and version
project(PlaneControlService VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Linux Binder Dependencies
if (NOT DEFINED LINUX_BINDER_OUT_DIR)
    message(STATUS "Setting LINUX_BINDER_OUT_DIR to default value")
    set(LINUX_BINDER_OUT_DIR "${CMAKE_SOURCE_DIR}/../../linux_binder_idl/local")
endif()
message(STATUS "LINUX_BINDER_OUT_DIR set to ${LINUX_BINDER_OUT_DIR}")

set(LINUX_BINDER_BIN_DIR "${LINUX_BINDER_OUT_DIR}/bin/")
message(STATUS "LINUX_BINDER_BIN_DIR set to ${LINUX_BINDER_BIN_DIR}")

set(LINUX_BINDER_LIB_DIR "${LINUX_BINDER_OUT_DIR}/lib/")
message(STATUS "LINUX_BINDER_LIB_DIR set to ${LINUX_BINDER_LIB_DIR}")

set(LINUX_BINDER_INCLUDE_DIR "${LINUX_BINDER_OUT_DIR}/include/")
message(STATUS "LINUX_BINDER_INCLUDE_DIR set to ${LINUX_BINDER_INCLUDE_DIR}")

# AIDL Interface Dependencies
if (NOT DEFINED AIDL_PLANECONTROL_GEN_DIR)
	message(STATUS "Setting AIDL_PLANECONTROL_GEN_DIR to default value")
	set(AIDL_PLANECONTROL_GEN_DIR "${CMAKE_SOURCE_DIR}/../../vdevice/aidl/hal/planecontrol")
endif()
message(STATUS "AIDL_PLANECONTROL_GEN_DIR set to ${AIDL_PLANECONTROL_GEN_DIR}")

#AIDL Version
if (NOT DEFINED PLANECONTROL_AIDL_VERSION)
	message(STATUS "Setting PLANECONTROL_AIDL_VERSION to current latest")
        set(PLANECONTROL_AIDL_VERSION "current")
endif()
message(STATUS "PLANECONTROL_AIDL_VERSION set to ${PLANECONTROL_AIDL_VERSION}")

# PlaneControlService Installation Directories
if (NOT DEFINED CMAKE_INSTALL_LIBDIR)
    message("Set and create cmake install lib directory")
    set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
    file(MAKE_DIRECTORY ${CMAKE_INSTALL_LIBDIR})
    message("cmake install lib dir ${CMAKE_INSTALL_LIBDIR}")
endif ()

if (NOT DEFINED CMAKE_INSTALL_BINDIR)
    message("Set and create cmake install bin directory")
    set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/bin)
    file(MAKE_DIRECTORY ${CMAKE_INSTALL_BINDIR})
    message("cmake install bin dir ${CMAKE_INSTALL_BINDIR}")
endif ()

#############################################################
# MockPlaneService Build
#############################################################
# Set Service Directory
set(PLANE_CONTROL_SERVICE ${CMAKE_SOURCE_DIR})
#Set Source files path
set(PLANECONTROL_SRC_PATH ${AIDL_PLANECONTROL_GEN_DIR}/gen/${PLANECONTROL_AIDL_VERSION}/cpp/com/rdk/hal/planecontrol)
# Set AIDL Source files
set(PLANECONTROL_AIDL_SRCS
	${PLANECONTROL_SRC_PATH}/AspectRatio.cpp
	${PLANECONTROL_SRC_PATH}/IPlaneControl.cpp
	${PLANECONTROL_SRC_PATH}/IPlaneControlListener.cpp
	${PLANECONTROL_SRC_PATH}/PlaneType.cpp
	${PLANECONTROL_SRC_PATH}/Capabilities.cpp
	${PLANECONTROL_SRC_PATH}/Property.cpp
	${PLANECONTROL_SRC_PATH}/PropertyKVPair.cpp
	${PLANECONTROL_SRC_PATH}/SourcePlaneMapping.cpp
	${PLANECONTROL_SRC_PATH}/SourceType.cpp
	${PLANECONTROL_SRC_PATH}/../PropertyValue.cpp
)

# Add yaml-cpp
#Need to include yaml-cpp in vdevice-manifest and add the corresponding path
set(YAML_CPP_DIR "~/rdk-halif-binder-test-planecontrol/output/lib/cmake/yaml-cpp")
find_package(yaml-cpp REQUIRED PATHS ${YAML_CPP_DIR})

add_library(mplanecontrol SHARED
    ${PLANECONTROL_AIDL_SRCS}
    ${PLANE_CONTROL_SERVICE}/PlaneControl.cpp
)
target_compile_options(mplanecontrol PRIVATE
    -Wall -Wextra -std=c++17
)
target_include_directories(mplanecontrol PRIVATE
    ${AIDL_PLANECONTROL_GEN_DIR}/gen/${PLANECONTROL_AIDL_VERSION}/h
    ${LINUX_BINDER_INCLUDE_DIR}
    ${YAML_CPP_INCLUDE_DIRS}  # Include yaml-cpp headers
)
# Add the library directory
link_directories(${LINUX_BINDER_LIB_DIR})
find_library(BINDER_LIB  binder PATHS ${LINUX_BINDER_LIB_DIR})
find_library(UTILS_LIB utils PATHS ${LINUX_BINDER_LIB_DIR})

target_link_libraries(mplanecontrol PRIVATE ${BINDER_LIB} ${UTILS_LIB} ${YAML_CPP_LIBRARIES})

# Add Service Executable
add_executable(PlaneControLService
        ${PLANE_CONTROL_SERVICE}/PlaneControlService.cpp
)
target_compile_options(PlaneControLService PRIVATE
    -Wall -Wextra -std=c++17
)
target_include_directories(PlaneControLService PRIVATE
    ${AIDL_PLANECONTROL_GEN_DIR}/gen/current/h
    ${LINUX_BINDER_INCLUDE_DIR}
    ${YAML_CPP_INCLUDE_DIRS}  # Include yaml-cpp headers
)
target_link_libraries(PlaneControLService PRIVATE ${BINDER_LIB} ${UTILS_LIB} ${YAML_CPP_LIBRARIES} mplanecontrol)
###########################################################
# Install the library
###########################################################
install(
    TARGETS
    mplanecontrol

    LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
###########################################################
# Install the binary
###########################################################
install(
    TARGETS
    PlaneControLService

    RUNTIME
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

