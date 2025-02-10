/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @brief
 * PlaneControl.h
 *
 * The header file contains the PlaneControl service interfaces.
 */

#ifndef _PLANE_CONTROL_H_
#define _PLANE_CONTROL_H_

#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include <android/log.h>
#include <binder/BinderService.h>
#include <binder/Status.h>
#include <utils/Errors.h>
#include <utils/Log.h>

#include <utils/String16.h>
#include <utils/Vector.h>

#include "com/rdk/hal/planecontrol/BnPlaneControl.h"
#include "com/rdk/hal/planecontrol/IPlaneControlListener.h"
// TODO: PlaneResource got renamed in commit `3b32784f33bf8140bfd3dcf3e7239013583c5566` in hal.git
// #include "com/rdk/hal/planecontrol/PlaneResource.h"
#include "com/rdk/hal/planecontrol/SourcePlaneMapping.h"
#include "com/rdk/hal/planecontrol/Property.h"
#include "com/rdk/hal/PropertyValue.h"
#include "com/rdk/hal/planecontrol/Property.h"
#include "com/rdk/hal/planecontrol/PropertyKVPair.h"

// libutils
using android::OK;
using android::String8;
using android::String16;
using android::sp;

// libbinder
using android::defaultServiceManager;
using android::binder::Status;

// Generated code
using com::rdk::hal::planecontrol::BnPlaneControl;

// Standard library
using std::string;
using std::unique_ptr;
using std::vector;
using std::mutex;

using namespace com::rdk::hal::planecontrol;

class PlaneControl : public android::BinderService<PlaneControl>,
                  public BnPlaneControl {
    private:
        friend class BinderService<PlaneControl>;

    PlaneControl() {}
        virtual ~PlaneControl() = default;

        static char const* getServiceName() { return IPlaneControl::serviceName().c_str(); }

    // ::android::binder::Status getResourceCapabilities(::std::vector<::com::rdk::hal::planecontrol::PlaneResource>* _aidl_return) override;
    ::android::binder::Status getNativeGraphicsWindowHandle(int32_t planeResourceIndex, int64_t* _aidl_return) override;
    ::android::binder::Status releaseNativeGraphicsWindowHandle(int32_t planeResourceIndex, int64_t nativeWindowHandle) override;
    ::android::binder::Status flipGraphicsBuffer(int32_t planeResourceIndex, bool* _aidl_return) override;
    // ::android::binder::Status setVideoSourceDestinationPlane(const ::std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping>& listSourcePlaneMapping, bool* _aidl_return) override;
    ::android::binder::Status setVideoSourceDestinationPlaneMapping(const ::std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping>& listSourcePlaneMapping, bool* _aidl_return) override;
    ::android::binder::Status getProperty(int32_t planeResourceIndex, ::com::rdk::hal::planecontrol::Property property, ::std::optional<::com::rdk::hal::PropertyValue>* _aidl_return) override;
    ::android::binder::Status setProperty(int32_t planeResourceIndex, ::com::rdk::hal::planecontrol::Property property, const ::com::rdk::hal::PropertyValue& propertyValue, bool* _aidl_return) override;
    // ::android::binder::Status getPropertyMulti(int32_t planeResourceIndex, ::std::vector<::com::rdk::hal::planecontrol::PropertyKVPair>* propertyKVList) override;
    ::android::binder::Status getPropertyMulti(int32_t planeResourceIndex, ::std::vector<::com::rdk::hal::planecontrol::PropertyKVPair>* propertyKVList, bool* _aidl_return) override;
    // ::android::binder::Status setPropertyMultiAtomic(int32_t planeResourceIndex, const ::std::vector<::com::rdk::hal::planecontrol::PropertyKVPair>& propertyKVList) override;
    ::android::binder::Status setPropertyMultiAtomic(int32_t planeResourceIndex, const ::std::vector<::com::rdk::hal::planecontrol::PropertyKVPair>& propertyKVList, bool* _aidl_return) override;
    ::android::binder::Status registerListener(const ::android::sp<::com::rdk::hal::planecontrol::IPlaneControlListener>& listener, bool* _aidl_return) override;
    ::android::binder::Status unregisterListener(const ::android::sp<::com::rdk::hal::planecontrol::IPlaneControlListener>& listener, bool* _aidl_return) override;
    ::android::binder::Status getCapabilities(::std::vector<::com::rdk::hal::planecontrol::Capabilities>* _aidl_return) override;
    ::android::binder::Status getVideoSourceDestinationPlaneMapping(::std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping>* _aidl_return) override;
};

#endif //_PLANE_CONTROL_H_
