/*
 * Copyright (C) 2017  Florian GOLESTIN
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "AndroidGLESContext.hpp"
#include "Logger.hpp"

namespace Soleil {
  void throwOnEGLError(const char* file, const int line)
  {
    EGLint error = eglGetError();

    if (error == EGL_SUCCESS) return;

    std::string message;
    switch (error) {
      case EGL_NOT_INITIALIZED:
        message = "EGL is not initialized, or could not be initialized, for "
                  "the specified EGL display connection.";
        break;
      case EGL_BAD_ACCESS:
        message = "EGL cannot access a requested resource (for example a "
                  "context is bound in another thread).";
        break;
      case EGL_BAD_ALLOC:
        message =
          "EGL failed to allocate resources for the requested operation.";
        break;
      case EGL_BAD_ATTRIBUTE:
        message = "An unrecognized attribute or attribute value was passed in "
                  "the attribute list.";
        break;
      case EGL_BAD_CONTEXT:
        message =
          "An EGLContext argument does not name a valid EGL rendering context.";
        break;
      case EGL_BAD_CONFIG:
        message = "An EGLConfig argument does not name a valid EGL frame "
                  "buffer configuration.";
        break;
      case EGL_BAD_CURRENT_SURFACE:
        message = "The current surface of the calling thread is a window, "
                  "pixel buffer or pixmap that is no longer valid.";
        break;
      case EGL_BAD_DISPLAY:
        message = "An EGLDisplay argument does not name a valid EGL display "
                  "connection.";
        break;
      case EGL_BAD_SURFACE:
        message = "An EGLSurface argument does not name a valid surface "
                  "(window, pixel buffer or pixmap) configured for GL "
                  "rendering.";
        break;
      case EGL_BAD_MATCH:
        message = "Arguments are inconsistent (for example, a valid context "
                  "requires buffers not supplied by a valid surface).";
        break;
      case EGL_BAD_PARAMETER:
        message = "One or more argument values are invalid.";
        break;
      case EGL_BAD_NATIVE_PIXMAP:
        message = "A NativePixmapType argument does not refer to a valid "
                  "native pixmap.";
        break;
      case EGL_BAD_NATIVE_WINDOW:
        message = "A NativeWindowType argument does not refer to a valid "
                  "native window.";
        break;
      case EGL_CONTEXT_LOST:
        message = "A power management event has occurred. The application must "
                  "destroy all contexts and reinitialise OpenGL ES state and "
                  "objects to continue rendering. ";
        break;
      default: message = toString("Unknown error N.", error);
    }
    throw EGLException(toString(file, ":", line, "\t", message));
  }

  AndroidGLESContext::AndroidGLESContext()
    : window(nullptr)
    , display(EGL_NO_DISPLAY)
    , surface(EGL_NO_SURFACE)
    , context(EGL_NO_CONTEXT)
    , glVersion(0)
  {
    SOLEIL__LOGGER_DEBUG("Constructing GLES Context");
  }

  AndroidGLESContext::~AndroidGLESContext(void)
  {
    SOLEIL__LOGGER_DEBUG("Destructing Android GLES Context");
    terminate();
  }

  void AndroidGLESContext::initGLES(void)
  {
    /* Currently only opengl 2.0 is supported */
    // const std::string glVersionStr((const char *)glGetString(GL_VERSION));
    // if (glVersionStr.find("OpenGL ES 3.") != std::string::npos &&
    // gl3stubInit())
    //   {
    // 	glVersion = 3.0f;
    // 	return ;
    //   }

    glVersion = 2.0f;
  }

  void AndroidGLESContext::init(ANativeWindow* window)
  {
    if (context != EGL_NO_CONTEXT) return;

    this->window = window;

    initEGLSurface();
    initEGLContext();
    initGLES();
  }

  bool AndroidGLESContext::initEGLSurface(void)
  {
    this->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(this->display != EGL_NO_DISPLAY && "No 'Default' EGL Display");

    eglInitialize(display, nullptr, nullptr);
    SOLEIL__THROW_ON_EGLERROR();

    const EGLint attribs[] = {EGL_RENDERABLE_TYPE,
                              EGL_OPENGL_ES2_BIT, // Request opengl ES2.0
                              EGL_SURFACE_TYPE,
                              EGL_WINDOW_BIT,
                              EGL_BLUE_SIZE,
                              8,
                              EGL_GREEN_SIZE,
                              8,
                              EGL_RED_SIZE,
                              8,
                              EGL_DEPTH_SIZE,
                              24,
                              EGL_NONE};

    colorSize = 8;
    depthSize = 32;

    EGLint numConfigs = 0;
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    if (numConfigs < 1) {
      throw EGLException("Device do not match minimum configuration");
    }
    SOLEIL__THROW_ON_EGLERROR();

    surface = eglCreateWindowSurface(display, config, window, nullptr);
    SOLEIL__THROW_ON_EGLERROR();
    eglQuerySurface(display, surface, EGL_WIDTH, &screenWidth);
    eglQuerySurface(display, surface, EGL_HEIGHT, &screenHeight);

    return true;
  }

  bool AndroidGLESContext::initEGLContext(void)
  {
    const EGLint attribs[] = {EGL_CONTEXT_CLIENT_VERSION,
                              2, // Request opengl ES2.0
                              EGL_NONE};

    context = eglCreateContext(display, config, nullptr, attribs);
    SOLEIL__THROW_ON_EGLERROR();

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
      SOLEIL__THROW_ON_EGLERROR();
    }
    return true;
  }

  void AndroidGLESContext::terminate(void) noexcept
  {
    if (display != EGL_NO_DISPLAY) {
      eglMakeCurrent(display, surface, surface, EGL_NO_CONTEXT);

      if (context != EGL_NO_CONTEXT) {
        eglDestroyContext(display, context);
      }

      if (surface != EGL_NO_SURFACE) {
        eglDestroySurface(display, surface);
      }

      eglTerminate(display);
    }

    display = EGL_NO_DISPLAY;
    context = EGL_NO_CONTEXT;
    surface = EGL_NO_SURFACE;
  }

  bool AndroidGLESContext::swap(void)
  {
    EGLBoolean swapped = eglSwapBuffers(display, surface);

    if (swapped == EGL_FALSE) {
      EGLint err = eglGetError();

      if (err == EGL_BAD_SURFACE) {
        SOLEIL__LOGGER_DEBUG("Bad surface while egl-swapping");

        initEGLSurface();
        return true; // Consider that the context is still valide
      } else if (err == EGL_CONTEXT_LOST || err == EGL_BAD_CONTEXT) {
        SOLEIL__LOGGER_DEBUG(
          "Oups, err == EGL_CONTEXT_LOST || err == EGL_BAD_CONTEXT");

        terminate();
        initEGLContext();
      }
      return false;
    }
    return true;
  }

  void AndroidGLESContext::invalidate(void) { terminate(); }

  void AndroidGLESContext::suspend(void)
  {
    if (surface != EGL_NO_SURFACE) {
      eglDestroySurface(display, surface);
      surface = EGL_NO_SURFACE;
    }
  }

  bool AndroidGLESContext::resume(ANativeWindow* window)
  {
    if (context == EGL_NO_CONTEXT) {
      init(window);
      return true;
    }

    int originalWidth  = screenWidth;
    int originalHeight = screenHeight;

    this->window = window;

    surface = eglCreateWindowSurface(display, config, window, nullptr);
    eglQuerySurface(display, surface, EGL_WIDTH, &screenWidth);
    eglQuerySurface(display, surface, EGL_HEIGHT, &screenHeight);
    SOLEIL__THROW_ON_EGLERROR();

    if (originalWidth != screenWidth || originalHeight != screenHeight) {
      // TODO: calls Resize
    }

    if (eglMakeCurrent(display, surface, surface, context) == EGL_TRUE) {
      return true;
    }

    EGLint err = eglGetError();
    if (err == EGL_CONTEXT_LOST) {
      initEGLContext();
    } else {
      terminate();
      initEGLSurface();
      initEGLContext();
    }
    return false;
  }

  AndroidGLESContext* AndroidGLESContext::getInstance(void)
  {
    static AndroidGLESContext instance;

    return &instance;
  }

} // Soleil
