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

#include "AndroidEngine.hpp"
#include "types.hpp"

#include "Ruine.hpp"
#include "AndroidAssetService.hpp"

namespace Soleil {

  AndroidEngine::AndroidEngine()
    : inProgress(true)
    , focus(false)
    , glContext(AndroidGLESContext::getInstance())
    , initialized(false)
  {
  }

  AndroidEngine::~AndroidEngine() {}

  void AndroidEngine::run(struct android_app* androidApp)
  {
    androidApp->userData = this;
    androidApp->onAppCmd = AndroidEngine::HandleCommand;
    assetService = std::make_unique<AndroidAssetService>(androidApp->activity->assetManager);

    while (inProgress) {
      int                         ident;
      int                         events;
      struct android_poll_source* source;

      while ((ident = ALooper_pollAll(focus ? 0 : -1, NULL, &events,
                                      (void**)&source)) >= 0) {

        // Process this event:
        if (source != NULL) {
          source->process(androidApp, source);
        }

        // If we are exiting:
        if (androidApp->destroyRequested != 0) {
          // TODO: Detroy engine
          inProgress = false;
        }
      }

      if (focus && inProgress) {
        drawFrame();
      }
    }
  }

  void AndroidEngine::drawFrame()
  {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    Timer time = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::seconds(now.tv_sec) + std::chrono::nanoseconds(now.tv_nsec));

    // TODO: viewer->render(time);
    static Soleil::Ruine r(assetService.get());
    r.render(time);

    if (glContext->swap() == false) {
      // TODO: reloadResources();
    }
  }

  void AndroidEngine::terminateDisplay(void) { glContext->suspend(); }

  void AndroidEngine::HandleCommand(struct android_app* androidApp,
                                    int32_t             command)
  {
    AndroidEngine* This = static_cast<AndroidEngine*>(androidApp->userData);

    switch (command) {
      case APP_CMD_SAVE_STATE:
        // The system has asked us to save our current state. Do so.
        break;
      case APP_CMD_INIT_WINDOW:
        // The window is being shown, get it ready.
        if (androidApp->window != NULL) {
          This->initDisplay(androidApp);
          This->drawFrame();
        }
        break;
      case APP_CMD_TERM_WINDOW:
        // The window is being hidden or closed, clean it up.
        This->terminateDisplay();
        This->setFocusLost();
        break;
      case APP_CMD_GAINED_FOCUS:
        // When our app gains focus, we start monitoring the accelerometer.
        This->setFocusGranted();
        break;
      case APP_CMD_LOST_FOCUS:
        This->setFocusLost();
        This->drawFrame();
        break;
    }
  }

  void AndroidEngine::initDisplay(struct android_app* androidApp)
  {
    if (initialized == false) {
      glContext->init(androidApp->window);

      loadResources();

      initialized = true;
    } else {
      if (glContext->resume(androidApp->window) == false) {
        reloadResources();
      }
    }
  }

  void AndroidEngine::resize(void)
  {
    // AConfiguration* config = AConfiguration_new();
    // AConfiguration_fromAssetManager(config, state->activity->assetManager);
    // auto xdpi = AConfiguration_getDensity(config);
    // auto ydpi = AConfiguration_getDensity(config);
    // AConfiguration_delete(config);

    // TODO: viewer->resize(glContext->getScreenWidth(),
    // glContext->getScreenHeight());
  }

  void AndroidEngine::loadResources(void)
  {
    // TODO: assetService =
    // std::make_unique<Soleil::AndroidAssetService>(state->activity->assetManager,
    // "");
    // TODO: sle =
    // std::make_unique<Android::AndroidAudioService>(assetService.get());
    // TODO: viewer = generateViewerHookFunction(assetService.get(), sle.get());

    resize();
  }

  void AndroidEngine::reloadResources(void)
  {
    // TODO: AConfiguration* config = AConfiguration_new();
    // AConfiguration_fromAssetManager(config, state->activity->assetManager);

    resize();
  }

  void AndroidEngine::setFocusGranted() { focus = true; }

  void AndroidEngine::setFocusLost() { focus = false; }

} // Soleil
