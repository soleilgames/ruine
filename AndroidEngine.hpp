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

#ifndef SOLEIL__ANDROIDENGINE_HPP_
#define SOLEIL__ANDROIDENGINE_HPP_

#include <android_native_app_glue.h>

#include "AndroidGLESContext.hpp"
#include "OpenGLInclude.hpp"
#include "AssetService.hpp"

#include <memory>

namespace Soleil {

  class AndroidEngine
  {
  public:
    AndroidEngine();
    virtual ~AndroidEngine();

  public:
    void run(struct android_app* androidApp);

  public:
    static void HandleCommand(struct android_app* androidApp, int32_t command);

  private:
    void drawFrame();
    void initDisplay(struct android_app* androidApp);
    void terminateDisplay();
    void loadResources();
    void reloadResources();
    void resize();
    void setFocusGranted();
    void setFocusLost();

  private:
    bool                          inProgress;
    bool                          focus;
    AndroidGLESContext*           glContext;
    bool                          initialized;
    std::unique_ptr<AssetService> assetService;
  };

} // Soleil

#endif /* SOLEIL__ANDROIDENGINE_HPP_ */
