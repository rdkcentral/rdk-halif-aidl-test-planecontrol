
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
#include <iostream>
#include <chrono>

PlaneControlGraphicsController::PlaneControlGraphicsController() {}

PlaneControlGraphicsController::~PlaneControlGraphicsController() {
    stopRender();
    destroyGLES();
    destroyEGL();
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

