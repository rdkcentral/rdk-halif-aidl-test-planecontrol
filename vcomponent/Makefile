.PHONY: all

HAL_DIR ?= $(CURDIR)

PLANECONTROL_AIDL_LIB_NAME := planecontrolaidl

EMULATE_SERVICE_BIN := EmulatePlaneControlService
PLANECONTROL_AIDL_LIB := lib$(PLANECONTROL_AIDL_LIB_NAME).so

BUILD_DIR ?= $(CURDIR)/../build

AIDL_GEN_DIR ?= $(BUILD_DIR)/planecontrol_gen/current

EMULATE_SERVICE_DIR ?= $(CURDIR)/src

PLANECONTROL_AIDL_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/PropertyValue.cpp
PLANECONTROL_AIDL_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/planecontrol/PropertyKVPair.cpp
PLANECONTROL_AIDL_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/planecontrol/PlaneType.cpp
PLANECONTROL_AIDL_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/planecontrol/AspectRatio.cpp
PLANECONTROL_AIDL_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/planecontrol/Property.cpp
PLANECONTROL_AIDL_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/planecontrol/SourceType.cpp
PLANECONTROL_AIDL_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/planecontrol/SourcePlaneMapping.cpp
PLANECONTROL_AIDL_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/planecontrol/IPlaneControl.cpp
PLANECONTROL_AIDL_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/planecontrol/IPlaneControlListener.cpp
PLANECONTROL_AIDL_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/planecontrol/Capabilities.cpp

EMULATE_SERVICE_BIN_SRC := $(EMULATE_SERVICE_DIR)/PlaneControlService.cpp
EMULATE_SERVICE_BIN_SRC += $(EMULATE_SERVICE_DIR)/PlaneControl.cpp


EMULATE_SERVICE_INC_FLAGS := -I$(AIDL_GEN_DIR)/h
EMULATE_SERVICE_INC_FLAGS += -I$(BUILD_DIR)/usr/include

EMULATE_SERVICE_LD_LIBS := -lbinder -lutils

EMULATE_SERVICE_LDFLAGS := -L$(BUILD_DIR)/usr/lib
EMULATE_SERVICE_LDFLAGS += -Wl,-rpath-link=$(BUILD_DIR)/usr/lib

EMULATE_SERVICE_CFLAGS := -Wall -Wextra -std=c++17 -ggdb

PLANECONTROL_AIDL_LIB := $(BUILD_DIR)/usr/lib/$(PLANECONTROL_AIDL_LIB)
EMULATE_SERVICE_BIN := $(BUILD_DIR)/usr/lib/$(EMULATE_SERVICE_BIN)

all: $(EMULATE_SERVICE_BIN) $(PLANECONTROL_AIDL_LIB)

$(PLANECONTROL_AIDL_LIB): $(PLANECONTROL_AIDL_SRC:%.cpp=%.o)
	g++ -shared $(EMULATE_SERVICE_LDFLAGS) $^ -o $@ $(EMULATE_SERVICE_LD_LIBS)

$(EMULATE_SERVICE_BIN): $(PLANECONTROL_AIDL_LIB)
$(EMULATE_SERVICE_BIN): $(EMULATE_SERVICE_BIN_SRC:%.cpp=%.o)
	g++ $(EMULATE_SERVICE_LDFLAGS) \
		$(EMULATE_SERVICE_BIN_SRC:%.cpp=%.o) -o $@ \
		$(EMULATE_SERVICE_LD_LIBS) \
		-lpthread \
		-l$(PLANECONTROL_AIDL_LIB_NAME)

# TODO: to be compile out of source tree
%.o: %.cpp
	g++ $(EMULATE_SERVICE_CFLAGS) $(EMULATE_SERVICE_INC_FLAGS) -fPIC -c $< -o $@
