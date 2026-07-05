#* ******************************************************************************
#*  If not stated otherwise in this file or this component's LICENSE
#*  file the following copyright and licenses apply:
#*
#*  Copyright 2025 RDK Management
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

.PHONY: all clean check-libs

PLANECONTROL_AIDL_LIB_NAME := planecontrolaidl
EMULATE_SERVICE_BIN_NAME := EmulatePlaneControlService

BUILD_DIR ?= $(CURDIR)/../build

# Use cross-compiler if available (set by toolchain), otherwise use native.
CXX ?= g++
CXXFLAGS ?= -Wall -Wextra -std=c++17 -ggdb
EMULATE_SERVICE_CXXFLAGS := $(CXXFLAGS) -Wall -Wextra -std=c++17 -ggdb

EMULATE_SERVICE_DIR ?= $(CURDIR)/src

EMULATE_SERVICE_BIN_SRC := $(EMULATE_SERVICE_DIR)/PlaneControlService.cpp
EMULATE_SERVICE_BIN_SRC += $(EMULATE_SERVICE_DIR)/PlaneControl.cpp

# Use headers from CMake-generated locations in rdk-halif-aidl
RDK_HAL_DIR ?= $(BUILD_DIR)/../rdk-halif-aidl
COMMON_HEADERS ?= $(RDK_HAL_DIR)/common/current/include
PLANECONTROL_HEADERS ?= $(RDK_HAL_DIR)/out/build/include/planecontrol/current/include
VIDEODECODER_HEADERS ?= $(RDK_HAL_DIR)/out/build/include/videodecoder/current/include
BINDER_SDK_HEADERS ?= $(RDK_HAL_DIR)/out/build/include/binder_sdk

EMULATE_SERVICE_INC_FLAGS := -I$(COMMON_HEADERS)
EMULATE_SERVICE_INC_FLAGS += -I$(PLANECONTROL_HEADERS)
EMULATE_SERVICE_INC_FLAGS += -I$(VIDEODECODER_HEADERS)
EMULATE_SERVICE_INC_FLAGS += -I$(BINDER_SDK_HEADERS)
EMULATE_SERVICE_INC_FLAGS += -I$(BUILD_DIR)/usr/include

EMULATE_SERVICE_LD_LIBS := -lbinder -lutils -lbase

EMULATE_SERVICE_LDFLAGS := -L$(BUILD_DIR)/usr/lib
EMULATE_SERVICE_LDFLAGS += -Wl,-rpath-link=$(BUILD_DIR)/usr/lib
EMULATE_SERVICE_LDFLAGS += -Wl,--copy-dt-needed-entries

EMULATE_SERVICE_BIN := $(BUILD_DIR)/bin/$(EMULATE_SERVICE_BIN_NAME)

all: $(EMULATE_SERVICE_BIN)

check-libs:
	@echo "Checking required runtime libraries in $(BUILD_DIR)/usr/lib"
	@test -e "$(BUILD_DIR)/usr/lib/libplanecontrol-vcurrent-cpp.so" || { echo "ERROR: missing $(BUILD_DIR)/usr/lib/libplanecontrol-vcurrent-cpp.so"; exit 1; }
	@test -e "$(BUILD_DIR)/usr/lib/libvideodecoder-vcurrent-cpp.so" || { echo "ERROR: missing $(BUILD_DIR)/usr/lib/libvideodecoder-vcurrent-cpp.so"; exit 1; }
	@test -e "$(BUILD_DIR)/usr/lib/libcommon-vcurrent-cpp.so" || { echo "ERROR: missing $(BUILD_DIR)/usr/lib/libcommon-vcurrent-cpp.so"; exit 1; }
	@test -e "$(BUILD_DIR)/usr/lib/libbinder.so" || { echo "ERROR: missing $(BUILD_DIR)/usr/lib/libbinder.so"; exit 1; }
	@test -e "$(BUILD_DIR)/usr/lib/libutils.so" || { echo "ERROR: missing $(BUILD_DIR)/usr/lib/libutils.so"; exit 1; }
	@test -e "$(BUILD_DIR)/usr/lib/libbase.so" || { echo "ERROR: missing $(BUILD_DIR)/usr/lib/libbase.so"; exit 1; }
	@echo "Library check passed"

$(EMULATE_SERVICE_BIN): check-libs $(EMULATE_SERVICE_BIN_SRC:%.cpp=%.o)
	@mkdir -p $(dir $@)
	$(CXX) $(EMULATE_SERVICE_CXXFLAGS) $(EMULATE_SERVICE_LDFLAGS) \
		$(EMULATE_SERVICE_BIN_SRC:%.cpp=%.o) -o $@ \
		$(EMULATE_SERVICE_LD_LIBS) \
		-lpthread \
		-lplanecontrol-vcurrent-cpp \
		-lvideodecoder-vcurrent-cpp \
		-lcommon-vcurrent-cpp

%.o: %.cpp
	$(CXX) $(EMULATE_SERVICE_CXXFLAGS) $(EMULATE_SERVICE_INC_FLAGS) -fPIC -Wno-error -w -c $< -o $@

clean:
	rm -f $(EMULATE_SERVICE_BIN_SRC:%.cpp=%.o)
	rm -f $(EMULATE_SERVICE_BIN)
