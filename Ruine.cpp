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

#include "Ruine.hpp"

#include "AssetService.hpp"
#include "BoundingBox.hpp"
#include "ControllerService.hpp"
#include "Group.hpp"
#include "Logger.hpp"
#include "OpenGLDataInstance.hpp"
#include "Pristine.hpp"
#include "Shape.hpp"
#include "SoundService.hpp"
#include "TypesToOStream.hpp"
#include "WavefrontLoader.hpp"
#include "World.hpp"
#include "mathutils.hpp"
#include "types.hpp"

#include <cmath>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>

namespace Soleil {

  static void DrawPad()
  {
    static const glm::mat4 modelMatrix =
      glm::scale(glm::translate(glm::mat4(), glm::vec3(-1.0f, -1.0f, 0.0f)),
                 glm::vec3(0.5625f, 1.0f, 1.0f));

    DrawImage(*OpenGLDataInstance::Instance().texturePad, modelMatrix);
  }

  static void InitializeWorld(World& world, Frame& frame, Camera& camera,
                              PopUp& caption)
  {
    InitializeWorldModels(world);
    InitializeWorldDoors(world, "doors.ini");
    InitializeLevel(world, gval::firstLevel, frame, camera, caption);
#if 1
    SoundService::PlayMusic("farlands.ogg");
#endif
  }

  static void UpdateTorchColor(std::vector<PointLight>& lights)
  {
    for (unsigned int i = 1; i < lights.size(); ++i) {
      int r                          = std::rand() % 65;
      int doit                       = std::rand() % 100;
      if (doit > 50) lights[i].color = glm::vec3(r / 255.0f);
    }
  }

  static void UpdateGhost(std::vector<GhostData>&  ghosts,
                          std::vector<PointLight>& lights, const Timer& delta,
                          const glm::vec3& worldSize)
  {
    for (GhostData& ghost : ghosts) {
      const glm::mat4 transformation = glm::translate(
        *(ghost.transformation),
        ghost.direction * gval::ghostSpeed * (float)delta.count());

      glm::vec3 position = glm::vec3(transformation[3]);
      if (position.z < 0.0f || position.z > worldSize.z) {
        const glm::mat4 rotation = glm::rotate(
          *(ghost.transformation), glm::pi<float>(), glm::vec3(0, 1, 0));
        *(ghost.transformation) = rotation;
      } else {
        *(ghost.transformation) = transformation;
      }

      ghost.updateBounds();
      lights[ghost.lightPosition].position = glm::vec3(transformation[3]);
    }
  }

  static void RenderScene(World& world, Frame& frame)
  {
    if (ControllerService::GetPlayerController().option2)
      UpdateGhost(world.sentinels, frame.pointLights, frame.delta,
                  world.bounds);

    if (ControllerService::GetPlayerController().option3) {

#if 0
        UpdateTorchColor(frame.pointLights);
#endif
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
#if 0
    RenderPhongShape(world.statics, frame);
#endif
      RenderFlatShape(world.statics, frame);
    }

#ifndef NDEBUG
    if (ControllerService::GetPlayerController().option4) {
      glDisable(GL_DEPTH_TEST);

      for (const auto& box : world.hardSurfaces) {
        DrawBoundingBox(box, frame);
      }

      for (const auto& t : world.coinTriggers) {
        DrawBoundingBox(t.aoe, frame, glm::vec4(1.0f, 1.0f, 0.0f, 0.5f));
      }

      for (const auto& g : world.sentinels) {
        DrawBoundingBox(g.bounds, frame, glm::vec4(0.0f, 0.1f, 0.3f, 0.5f));

        DrawBoundingBox(g.stressBounds, frame,
                        glm::vec4(0.1f, 0.1f, 0.3f, 0.5f));
      }
    }
#endif
    if (ControllerService::GetPlayerController().locked == false) DrawPad();
  }

  static glm::mat4 updateCamera(Camera* camera, const glm::vec3& translation,
                                const float                     yaw,
                                const std::vector<BoundingBox>& wallBoundingBox,
                                const Timer&                    deltaTime)
  {
    camera->yaw += yaw * gval::cameraYawSpeed * deltaTime.count();

    const float speed = gval::cameraSpeed * deltaTime.count();

    // There is no pitch nor Roll, to the equation is simplified:
    glm::vec3 direction =
      glm::normalize(glm::vec3(sin(camera->yaw), 0.0f, cos(camera->yaw)));
    glm::vec3 newPosition =
      camera->position + (translation.z * direction * speed);
    const glm::vec3 positionForward(camera->position.x, newPosition.y,
                                    newPosition.z);
    const glm::vec3 positionSideward(newPosition.x, newPosition.y,
                                     camera->position.z);

#if 1 // TODO: controller Option
    BoundingBox bboxForward;
    bboxForward.expandBy(positionForward);
    bboxForward.expandBy(positionForward + 0.25f);
    bboxForward.expandBy(positionForward - 0.25f);
    BoundingBox bboxSideward;
    bboxSideward.expandBy(positionSideward);
    bboxSideward.expandBy(positionSideward + 0.25f);
    bboxSideward.expandBy(positionSideward - 0.25f);

    for (const auto& boundingBox : wallBoundingBox) {
      if (boundingBox.intersect(bboxForward))
        newPosition.z = camera->position.z;
      if (boundingBox.intersect(bboxSideward))
        newPosition.x = camera->position.x;
    }
#endif

    camera->position = newPosition;
    return glm::lookAt(camera->position, camera->position + direction,
                       glm::vec3(0, 1, 0));
  }

  glm::mat4 TargetPosition(const glm::vec3& target, const float zoom,
                           const float angle)
  {
    // TODO: Angle -> degrees as a type

    glm::vec3 position;

    position.z = glm::cos(angle);
    position.y = glm::sin(angle);

    position *= zoom;

    return glm::lookAt(position + target, target, glm::vec3(0, 1, 0));
  }

  // TODO: Refactor this method as it has too much parameters
  void Ruine::updateTriggers(World& world, Frame& frame)
  {
    BoundingBox playerbox;
    playerbox.expandBy(camera.position);
    playerbox.expandBy(camera.position + glm::vec3(0.25f, 1.5f, 0.25f));
    playerbox.expandBy(camera.position - glm::vec3(0.25f, 1.5f, 0.25f));

    for (const auto& trigger : world.nextZoneTriggers) {
      if (playerbox.intersect(trigger.aoe)) {
        world.resetLevel();
        frame.pointLights.clear();

        SoundService::FireSound("doors.wav", SoundProperties(100));

        InitializeLevel(world, trigger.nextZone, frame, camera, caption);
        return;
      }
    }

    for (const auto& ghost : world.sentinels) {

      if (playerbox.intersect(ghost.bounds)) {
        glm::mat4 transformation =
          glm::translate(glm::mat4(), glm::vec3(camera.position.x, -0.60f,
                                                camera.position.z)) *
          glm::scale(glm::mat4(), glm::vec3(.3f));
        // TODO: Use same transformation as for world

        world.statics.emplace_back(
          DrawCommand(*world.ghostShape, transformation));
        world.sentinels.push_back(GhostData(
          &(world.statics.back().transformation), 0, glm::vec3(0, 0, 1)));

        SOLEIL__LOGGER_DEBUG(toString("Perduuuuuuuuuuuuuuuuuuuuuuuu"));
        ControllerService::GetPlayerController().locked = true;
        timeToReset = frame.time + Timer(5000);
        return;
      }
    }

    if (nextGhostSound.count() <= 0) {
      for (const auto& ghost : world.sentinels) {
        if (playerbox.intersect(ghost.stressBounds)) {
          SoundService::FireSound("ghost.wav", SoundProperties(100));
          nextGhostSound = gval::timeBeforeWhisper;
          break;
        }
      }
    }

    for (auto coinTrigger = world.coinTriggers.begin();
         coinTrigger != world.coinTriggers.end();) {
      if (playerbox.intersect(coinTrigger->aoe)) {
        SOLEIL__LOGGER_DEBUG(toString("GOLDDDDD"));
        goldScore += 5; // TODO: Constant
        Text::FillBuffer(toWString("BUTIN: ", goldScore), goldLabel,
                         OpenGLDataInstance::Instance().textAtlas,
                         gval::textLabelSize);

        for (auto it = world.statics.begin(); it != world.statics.end(); ++it) {
          const auto& drawCommand = *it;
          if (drawCommand.buffer == world.purseShape->getBuffer() &&
              drawCommand.transformation == coinTrigger->coinTransformation) {
            world.statics.erase(it);
            break;
          }
        }

        world.coinPickedUp.push_back(coinTrigger->coinTransformation);
        world.coinTriggers.erase(coinTrigger);
        SoundService::FireSound("coins.wav", SoundProperties(100));
      } else {
        ++coinTrigger;
      }
    }

    if (world.keyPickedUp == false) {
      const BoundingBox keybox(world.theKey, 0.3f);

      if (playerbox.intersect(keybox)) {
        world.keyPickedUp = true;
        auto pos = std::find(std::begin(world.statics), std::end(world.statics),
                             DrawCommand(*world.keyShape, world.theKey));
        assert(pos != std::end(world.statics) && "Key was not found");
        world.statics.erase(pos);

        SoundService::FireSound("key.wav", SoundProperties(100));
      }
    }
    // Quick hack before a better implementation: ------------
    const BoundingBox start(glm::vec3(4.9f, 0.0f, 0.0f),
                            glm::vec3(7.1f, 0.0f, 1.1f));
    if (caption.isActive() == false && playerbox.intersect(start)) {
      SoundService::FireSound("locked.wav", SoundProperties(100));

      if (world.keyPickedUp) {
#if 0
        caption.fillText(L"BRAVO ! IL N'Y A RIEN D'AUTRE À FAIRE...", 0.25f);
#else
        state = State::StateCredits;
#endif
      } else {
        caption.fillText(L"J'AI BESOIN D'UNE CLEE", 0.45f);
      }
      caption.activate(gval::timeToFadeText, frame.time);
    }
#if 0
    const BoundingBox dbox(
      glm::translate(glm::mat4(), glm::vec3(5.0f * 2.0f, 16.0f * 2.0f, 0)), 5.0f);
#else
    const BoundingBox dbox(
      glm::translate(glm::mat4(), glm::vec3(2.0f, 2.0f, 0)), 2.0f);
#endif
    if (playerbox.intersect(dbox)) {
      state |= State::StateDialogue;
    } else if (state & State::StateDialogue) {
      state &= ~State::StateDialogue;
    }
  }

  Ruine::Ruine(AssetService* assetService, SoundService* soundService,
               int viewportWidth, int viewportHeight)
    : assetService(assetService)
    , soundService(soundService)
    , viewportWidth(viewportWidth)
    , viewportHeight(viewportHeight)
    , goldScore(0)
    , nextGhostSound(0)
    , state(State::StateMenu)
  {
    warnOnGlError();
    OpenGLDataInstance::Initialize();

    OpenGLDataInstance::Instance().viewport =
      glm::vec2(viewportWidth, viewportHeight);

    camera.yaw      = 0.0f;
    camera.position = glm::vec3(0.0f);

#ifndef NDEBUG
    console.transformation =
      glm::translate(glm::mat4(), glm::vec3(-1.0f, 0.7f, 0.0f));
#endif
  }

  Ruine::~Ruine() {}

  void Ruine::render(Timer time)
  {
#ifndef NDEBUG
    // TODO: Use CPU clock
    auto workBegin = std::chrono::high_resolution_clock::now();
#endif

    frame.delta = time - frame.time;
    frame.time  = time;

    if (state & State::StateInitializing) initializeGame(time);
    if (state & State::StateGame) renderGame(time);
    if (state & State::StateCredits) renderCredits(time);
    if (state & State::StateMenu) renderMenu(time);
    if (state & State::StateDialogue) {
      renderDialogue(time);
    }
    if (state & State::StateFadingIn) {
      Fade(fading.ratio(time));

#if 1
      if (fading.ratio(time) >= 1.0f) {
        state = State::StateInitializing;
      }
#endif
    }
    if (state & State::StateFadingOut) {
      Fade(1.0f - fading.ratio(time));
      if (fading.ratio(time) >= 1.0f) state &= ~State::StateFadingOut;
    }

    // Reset the release state
    if (ControllerService::GetPush().active == PushState::Release) {
      ControllerService::GetPush().active = PushState::Inactive;
    }

#ifndef NDEBUG
    {
      /* --- Peformance log --- */
      static Timer       firstTime = time;
      static unsigned    frames    = 0;
      static const auto  oneSec    = std::chrono::milliseconds(1000);
      static TextCommand textCommand;

      frames++;
      auto         workEnd = std::chrono::high_resolution_clock::now();
      static Timer TotalDuration(0);

      TotalDuration += std::chrono::duration_cast<std::chrono::milliseconds>(
        workEnd - workBegin);

      if (time - firstTime > oneSec) {
#if 0
        const auto duration = (time - firstTime) / frames;
#else
        const auto duration = TotalDuration / frames;
#endif
        SOLEIL__LOGGER_DEBUG("Time to draw previous frame: ", duration,
                             " (FPS=", frames, ")");
        FillBuffer(toWString("TIME TO DRAW PREVIOUS FRAME: ", duration,
                             " (FPS=", frames, ")"),
                   textCommand, OpenGLDataInstance::Instance().textAtlas, 0.4f);
        firstTime     = time;
        frames        = 0;
        TotalDuration = Timer(0);
      }
      DrawText(
        textCommand,
        glm::scale(glm::translate(glm::mat4(), glm::vec3(-1.0f, 0.9f, 0.0f)),
                   glm::vec3(0.3f)),
        Color(0.8f));

      SOLEIL__CONSOLE_DRAW();
#if 0 // TextAtlas
      glm::mat4 transformation =
        glm::scale(glm::translate(glm::mat4(), glm::vec3(-1.0f, -1.0f, 0.0f)),
                   glm::vec3(2.0f));
      DrawImage(*OpenGLDataInstance::Instance().textDefaultFontAtlas,
                transformation);
#endif
    }
#endif
  }

  void Ruine::initializeGame(const Timer& /*time*/)
  {
    InitializeWorld(world, frame, camera, caption);
    caption.transformation =
      glm::translate(glm::mat4(), glm::vec3(-0.35f, -0.35f, 0.0f));
    goldLabelTransformation =
      glm::translate(glm::mat4(), glm::vec3(+0.35f, +0.75f, 0.0f));
    Text::FillBuffer(toWString("BUTTIN: ", goldScore), goldLabel,
                     OpenGLDataInstance::Instance().textAtlas,
                     gval::textLabelSize);

    
    dialogueTransformation =
      glm::translate(glm::mat4(), glm::vec3(-0.35f, -0.35f, 0.0f));
    dialogueBackgroundTransformation =
      glm::scale(glm::translate(glm::mat4(), glm::vec3(-0.4f, -0.8f, 0.0f))
		 , glm::vec3(1.2f, .8f, 1.0f));

    state    = State::StateFadingOut | State::StateGame;
    sentence = 0;
    dirty    = true;
  }

  void Ruine::renderGame(const Timer& time)
  {

#if 0
    glClearColor(0.0f, 0.3f, 0.7f, 1.0f);
#else
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#endif
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static const glm::mat4 projection = glm::perspective(
      glm::radians(50.0f), (float)viewportWidth / (float)viewportHeight, 0.1f,
      50.0f);

    if (nextGhostSound > Timer(0)) {
      nextGhostSound -= frame.delta;
    }

    Controller& playerPad = ControllerService::GetPlayerController();

    if (playerPad.locked == false) {
      const glm::vec3 translation(0.0f, 0.0f, playerPad.dpad.z / 20.0f);
      const float     yaw = playerPad.dpad.x;
      this->view = updateCamera(&camera, translation, yaw, world.hardSurfaces,
                                frame.delta);

      frame.cameraPosition = camera.position;
      frame.updateViewProjectionMatrices(view, projection);
      frame.pointLights[0].position = camera.position;
    } else {
      this->view =
        TargetPosition(glm::vec3(world.statics.back().transformation[3]), 5.0f,
                       glm::pi<float>() / 2.5f);
      frame.cameraPosition = camera.position;
      frame.updateViewProjectionMatrices(view, projection);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // --------------- Render Scene ---------------
    RenderScene(world, frame);
    if (caption.isActive()) {
      caption.render(time);
    }
    DrawText(goldLabel, goldLabelTransformation, gval::textLabelColor);
// --------------- Render Scene ---------------

#if 0
    glm::mat4 transformation =
      glm::scale(glm::translate(glm::mat4(), glm::vec3(-1.0f, -1.0f, 0.0f)),
                 glm::vec3(2.0f));
    DrawImage(gval::bezierTex, transformation);
#endif

    if (playerPad.locked == false)
      updateTriggers(world, frame);
    else if (time >= timeToReset) {
      // TODO: Use different function for each scene or camera
      playerPad.locked = false;
      world.resetLevel();
      frame.pointLights.clear();
      camera.yaw = 0.0f;
      InitializeLevel(world, gval::firstLevel, frame, camera, caption);
    }
  }

  void Ruine::renderMenu(const Timer& time)
  {
    // TODO: If no change, no need to update

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: Do not use static for that:
    static Menu     menu;
    static Pristine firstFrame;
    static Frame    frame;

    if (firstFrame) {
      AssetService::LoadTextureLow(*menu.door, "menu.png");
      Text::FillBuffer(L"R U I N E", menu.title,
                       OpenGLDataInstance::Instance().textAtlas, 3.0f);
      Text::FillBuffer(L"NEW GAME", menu.newGame,
                       OpenGLDataInstance::Instance().textAtlas, 1.0f,
                       &menu.newGameBounds);

      const glm::vec2 scale =
        glm::vec2(2.0f) / glm::vec2(viewportWidth, viewportHeight);
      menu.doorTransformation =
        glm::scale(glm::translate(glm::mat4(), glm::vec3(-0.6f, -1.0f, 0.0f)),
                   glm::vec3(1024.0f * scale.x, 1024.0f * scale.y, 0.0f));

      menu.titleTransformation =
        glm::translate(glm::mat4(), glm::vec3(-0.5f, 0.6f, 0.0f));
      menu.newGameTransformation =
        glm::translate(glm::mat4(), glm::vec3(-0.9f, 0.4f, 0.0f));
      menu.newGameBounds.transform(menu.newGameTransformation);

      frame.updateViewProjectionMatrices(
        glm::mat4(),
        glm::ortho(0.0f, (float)viewportWidth, 0.0f, (float)viewportHeight));
    }

    DrawImage(*menu.door, menu.doorTransformation);
    DrawText(menu.title, menu.titleTransformation, gval::textLabelColor);
    DrawText(menu.newGame, menu.newGameTransformation, gval::textLabelColor);

    const Push& push = ControllerService::GetPush();
    if (push.active == PushState::Release) {
      SOLEIL__CONSOLE_TEXT(toWString(L"Push: ", push.position));
      if (menu.newGameBounds.containsFlat(push.position)) {
        state  = State::StateFadingIn;
        fading = FadeTimer(time, Timer(1000));
      } else
        SOLEIL__CONSOLE_APPEND(StringToWstring(toString(menu.newGameBounds)));
    }
  }

  void Ruine::renderDialogue(const Timer&)
  {
    static BoundingBox bounds;

    static const std::wstring text[] = {
      L"ATTENDS ! AIDE-MOI, J'AI CONSTRUIT CE SOUTERRAIN. MAIS NOTRE SIR NOUS Y A ENFERME",
      L"J'AI CONSTRUIT CE SOUTERRAIN. MAIS NOTRE SIR NOUS Y A ENFERME",
      L"LIBERES-MOI ET JE\nT'AIDERAI A SORTIR",
      L"VA DANS LA PRISON.\nIL Y A UN SCEAU\nQUI ME RETIENT", L"DETRUIT-LE"};

    if (dirty) {
      bounds = BoundingBox();
      Text::FillBufferWithDimensions(text[sentence], dialogueLabel,
				     OpenGLDataInstance::Instance().textAtlas, 1.0f, glm::vec2(1.00f, 1.0f));
      dirty = false;
      bounds.transform(dialogueTransformation);
    }
    DrawImage(*OpenGLDataInstance::Instance().textureBlack,
              dialogueBackgroundTransformation);
    DrawText(dialogueLabel, dialogueTransformation, gval::textLabelColor);
    const Push& push = ControllerService::GetPush();

    if (push.active == PushState::Release &&
        bounds.containsFlat(ControllerService::GetPush().position)) {
      sentence++;
      dirty = true;
      if (sentence > 4) // TODO: No hard code
      {
        sentence = 4;
      }
    }
  }

  void Ruine::renderCredits(const Timer& time)
  {
    static Pristine first;

    if (first) {
      // TODO: Replace this by an image, it will be better
      Text::FillBuffer(L"      RUINE\n       -----\n\n\nPar Florian "
                       L"GOLESTIN\n\n\n   REMERCIEMENTS\n    "
                       L"---------------\nAntoine TALLON\nJuju\nHoracio "
                       L"GOLDBERG",
                       credits, OpenGLDataInstance::Instance().textAtlas, 1.0f);
      creditsTransformation =
        glm::translate(glm::mat4(), glm::vec3(-0.30f, 0.0f, 0.0f));
    }

    glClearColor(0.0f, 0.05f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: Delta time
    creditsTransformation =
      glm::translate(creditsTransformation, glm::vec3(0.0f, 0.001f, 0.0f));
    DrawText(credits, creditsTransformation, gval::textLabelColor);

    if (ControllerService::GetPush().active == PushState::Release) {
      state = State::StateMenu;
    }
  }

} // Soleil
