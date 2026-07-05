
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
#include <EGL/egl.h>
#include <GLES2/gl2.h>

class PlaneControlGraphicsController {
public:
    PlaneControlGraphicsController();
    ~PlaneControlGraphicsController();

    // Provide only native window handle
    bool setWindowHandle(void* nativeWindowHandle);

    // Start/stop rendering thread
    void startRender();
    void stopRender();

private:
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

    std::thread renderThread_;
    std::atomic<bool> running_{false};
};
