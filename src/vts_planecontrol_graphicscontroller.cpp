
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

#include "vts_planecontrol_graphicscontroller.h"
#include "vts_planecontrol.h"
#include <iostream>
#include <chrono>
#include <unistd.h>

#include <binder/Status.h>
#include <binder/ProcessState.h>
#include <binder/ParcelFileDescriptor.h>
#include <sys/mman.h>

#ifdef Status
#undef Status
#endif

PlaneControlGraphicsController::PlaneControlGraphicsController() {}

PlaneControlGraphicsController::~PlaneControlGraphicsController() {
    stopRender();
    destroyGLES();
    destroyEGL();
    disconnectPlaneControlProvider();
}

PlaneControlGraphicsController::GraphicsFbProviderListener::GraphicsFbProviderListener(PlaneControlGraphicsController* owner)
    : owner_(owner) {}

::android::binder::Status PlaneControlGraphicsController::GraphicsFbProviderListener::onGraphicsFbReleased(
    int32_t oldGraphicsFbId, int64_t elapsedRealtimeNanos)
{
    if (owner_ != nullptr)
    {
        owner_->onGraphicsFbReleased(oldGraphicsFbId, elapsedRealtimeNanos);
    }
    return ::android::binder::Status::ok();
}

bool PlaneControlGraphicsController::setWindowHandle(void* nativeWindowHandle) {
    windowHandle_ = nativeWindowHandle;

    // Initialize EGL + GLES when window handle is known
    if (!initializeEGL()) {
        std::cerr << "EGL initialization failed\n";
        return false;
    }
    if (!initializeGLES()) {
        std::cerr << "GLES initialization failed\n";
        return false;
    }

    return true;
}

void PlaneControlGraphicsController::startRender() {
    if (running_) return;
    running_ = true;
    renderThread_ = std::thread(&PlaneControlGraphicsController::renderLoop, this);
}

void PlaneControlGraphicsController::stopRender() {
    if (!running_) return;
    running_ = false;
    if (renderThread_.joinable())
        renderThread_.join();
}

bool PlaneControlGraphicsController::connectPlaneControlProvider(int32_t planeResourceIndex)
{
    if (graphicsFbProvider_ != nullptr)
    {
        return true;
    }

    if (getPlaneControlService() != UT_STATUS_OK || gPlaneControlservice == nullptr)
    {
        std::cerr << "PlaneControl service not available\n";
        return false;
    }

    planeControl_ = gPlaneControlservice;

    android::ProcessState::self()->startThreadPool();

    graphicsListener_ = new GraphicsFbProviderListener(this);
    ::android::binder::Status binderStatus = planeControl_->getGraphicsFbProvider(planeResourceIndex, graphicsListener_, &graphicsFbProvider_);
    if (!binderStatus.isOk() || graphicsFbProvider_ == nullptr)
    {
        std::cerr << "getGraphicsFbProvider failed: " << binderStatus.toString8().c_str() << "\n";
        graphicsFbProvider_.clear();
        graphicsListener_.clear();
        planeControl_.clear();
        return false;
    }

    return true;
}

void PlaneControlGraphicsController::disconnectPlaneControlProvider()
{
    graphicsFbProvider_.clear();
    graphicsListener_.clear();
    planeControl_.clear();

    {
        std::lock_guard<std::mutex> lock(releaseMutex_);
        std::queue<std::pair<int32_t, int64_t>> empty;
        std::swap(releasedFbEvents_, empty);
    }
    releaseCv_.notify_all();
}

bool PlaneControlGraphicsController::createGraphicsFbBuffer(int32_t width, int32_t height, GraphicsFbClientBuffer& outBuffer)
{
    if (graphicsFbProvider_ == nullptr)
    {
        return false;
    }

    com::rdk::hal::planecontrol::GraphicsFbInfo info;
    ::android::os::ParcelFileDescriptor pfd;
    ::android::binder::Status binderStatus = graphicsFbProvider_->createGraphicsFb(width, height, &info, &pfd);
    if (!binderStatus.isOk())
    {
        std::cerr << "createGraphicsFb failed: " << binderStatus.toString8().c_str() << "\n";
        return false;
    }

    const int dupFd = dup(pfd.get());
    if (dupFd < 0)
    {
        std::cerr << "dup for graphics fb fd failed\n";
        (void)graphicsFbProvider_->destroyGraphicsFb(info.graphicsFbId);
        return false;
    }

    outBuffer.graphicsFbId = info.graphicsFbId;
    outBuffer.width = info.pixelWidth;
    outBuffer.height = info.pixelHeight;
    outBuffer.stride = info.stride;
    outBuffer.offset = info.offset;
    outBuffer.format = 0;
    outBuffer.fd = dupFd;
    return true;
}

bool PlaneControlGraphicsController::createAndCommitSolidColorBuffer(
    int32_t width, int32_t height, uint8_t r, uint8_t g, uint8_t b,
    GraphicsFbClientBuffer& outBuffer)
{
    if (!createGraphicsFbBuffer(width, height, outBuffer))
    {
        return false;
    }

    const size_t mapLength = static_cast<size_t>(outBuffer.offset) +
        static_cast<size_t>(outBuffer.stride) * static_cast<size_t>(outBuffer.height);
    void* mapped = mmap(nullptr, mapLength, PROT_READ | PROT_WRITE,
                        MAP_SHARED, outBuffer.fd, 0);
    if (mapped == MAP_FAILED)
    {
        destroyGraphicsFbBuffer(outBuffer);
        return false;
    }

    fillSolidColorToDmaBuf(static_cast<uint8_t*>(mapped), outBuffer.stride,
                           outBuffer.width, outBuffer.height, outBuffer.offset,
                           r, g, b);
    munmap(mapped, mapLength);

    bool accepted = false;
    if (!commitGraphicsFbBuffer(outBuffer.graphicsFbId, accepted) || !accepted)
    {
        destroyGraphicsFbBuffer(outBuffer);
        return false;
    }

    return true;
}

bool PlaneControlGraphicsController::commitGraphicsFbBuffer(int32_t graphicsFbId, bool& accepted)
{
    if (graphicsFbProvider_ == nullptr)
    {
        return false;
    }

    accepted = false;
    ::android::binder::Status binderStatus = graphicsFbProvider_->commitGraphicsFb(graphicsFbId, &accepted);
    if (!binderStatus.isOk())
    {
        std::cerr << "commitGraphicsFb failed: " << binderStatus.toString8().c_str() << "\n";
        return false;
    }
    return true;
}

bool PlaneControlGraphicsController::waitForReleasedGraphicsFb(
    int32_t& oldGraphicsFbId, int64_t& elapsedRealtimeNanos, int timeoutMs)
{
    std::unique_lock<std::mutex> lock(releaseMutex_);
    const bool ready = releaseCv_.wait_for(
        lock,
        std::chrono::milliseconds(timeoutMs),
        [this]() { return !releasedFbEvents_.empty(); });

    if (!ready)
    {
        return false;
    }

    const auto event = releasedFbEvents_.front();
    releasedFbEvents_.pop();
    oldGraphicsFbId = event.first;
    elapsedRealtimeNanos = event.second;
    return true;
}

bool PlaneControlGraphicsController::destroyGraphicsFbBuffer(int32_t graphicsFbId)
{
    if (graphicsFbProvider_ == nullptr)
    {
        return false;
    }

    ::android::binder::Status binderStatus = graphicsFbProvider_->destroyGraphicsFb(graphicsFbId);
    if (!binderStatus.isOk())
    {
        std::cerr << "destroyGraphicsFb failed: " << binderStatus.toString8().c_str() << "\n";
        return false;
    }
    return true;
}

bool PlaneControlGraphicsController::destroyGraphicsFbBuffer(GraphicsFbClientBuffer& buffer)
{
    const bool destroyed = buffer.graphicsFbId < 0 ||
        destroyGraphicsFbBuffer(buffer.graphicsFbId);
    if (buffer.fd >= 0)
    {
        close(buffer.fd);
    }
    buffer = GraphicsFbClientBuffer{};
    return destroyed;
}

void PlaneControlGraphicsController::onGraphicsFbReleased(int32_t oldGraphicsFbId, int64_t elapsedRealtimeNanos)
{
    {
        std::lock_guard<std::mutex> lock(releaseMutex_);
        releasedFbEvents_.push({oldGraphicsFbId, elapsedRealtimeNanos});
    }
    releaseCv_.notify_one();
}

bool PlaneControlGraphicsController::initializeEGL() {
    if (!windowHandle_) return false;

    EGLNativeWindowType eglNativeWin = (EGLNativeWindowType)windowHandle_;

    // Display automatically chosen by EGL
    eglDisplay_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay_ == EGL_NO_DISPLAY) {
        std::cerr << "EGL: No default display\n";
        return false;
    }

    if (!eglInitialize(eglDisplay_, nullptr, nullptr)) {
        std::cerr << "EGL: eglInitialize failed\n";
        return false;
    }

    const EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_NONE
    };

    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(eglDisplay_, attribs, &config, 1, &numConfigs) || numConfigs < 1) {
        std::cerr << "EGL: No matching config\n";
        return false;
    }

    eglSurface_ = eglCreateWindowSurface(eglDisplay_, config, eglNativeWin, nullptr);
    if (eglSurface_ == EGL_NO_SURFACE) {
        std::cerr << "EGL: eglCreateWindowSurface failed\n";
        return false;
    }

    const EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    eglContext_ = eglCreateContext(eglDisplay_, config, EGL_NO_CONTEXT, ctxAttribs);
    if (eglContext_ == EGL_NO_CONTEXT) {
        std::cerr << "EGL: eglCreateContext failed\n";
        return false;
    }

    if (!eglMakeCurrent(eglDisplay_, eglSurface_, eglSurface_, eglContext_)) {
        std::cerr << "EGL: eglMakeCurrent failed\n";
        return false;
    }

    return true;
}

bool PlaneControlGraphicsController::initializeGLES() {
    const char* vshader =
        "attribute vec2 aPos;"
        "void main() { gl_Position = vec4(aPos,0.0,1.0); }";

    const char* fshader =
        "precision mediump float;"
        "void main() { gl_FragColor = vec4(1.0,0.4,0.0,1.0); }";

    auto compile = [](GLenum type, const char* src)->GLuint {
        GLuint sh = glCreateShader(type);
        glShaderSource(sh, 1, &src, nullptr);
        glCompileShader(sh);
        return sh;
    };

    GLuint vs = compile(GL_VERTEX_SHADER, vshader);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fshader);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    glUseProgram(prog);

    return true;
}

void PlaneControlGraphicsController::destroyGLES() {
    // If you store buffers or program handles, delete them here
}

void PlaneControlGraphicsController::destroyEGL() {
    if (eglDisplay_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (eglContext_ != EGL_NO_CONTEXT) eglDestroyContext(eglDisplay_, eglContext_);
        if (eglSurface_ != EGL_NO_SURFACE) eglDestroySurface(eglDisplay_, eglSurface_);
        eglTerminate(eglDisplay_);
    }

    eglDisplay_ = EGL_NO_DISPLAY;
    eglSurface_ = EGL_NO_SURFACE;
    eglContext_ = EGL_NO_CONTEXT;
}

void PlaneControlGraphicsController::renderLoop() {
    while (running_) {
        glClearColor(0.1f,0.1f,0.2f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw triangles or UI here

        eglSwapBuffers(eglDisplay_, eglSurface_);
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60FPS
    }
}

