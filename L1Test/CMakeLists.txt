cmake_minimum_required(VERSION 3.8)

# Set the project name and version
project(PlaneControlClient VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# Linux Binder Dependencies
if (NOT DEFINED LINUX_BINDER_OUT_DIR)
    message(STATUS "Setting LINUX_BINDER_OUT_DIR to default value")
    set(LINUX_BINDER_OUT_DIR "${CMAKE_SOURCE_DIR}/../../rdk-halif-binder/local")
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
    set(PLANE_CONTROL_GEN_DIR "${CMAKE_SOURCE_DIR}/../hal/planecontrol")
endif()
message(STATUS "AIDL_PLANECONTROL_GEN_DIR set to ${AIDL_PLANECONTROL_GEN_DIR}")

if (NOT DEFINED PLANECONTROL_AIDL_VERSION)
	message(STATUS "Setting PLANECONTROL_AIDL_VERSION to current latest")
	set(PLANECONTROL_AIDL_VERSION "current")
endif()
message(STATUS "PLANECONTROL_AIDL_VERSION set to ${PLANECONTROL_AIDL_VERSION}")

# MockPlaneControlClient Installation Directories
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
# MockPlaneControlClient Build
#############################################################

# Set Service Directory
set(PLANE_CONTROL_CLIENT ${CMAKE_SOURCE_DIR})
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
# Set the path to GTest configuration and targets
set(GTEST_PATH_DIR "~/vdevice/output/build/gtest/lib/cmake/GTest")

# Set the path to GTest include directory
set(GTEST_INCLUDE_DIR "~/vdevice/output/build/gtest/include")

# Manually specify the GTest library paths
set(GTEST_LIBRARY_DIR "~/vdevice/output/build/gtest/lib")
set(GTEST_LIBRARIES
    "${GTEST_LIBRARY_DIR}/libgtest.a"
    "${GTEST_LIBRARY_DIR}/libgtest_main.a"
)

# Find the GTest package
find_package(GTest REQUIRED PATHS ${GTEST_PATH_DIR})

## Add Service Executable
add_executable(PlaneControlClient
        ${PLANECONTROL_AIDL_SRCS}
        ${PLANE_CONTROL_CLIENT}/PlaneControlClient.cpp
)

target_compile_options(PlaneControlClient PRIVATE
    -Wall -Wextra -std=c++17
)

target_include_directories(PlaneControlClient PRIVATE
     ${AIDL_PLANECONTROL_GEN_DIR}/gen/${PLANECONTROL_AIDL_VERSION}/h
    ${LINUX_BINDER_INCLUDE_DIR}
    ${GTEST_INCLUDE_DIR}

)

# Add the library directory
link_directories(${LINUX_BINDER_LIB_DIR})
find_library(BINDER_LIB  binder PATHS ${LINUX_BINDER_LIB_DIR})
find_library(UTILS_LIB utils PATHS ${LINUX_BINDER_LIB_DIR})

target_link_libraries(PlaneControlClient PRIVATE ${BINDER_LIB} ${UTILS_LIB} ${GTEST_LIBRARIES})


###########################################################
# Install the binary
###########################################################
install(
    TARGETS
    PlaneControlClient

    RUNTIME
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

