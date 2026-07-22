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

/** @brief
 * PlaneControl.h
 *
 * The header file contains the PlaneControl service interfaces.
 */

#ifndef PLANE_CONTROL_H
#define PLANE_CONTROL_H

#include <binder/BinderService.h>
#include <optional>
#include <string>
#include <utils/StrongPointer.h>
#include <vector>
#include <set>
#include <mutex>
#include <condition_variable>
#include <future>
#include <chrono>
#include <thread>
#include <ctime>
#include <sstream>
#include <iomanip>

#include "com/rdk/hal/planecontrol/BnPlaneControl.h"
#include "com/rdk/hal/planecontrol/BnGraphicsFbProvider.h"
#include "com/rdk/hal/planecontrol/IPlaneControlListener.h"
#include "com/rdk/hal/planecontrol/IGraphicsFbProvider.h"
#include "com/rdk/hal/planecontrol/IGraphicsFbProviderListener.h"
#include <binder/ParcelFileDescriptor.h>
#include "com/rdk/hal/videodecoder/DynamicRange.h"
#include "com/rdk/hal/videodecoder/PixelFormat.h"

namespace com {
    namespace rdk {
        namespace hal {
            namespace planecontrol {

#define NUM_PLANES_VIDEO 2
#define NUM_PLANES_GRAPHIC 1

                class PlaneControl : public android::BinderService<PlaneControl>,
                                     public BnPlaneControl {

                public:

                    PlaneControl();
                    virtual ~PlaneControl();

                    static char const *getServiceName() { return IPlaneControl::serviceName().c_str(); }

                    ::android::binder::Status getCapabilities(::std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities>* _aidl_return) override;
                    ::android::binder::Status getGraphicsFbProvider(int32_t planeResourceIndex, const ::android::sp<::com::rdk::hal::planecontrol::IGraphicsFbProviderListener>& graphicsFbProviderListener, ::android::sp<::com::rdk::hal::planecontrol::IGraphicsFbProvider>* _aidl_return) override;
                    ::android::binder::Status setVideoSourceDestinationPlaneMapping(const ::std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping>& listSourcePlaneMapping, bool* _aidl_return) override;
                    ::android::binder::Status getVideoSourceDestinationPlaneMapping(::std::vector<::com::rdk::hal::planecontrol::SourcePlaneMapping>* _aidl_return) override;
                    ::android::binder::Status getProperty(int32_t planeResourceIndex, ::com::rdk::hal::planecontrol::Property property, ::std::optional<::com::rdk::hal::PropertyValue>* _aidl_return) override;
                    ::android::binder::Status setProperty(int32_t planeResourceIndex, ::com::rdk::hal::planecontrol::Property property, const ::com::rdk::hal::PropertyValue& propertyValue, bool* _aidl_return) override;
                    ::android::binder::Status getPropertyMulti(int32_t planeResourceIndex, const ::std::vector<::com::rdk::hal::planecontrol::Property>& properties, ::std::vector<::com::rdk::hal::planecontrol::PropertyKVPair>* propertyKVList, bool* _aidl_return) override;
                    ::android::binder::Status setPropertyMultiAtomic(int32_t planeResourceIndex, const ::std::vector<::com::rdk::hal::planecontrol::PropertyKVPair>& propertyKVList, bool* _aidl_return) override;
                    ::android::binder::Status registerListener(const ::android::sp<::com::rdk::hal::planecontrol::IPlaneControlListener>& listener, bool* _aidl_return) override;
                    ::android::binder::Status unregisterListener(const ::android::sp<::com::rdk::hal::planecontrol::IPlaneControlListener>& listener, bool* _aidl_return) override;

                private:
                    std::unordered_map<int, long> mNativeWindowHandles;
                    std::unordered_map<int, ::com::rdk::hal::planecontrol::SourcePlaneMapping> mVideoPlaneMappings;
                    std::map<int32_t, std::map<Property, PropertyValue>> mPlaneProperties;
                    std::set<android::sp<IPlaneControlListener>> mListeners;
                    std::mutex mHandleLock;
                    int64_t mNextHandle = 0x10000000;
                    std::vector<::com::rdk::hal::planecontrol::PlaneCapabilities> mCapabilities;
                    int mVideoPlaneCount;
                    int mGraphicsPlaneCount;

                    ::com::rdk::hal::planecontrol::PlaneCapabilities buildCapabilities(
                            int index,
                            ::com::rdk::hal::planecontrol::PlaneType type,
                            std::vector<::com::rdk::hal::videodecoder::PixelFormat> pixelFormats,
                            int maxWidth,
                            int maxHeight,
                            int frameWidth,
                            int frameHeight,
                            std::vector<::com::rdk::hal::videodecoder::DynamicRange> dynamicRanges,
                            int colorDepth,
                            int maxFrameRate,
                            bool supportsAlpha,
                            bool supportsZOrder,
                            int vsyncDisplayLatency
                    );

                    std::map<::com::rdk::hal::planecontrol::Property, ::com::rdk::hal::PropertyValue> buildProperties(::com::rdk::hal::planecontrol::PlaneCapabilities capabilities);
                };
            }
        }
    }
}

#endif PLANE_CONTROL_H
