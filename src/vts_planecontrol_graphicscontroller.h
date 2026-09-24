
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

#pragma once
#include <thread>
#include <atomic>
#include <cstdint>
#include <utility>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

// EGL/egl.h drags in X11/Xlib.h on Linux, which #defines these as macros
// and breaks unrelated identifiers of the same name in gtest/Android headers.
#ifdef Status
#undef Status
#endif
#ifdef None
#undef None
#endif

#include <utils/StrongPointer.h>
#include <com/rdk/hal/planecontrol/IPlaneControl.h>
#include <com/rdk/hal/planecontrol/IGraphicsFbProvider.h>
#include <com/rdk/hal/planecontrol/BnGraphicsFbProviderListener.h>

class PlaneControlGraphicsController {
public:
    struct GraphicsFbClientBuffer {
        int32_t graphicsFbId = -1;
        int32_t width = 0;
        int32_t height = 0;
        int32_t stride = 0;
        int32_t offset = 0;
        int32_t format = 0;
        int32_t fd = -1;
    };

    PlaneControlGraphicsController();
    ~PlaneControlGraphicsController();

    // Provide only native window handle
    bool setWindowHandle(void* nativeWindowHandle);

    // Start/stop rendering thread
    void startRender();
    void stopRender();

    // Lightweight PlaneControl client API for graphics framebuffer lifecycle.
    bool connectPlaneControlProvider(int32_t planeResourceIndex = 0);
    void disconnectPlaneControlProvider();
    bool createGraphicsFbBuffer(int32_t width, int32_t height, GraphicsFbClientBuffer& outBuffer);
    bool createAndCommitSolidColorBuffer(
        int32_t width, int32_t height, uint8_t r, uint8_t g, uint8_t b,
        GraphicsFbClientBuffer& outBuffer);
    bool commitGraphicsFbBuffer(int32_t graphicsFbId, bool& accepted);
    bool waitForReleasedGraphicsFb(int32_t& oldGraphicsFbId, int64_t& elapsedRealtimeNanos, int timeoutMs = 1200);
    bool destroyGraphicsFbBuffer(int32_t graphicsFbId);
    bool destroyGraphicsFbBuffer(GraphicsFbClientBuffer& buffer);

private:
    class GraphicsFbProviderListener final : public com::rdk::hal::planecontrol::BnGraphicsFbProviderListener {
    public:
        explicit GraphicsFbProviderListener(PlaneControlGraphicsController* owner);
        ::android::binder::Status onGraphicsFbReleased(int32_t oldGraphicsFbId, int64_t elapsedRealtimeNanos) override;

    private:
        PlaneControlGraphicsController* owner_;
    };

    void onGraphicsFbReleased(int32_t oldGraphicsFbId, int64_t elapsedRealtimeNanos);

    bool initializeEGL();
    bool initializeGLES();
    void destroyEGL();
    void destroyGLES();
    void renderLoop();

private:
    void* windowHandle_ = nullptr;

    EGLDisplay eglDisplay_ = EGL_NO_DISPLAY;
    EGLSurface eglSurface_ = EGL_NO_SURFACE;
    EGLContext eglContext_ = EGL_NO_CONTEXT;

    android::sp<com::rdk::hal::planecontrol::IPlaneControl> planeControl_;
    android::sp<com::rdk::hal::planecontrol::IGraphicsFbProvider> graphicsFbProvider_;
    android::sp<GraphicsFbProviderListener> graphicsListener_;
    std::mutex releaseMutex_;
    std::condition_variable releaseCv_;
    std::queue<std::pair<int32_t, int64_t>> releasedFbEvents_;

    std::thread renderThread_;
    std::atomic<bool> running_{false};
};
