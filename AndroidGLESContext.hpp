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

#ifndef SOLEIL__ANDROIDGLESCONTEXT_HPP_
#define SOLEIL__ANDROIDGLESCONTEXT_HPP_

// #include <cassert>
// #include <errno.h>
// #include <initializer_list>
// #include <jni.h>
// #include <memory>

#include <stdexcept>

#include <android_native_app_glue.h>

#include "OpenGLInclude.hpp"

namespace Soleil {

  class AndroidGLESContext
  {
  public:
    AndroidGLESContext();
    virtual ~AndroidGLESContext();

  public:
    void init(ANativeWindow* window);
    bool swap(void);

    void invalidate(void);
    void suspend(void);
    bool resume(ANativeWindow* window);

    int getScreenWidth(void) const noexcept { return screenWidth; }
    int getScreenHeight(void) const noexcept { return screenHeight; }

    static AndroidGLESContext* getInstance(void);

  private:
    void terminate(void) noexcept;
    void initGLES(void);
    bool initEGLSurface(void);
    bool initEGLContext(void);

  private:
    ANativeWindow* window;
    EGLDisplay     display;
    EGLSurface     surface;
    EGLContext     context;
    EGLConfig      config;

  private:
    float glVersion;

  private:
    int colorSize;
    int depthSize;
    int screenWidth;
    int screenHeight;
  };

  inline void throwOnEGLError(const char* file, const int line);
#define SOLEIL__THROW_ON_EGLERROR() ::Soleil::throwOnEGLError(__FILE__, __LINE__)

  typedef std::runtime_error EGLException;

} // Soleil

#endif /* SOLEIL__ANDROIDGLESCONTEXT_HPP_ */
