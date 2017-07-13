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

#include "AndroidAssetService.hpp"
#include "AndroidSoundService.hpp"
#include "ControllerService.hpp"
#include "Ruine.hpp"
#include "TypesToOStream.hpp"
#include "mathutils.hpp"
#include "stringutils.hpp"
#include "types.hpp"

#include <math.h>
#include <stdexcept>

#include <glm/common.hpp>
#include <glm/glm.hpp>

namespace Soleil {

  class AndroidControllerService
  {
  public:
    AndroidControllerService() {}
    virtual ~AndroidControllerService() {}

  public:
    Controller player;
    glm::vec2  padPosition;
  };

  AndroidControllerService controllerService;

  Controller& ControllerService::GetPlayerController() noexcept
  {
    return controllerService.player;
  }

  glm::vec2& GetPadPosition(void) noexcept
  {
    return controllerService.padPosition;
  }

  Push& ControllerService::GetPush(void) noexcept
  {
    return controllerService.player.push;
  }

  AndroidEngine::AndroidEngine()
    : inProgress(true)
    , focus(false)
    , glContext(AndroidGLESContext::getInstance())
    , initialized(false)
  {
    controllerService.padPosition = glm::vec2(0.14f, 0.75f);
    SOLEIL__LOGGER_DEBUG("Creating Android Engine");
  }

  AndroidEngine::~AndroidEngine()
  {
    SOLEIL__LOGGER_DEBUG("Destructing Android Engine");
  }

  void AndroidEngine::run(struct android_app* androidApp)
  {
    androidApp->userData     = this;
    androidApp->onInputEvent = AndroidEngine::HandleInput;
    androidApp->onAppCmd     = AndroidEngine::HandleCommand;
    AssetService::Instance =
      std::make_unique<AndroidAssetService>(androidApp->activity->assetManager);
    SoundService::Instance =
      std::make_unique<AndroidSoundService>(AssetService::Instance.get());

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
          SOLEIL__LOGGER_DEBUG("Destroy requested");
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

    ruine->render(time);

    if (glContext->swap() == false) {
      reloadResources();
    }
  }

  void AndroidEngine::terminateDisplay(void) { glContext->suspend(); }

  void AndroidEngine::HandleCommand(struct android_app* androidApp,
                                    int32_t             command)
  {
    try {
      AndroidEngine* This = (AndroidEngine*)androidApp->userData;
      // static_cast<AndroidEngine*>(androidApp->userData);

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
    } catch (const std::runtime_error& e) {
      Logger::error(toString("Fatal error while handling command: ", e.what()));
      androidApp->destroyRequested =
        1; // TODO: Use message window and exit gracefully
    } catch (...) {
      assert(false && "Someone raised a non runtime_error exception");
      Logger::error("Unknown error while handling command: ");
    }
  }

  int32_t AndroidEngine::HandleInput(struct android_app* androidApp,
                                     AInputEvent*        event)
  {
    AndroidEngine* This = (AndroidEngine*)androidApp->userData;

    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {

      int32_t  action = AMotionEvent_getAction(event);
      uint32_t flags  = action & AMOTION_EVENT_ACTION_MASK;

      switch (flags) {
        case AMOTION_EVENT_ACTION_UP:
          controllerService.player.dpad        = glm::vec3(0.0f);
          controllerService.player.push.active = false;
          break;
        case AMOTION_EVENT_ACTION_DOWN:
        case AMOTION_EVENT_ACTION_MOVE: {
          const float x = AMotionEvent_getX(event, 0);
          const float y = AMotionEvent_getY(event, 0);

          const glm::vec2& point = controllerService.padPosition;
          const glm::vec2  touch(x / (float)This->glContext->getScreenWidth(),
                                y / (float)This->glContext->getScreenHeight());

          SOLEIL__LOGGER_DEBUG(toString("touch:", touch));

          controllerService.player.push.position =
            (glm::vec2(touch.x, -touch.y) - glm::vec2(0.5, -0.5f)) * 2.0f;
          controllerService.player.push.active = true;

          const glm::vec2 distance = point - touch;
          const float     length   = glm::length(distance);

          static const float min = 0.05f;
          static const float max = 0.175f;

          if (length >= min && length <= max) {
            // SOLEIL__LOGGER_DEBUG(toString("Touch= ", touch, ". Length=",
            // length,
            //                               ". Distance=", distance * 6.666f));

            static const float A     = 0.0f;
            static const float B     = 0.05f;
            static const float C     = 0.5f;
            static const float D     = 1.0f;
            const float        signx = sgn(distance.x);
            const float        signy = sgn(distance.y);

            // SOLEIL__LOGGER_DEBUG(toString("Bezier(",signx, "*", distance.x,
            // "*", "6.666f)=", bezier(A, B, C, D, signx * distance.x *
            // 6.666f)));

            controllerService.player.dpad.x =
              signx * bezier(A, B, C, D, signx * distance.x * 6.666f);
            controllerService.player.dpad.z =
              signy * bezier(A, B, C, D, signy * distance.y * 6.666f);
            // SOLEIL__LOGGER_DEBUG(
            //   toString("PAD(after)=", controllerService.player.dpad));
          }
        }; break;
      }
    }
    return 0;
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

    // TODO: Temporary:
    glViewport(0, 0, glContext->getScreenWidth(), glContext->getScreenHeight());
  }

  void AndroidEngine::loadResources(void)
  {
    // TODO: assetService =
    // std::make_unique<Soleil::AndroidAssetService>(state->activity->assetManager,
    // "");
    // TODO: sle =
    // std::make_unique<Android::AndroidAudioService>(assetService.get());
    // TODO: viewer = generateViewerHookFunction(assetService.get(),
    // sle.get());

    glViewport(0, 0, glContext->getScreenWidth(), glContext->getScreenHeight());

    ruine = std::make_unique<Ruine>(
      AssetService::Instance.get(), SoundService::Instance.get(),
      glContext->getScreenWidth(), glContext->getScreenHeight());

    resize();
  }

  void AndroidEngine::reloadResources(void)
  {
    // TODO: AConfiguration* config = AConfiguration_new();
    // AConfiguration_fromAssetManager(config, state->activity->assetManager);

    resize();
  }

  void AndroidEngine::setFocusGranted()
  {
    SoundService::ResumeMusic();
    focus = true;
  }

  void AndroidEngine::setFocusLost()
  {
    SoundService::PauseMusic();
    focus = false;
  }

} // Soleil
