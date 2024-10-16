# Minimum required version of CMake
cmake_minimum_required(VERSION 3.8)

# Set the project name and version
project(PlaneControl VERSION 1.0)

# Set C++ standard for the entire project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Global installation prefix, default to 'output' directory in the root
if (NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/output")
endif()
message(STATUS "Install prefix set to ${CMAKE_INSTALL_PREFIX}")

# Global Linux Binder directory, to be used by all sub-projects
if (NOT DEFINED LINUX_BINDER_OUT_DIR)
    message(STATUS "Setting LINUX_BINDER_OUT_DIR to default value")
    set(LINUX_BINDER_OUT_DIR "${CMAKE_SOURCE_DIR}/../rdk-halif-binder/local")
endif()
message(STATUS "LINUX_BINDER_OUT_DIR set to ${LINUX_BINDER_OUT_DIR}")

# AIDL Interface Dependencies
if (NOT DEFINED AIDL_PLANECONTROL_GEN_DIR)
        message(STATUS "Setting AIDL_PLANECONTROL_GEN_DIR to default value")
        set(AIDL_PLANECONTROL_GEN_DIR "${CMAKE_SOURCE_DIR}/hal/planecontrol")
endif()
message(STATUS "AIDL_PLANECONTROL_GEN_DIR set to ${AIDL_PLANECONTROL_GEN_DIR}")

#AIDL Version
if (NOT DEFINED PLANECONTROL_AIDL_VERSION)
        message(STATUS "Setting PLANECONTROL_AIDL_VERSION to current latest")
        set(PLANECONTROL_AIDL_VERSION "current")
endif()
message(STATUS "PLANECONTROL_AIDL_VERSION set to ${PLANECONTROL_AIDL_VERSION}")


# Add subdirectories
# These will invoke the CMakeLists.txt inside MockService and L1Test directories
add_subdirectory(MockService)
add_subdirectory(L1Test)


message(STATUS "PlaneControl project setup is complete.")

