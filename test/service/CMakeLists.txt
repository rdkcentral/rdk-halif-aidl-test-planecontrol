cmake_minimum_required(VERSION 3.8)

# Set the project name and version
project(PlaneControlService VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Linux Binder Dependencies
if (NOT DEFINED LINUX_BINDER_OUT_DIR)
    message(STATUS "Setting LINUX_BINDER_OUT_DIR to default value")
    set(LINUX_BINDER_OUT_DIR "${CMAKE_SOURCE_DIR}/../../../rdk-halif-binder/local")
endif()
message(STATUS "LINUX_BINDER_OUT_DIR set to ${LINUX_BINDER_OUT_DIR}")

set(LINUX_BINDER_BIN_DIR "${LINUX_BINDER_OUT_DIR}/bin/")
message(STATUS "LINUX_BINDER_BIN_DIR set to ${LINUX_BINDER_BIN_DIR}")

set(LINUX_BINDER_LIB_DIR "${LINUX_BINDER_OUT_DIR}/lib/")
message(STATUS "LINUX_BINDER_LIB_DIR set to ${LINUX_BINDER_LIB_DIR}")

set(LINUX_BINDER_INCLUDE_DIR "${LINUX_BINDER_OUT_DIR}/include/")
message(STATUS "LINUX_BINDER_INCLUDE_DIR set to ${LINUX_BINDER_INCLUDE_DIR}")

# AIDL Interface Dependencies
if (NOT DEFINED AIDL_EXAMPLE_GEN_DIR)
    message(STATUS "Setting AIDL_EXAMPLE_GEN_DIR to default value")
    set(AIDL_EXAMPLE_GEN_DIR "${CMAKE_SOURCE_DIR}/../..")
endif()
message(STATUS "AIDL_EXAMPLE_GEN_DIR set to ${AIDL_EXAMPLE_GEN_DIR}")

if (NOT DEFINED PLANECONTROL_AIDL_VERSION)
        message(STATUS "Setting PLANECONTROL_AIDL_VERSION to current latest")
        set(PLANECONTROL_AIDL_VERSION "${AIDL_EXAMPLE_GEN_DIR}/current")
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

# Set AIDL Source files
set(PLANECONTROL_AIDL_SRCS
    ${AIDL_EXAMPLE_GEN_DIR}/test/gen/current/cpp/com/rdk/hal/planecontrol/AspectRatio.cpp
    ${AIDL_EXAMPLE_GEN_DIR}/test/gen/current/cpp/com/rdk/hal/planecontrol/IPlaneControl.cpp
    ${AIDL_EXAMPLE_GEN_DIR}/test/gen/current/cpp/com/rdk/hal/planecontrol/IPlaneControlListener.cpp
    ${AIDL_EXAMPLE_GEN_DIR}/test/gen/current/cpp/com/rdk/hal/planecontrol/PlaneResource.cpp
    ${AIDL_EXAMPLE_GEN_DIR}/test/gen/current/cpp/com/rdk/hal/planecontrol/Property.cpp
    ${AIDL_EXAMPLE_GEN_DIR}/test/gen/current/cpp/com/rdk/hal/planecontrol/PropertyKVPair.cpp
    ${AIDL_EXAMPLE_GEN_DIR}/test/gen/current/cpp/com/rdk/hal/planecontrol/SourcePlaneMapping.cpp
    ${AIDL_EXAMPLE_GEN_DIR}/test/gen/current/cpp/com/rdk/hal/planecontrol/SourceType.cpp
    ${AIDL_EXAMPLE_GEN_DIR}/hal/common/output/cpp/com/rdk/hal/PropertyValue.cpp
)

add_library(mplanecontrol SHARED
    ${PLANECONTROL_AIDL_SRCS}
    ${PLANE_CONTROL_SERVICE}/PlaneControl.cpp
)

target_compile_options(mplanecontrol PRIVATE
    -Wall -Wextra -std=c++17
)

target_include_directories(mplanecontrol PRIVATE
    ${AIDL_EXAMPLE_GEN_DIR}/test/gen/current/h
    ${AIDL_EXAMPLE_GEN_DIR}/hal/common/output/h
    ${AIDL_EXAMPLE_GEN_DIR}/hal/videodecoder/gen/current/h
    ${LINUX_BINDER_INCLUDE_DIR}

)

# Add the library directory
link_directories(${LINUX_BINDER_LIB_DIR})
find_library(BINDER_LIB  binder PATHS ${LINUX_BINDER_LIB_DIR})
find_library(UTILS_LIB utils PATHS ${LINUX_BINDER_LIB_DIR})

target_link_libraries(mplanecontrol PRIVATE ${BINDER_LIB} ${UTILS_LIB})

# Add Service Executable
add_executable(PlaneControLService
        ${PLANE_CONTROL_SERVICE}/PlaneControlService.cpp
)

target_compile_options(PlaneControLService PRIVATE
    -Wall -Wextra -std=c++17
)

target_include_directories(PlaneControLService PRIVATE
    ${AIDL_EXAMPLE_GEN_DIR}/test/gen/current/h
    ${AIDL_EXAMPLE_GEN_DIR}/hal/common/output/h
    ${AIDL_EXAMPLE_GEN_DIR}/hal/videodecoder/gen/current/h
    ${LINUX_BINDER_INCLUDE_DIR}
)

target_link_libraries(PlaneControLService PRIVATE ${BINDER_LIB} ${UTILS_LIB} mplanecontrol)

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

