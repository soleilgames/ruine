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
#include "Recorder.hpp"
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
#include <glm/gtx/rotate_vector.hpp>
#include <glm/matrix.hpp>

namespace Soleil {

#if 0
  /**
   * Draw a round shaped pad
   */
  static void DrawPad()
  {
    static const glm::mat4 modelMatrix =
      glm::scale(glm::translate(glm::mat4(), glm::vec3(-1.0f, -1.0f, 0.0f)),
                 glm::vec3(0.5625f, 1.0f, 1.0f));

    DrawImage(*OpenGLDataInstance::Instance().texturePad, modelMatrix);
  }
#endif

  static void InitializeWorld(World& world, Frame& frame, Camera& camera,
                              PopUp& caption, int& goldScore)
  {
    world.resetLevel();
    world.theKey = glm::mat4();
    world.coinPickedUp.clear();
    world.keyPickedUp = false;
    // ---
    frame.pointLights.clear();
    camera.yaw = 0.0f;
    goldScore  = 0;

    InitializeWorldModels(world);
    InitializeWorldDoors(world, "doors.ini");
    InitializeLevel(world, gval::firstLevel, frame, camera, caption);
#if 1
    SoundService::PlayMusic("farlands.ogg");
#endif
  }

  static void UpdateGhost(std::vector<GhostData>&  ghosts,
                          std::vector<PointLight>& lights, const Timer& delta,
                          const BoundingBox& worldSize,
                          const glm::vec3&   playerPosition)
  {
    for (GhostData& ghost : ghosts) {
      const glm::mat4 transformation = glm::translate(
        *(ghost.transformation),
        ghost.direction * gval::ghostSpeed * (float)delta.count());

      glm::vec3 position = glm::vec3(transformation[3]);
      if (!worldSize.contains(position)) {
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

  static void UpdateHunters(std::vector<GhostData>&  ghosts,
                            std::vector<PointLight>& lights, const Timer& delta,
                            const glm::vec3& playerPosition)
  {
    // Do not update the hunters if the player is not moving:
    if (ControllerService::GetPlayerController().locked) return;

    for (GhostData& ghost : ghosts) {
      const glm::vec3 ghostPosition = glm::vec3((*(ghost.transformation))[3]);
      const glm::vec3 cam = glm::vec3(playerPosition.x, 0.0f, playerPosition.z);
      const glm::mat4 fixGhostRotation =
        glm::rotate(glm::mat4(), glm::pi<float>(), glm::vec3(0, 1, 0));
      // TODO: ^ A fix because the ghost model is not pointing forward

      const glm::mat4 transformation =
        glm::translate(
          glm::inverse(glm::lookAt(ghostPosition, cam, glm::vec3(0, 1, 0))),
          glm::vec3(0, 0, -1) * gval::ghostSpeed * (float)delta.count() /
            10.0f) *
        fixGhostRotation;

      *(ghost.transformation) = transformation;
      ghost.updateBounds();
      lights[ghost.lightPosition].position = glm::vec3(transformation[3]);
    }
  }

  static void RenderScene(World& world, Frame& frame)
  {
    if (ControllerService::GetPlayerController().option2) {
      UpdateGhost(world.sentinels, frame.pointLights, frame.delta, world.bounds,
                  frame.cameraPosition);
      // UpdateHunters(world.sentinels, frame.pointLights, frame.delta,
      //               frame.cameraPosition);
    }

    if (ControllerService::GetPlayerController().option3) {
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);

      for (const auto& e : world.elements) {
        RenderFlatShape(e.transformation, *world.shapes[e.shapeIndex], frame);
      }
      for (const auto& e : world.items) {
        RenderFlatShape(e.transformation, *world.shapes[e.shapeIndex], frame);
      }
      for (const auto& e : world.ghosts) {
        RenderFlatShape(e.transformation, *world.shapes[e.shapeIndex], frame);
      }
    }

#ifndef NDEBUG
    // On development allow to draw bonding box of elements
    if (ControllerService::GetPlayerController().option4) {
      glDisable(GL_DEPTH_TEST);

      for (const auto& box : world.hardSurfaces) {
        DrawBoundingBox(box, frame);
      }

      for (const auto& box : world.triggers) {
        DrawBoundingBox(box.aoe, frame, RGBA(1.0f, 0.3f, 0.3f, 0.5f));
      }

#if 0      
      for (const auto& t : world.coinTriggers) {
        DrawBoundingBox(t.aoe, frame, glm::vec4(1.0f, 1.0f, 0.0f, 0.5f));
      }

      for (const auto& g : world.sentinels) {
        DrawBoundingBox(g.bounds, frame, glm::vec4(0.0f, 0.1f, 0.3f, 0.5f));

        DrawBoundingBox(g.stressBounds, frame,
                        glm::vec4(0.1f, 0.1f, 0.3f, 0.5f));
      }
#endif
    }
#endif
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

#if 1 // TODO: Enable/Disable via a controller option
    // Collision test
    BoundingBox bboxForward;
    bboxForward.expandBy(positionForward);
    bboxForward.expandBy(positionForward + 0.20f);
    bboxForward.expandBy(positionForward - 0.20f);
    BoundingBox bboxSideward;
    bboxSideward.expandBy(positionSideward);
    bboxSideward.expandBy(positionSideward + 0.2f);
    bboxSideward.expandBy(positionSideward - 0.2f);

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

  void Ruine::updateTriggers(World& world, Frame& frame)
  {
    const BoundingBox playerbox = [this]() -> BoundingBox {
      BoundingBox box;
      box.expandBy(camera.position);
      box.expandBy(camera.position + glm::vec3(0.25f, 1.5f, 0.25f));
      box.expandBy(camera.position - glm::vec3(0.25f, 1.5f, 0.25f));
      return box;
    }();

    // First check if the player is not colliding with a monster
    for (const auto& ghost : world.sentinels) {
      if (playerbox.intersect(ghost.bounds)) {
        glm::mat4 transformation = glm::translate(
          glm::mat4(), glm::vec3(camera.position.x, 0.0f, camera.position.z));

        DrawElement draw;
        draw.shapeIndex     = ShapeType::Ghost;
        draw.transformation = transformation;
        world.ghosts.emplace_back(draw);
        world.sentinels.push_back(GhostData(
          &(world.ghosts.back().transformation), 0, glm::vec3(0, 0, 1)));

        ControllerService::GetPlayerController().locked = true;
        timeToReset = frame.time + Timer(5000);
        return;
      }
    }

    // If not but he's close to one, play a frightening sound
    if (nextGhostSound.count() <= 0) {
      for (const auto& ghost : world.sentinels) {
        if (playerbox.intersect(ghost.stressBounds)) {
          SoundService::FireSound("ghost.wav", SoundProperties(100));
          nextGhostSound = gval::timeBeforeWhisper;
          break;
        }
      }
    }

    // Finally money and doors!
    for (auto trigger = world.triggers.begin();
         trigger != world.triggers.end();) {
      bool doIncrement = true;

      if (playerbox.intersect(trigger->aoe)) {

        if (trigger->state & TriggerState::JustTriggered) {
          trigger->state &= ~TriggerState::JustTriggered;
          trigger->state |= TriggerState::CurrentlyActive;
          trigger->state |= TriggerState::AlreadyTriggered;
        } else if (!(trigger->state & TriggerState::CurrentlyActive)) {
          trigger->state |= TriggerState::JustTriggered;
          trigger->state |= TriggerState::CurrentlyActive;
        }
        // else
        assert(trigger->state & TriggerState::CurrentlyActive);

        switch (trigger->type) {

          case TriggerType::Coin: {
            SOLEIL__LOGGER_DEBUG(toString("GOLDDDDD"));
            goldScore += 5; // TODO: Constant
            Text::FillBuffer(
              toWString(L"BUTIN: ", goldScore, L" / 260"), goldLabel,
              OpenGLDataInstance::Instance().textAtlas, gval::textLabelSize);

            for (auto it = world.items.begin(); it != world.items.end(); ++it) {
              const auto& coinItem = *it;
              if (coinItem.id == trigger->link) {
                world.items.erase(it);
                break;
              }
            }

            doIncrement = false;
            world.coinPickedUp.push_back(trigger->link);
            world.triggers.erase(trigger);
            SoundService::FireSound("coins.wav", SoundProperties(100));
          } break;
          case TriggerType::Door: {
            if (trigger->state & TriggerState::JustTriggered) {
              const Door* door = GetDoorByUID(world.doors, trigger->link);

              // Before, check if it's not the final door:
              if (door->id.compare("0") == 0) {
                SoundService::FireSound("locked.wav", SoundProperties(100));
                if (world.keyPickedUp) {
                  state = State::StateCredits;
                } else {
                  caption.fillText(L"IL FAUT UNE CLEF", 0.45f);
                }
                caption.activate(gval::timeToFadeText, frame.time);
              } else {

                world.resetLevel();
                frame.pointLights.clear();

                SoundService::FireSound("doors.wav", SoundProperties(100));

                const std::string nextZone = door->id;
                InitializeLevel(world, nextZone, frame, camera, caption);
              }
              return;
            }

          } break;
          case TriggerType::Ghost: break;
          case TriggerType::Key: {
            for (auto it = world.items.begin(); it != world.items.end(); ++it) {
              const auto& coinItem = *it;
              if (coinItem.id == trigger->link) {
                world.items.erase(it);
                break;
              }
            }
            doIncrement       = false;
            world.keyPickedUp = true;
            world.triggers.erase(trigger);
            SoundService::FireSound("key.wav", SoundProperties(100));
          } break;
        }
      } else {
        trigger->state &= ~TriggerState::JustTriggered;
        trigger->state &= ~TriggerState::CurrentlyActive;
      }

      // Only increment the iterator if it's not of type coin because iterator
      // already moved
      if (doIncrement) ++trigger;
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

#ifndef NDEBUG
    console.transformation =
      glm::translate(glm::mat4(), glm::vec3(-1.0f, 0.7f, 0.0f));
#endif
  }

  Ruine::~Ruine() {}

  void Ruine::render(Timer time)
  {
#ifndef NDEBUG
    auto workBegin = std::chrono::high_resolution_clock::now();
#endif

    frame.delta = time - frame.time;
    frame.time  = time;

    int currentState = state;
    if (currentState & State::StateInitializing) initializeGame(time);
    if (currentState & State::StateGame) renderGame(time);
    if (currentState & State::StateCredits) renderCredits(time);
    if (currentState & State::StateMenu) renderMenu(time);
    if (currentState & State::StateDialogue) {
      renderDialogue(time);
    }
    if (currentState & State::StateFadingIn) {
      Fade(fading.ratio(time));

#if 1
      if (fading.ratio(time) >= 1.0f) {
        state  = State::StateInitializing;
        fading = FadeTimer(time, Timer(2000));
      }
#endif
    }
    if (currentState & State::StateFadingOut) {
      Fade(1.0f - fading.ratio(time));
      if (fading.ratio(time) >= 1.0f) state &= ~State::StateFadingOut;
    }

    const Push& push = ControllerService::GetPlayerController().push;
    SOLEIL__RECORD_INPUT(push.active, push.start, push.position);
    SOLEIL__RECORD_FRAME(frame.cameraPosition, state, goldScore,
                         world.keyPickedUp, time);

    // TODO: Should be in a parent method
    // Reset the release state
    ControllerService::GetPush().active &= ~PushState::Fresh;
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
        const auto duration = TotalDuration / frames;
        SOLEIL__LOGGER_DEBUG("Time to draw previous frame: ", duration,
                             " (FPS=", frames, ")");
        FillBuffer(toWString("TIME TO DRAW PREVIOUS FRAME: ", duration,
                             " (FPS=", frames, ")"),
                   textCommand, OpenGLDataInstance::Instance().textAtlas, 0.8f);
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
    }
#endif
  }

  void Ruine::initializeGame(const Timer& /*time*/)
  {
    InitializeWorld(world, frame, camera, caption, goldScore);
    caption.transformation =
      glm::translate(glm::mat4(), glm::vec3(-0.35f, -0.35f, 0.0f));
    goldLabelTransformation =
      glm::translate(glm::mat4(), glm::vec3(+0.35f, +0.75f, 0.0f));
    Text::FillBuffer(toWString(L"BUTIN: ", goldScore, " / 260"), goldLabel,
                     OpenGLDataInstance::Instance().textAtlas,
                     gval::textLabelSize);

    dialogueTransformation =
      glm::translate(glm::mat4(), glm::vec3(-0.35f, -0.35f, 0.0f));
    dialogueBackgroundTransformation =
      glm::scale(glm::translate(glm::mat4(), glm::vec3(-0.45f, -0.8f, 0.0f)),
                 glm::vec3(1.55f, .8f, 1.0f));

    state    = State::StateFadingOut | State::StateGame | State::StateDialogue;
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
      if (playerPad.push.active == PushState::Active) {
#if 0
	// Old (dynamic) Forward translation system
        const glm::vec3 translation(
          0.0f, 0.0f,
          (playerPad.push.position.y - playerPad.push.start.y) / 10.0f);
#else
        const glm::vec3 translation(0.0f, 0.0f, 0.055f);
#endif

        const float yaw = playerPad.push.start.x - playerPad.push.position.x;
        this->view = updateCamera(&camera, translation, yaw, world.hardSurfaces,
                                  frame.delta);
      } else if (playerPad.push.active ==
                 (PushState::DoubleClick | PushState::Fresh)) {
        camera.yaw += glm::pi<float>();
        this->view = updateCamera(&camera, glm::vec3(0.0f), 0.0f,
                                  world.hardSurfaces, frame.delta);
      } else {
        this->view = updateCamera(&camera, glm::vec3(0.0f), 0.0f,
                                  world.hardSurfaces, frame.delta);
        // TODO: Only required for the first frame
      }

      frame.cameraPosition = camera.position;
      frame.updateViewProjectionMatrices(view, projection);
      frame.pointLights[0].position = camera.position;
    } else if ((state & StateDialogue)) {
      this->view = updateCamera(&camera, glm::vec3(0.0f), 0.0f,
                                world.hardSurfaces, frame.delta);
    } else {
      this->view =
        TargetPosition(glm::vec3(world.ghosts.back().transformation[3]), 5.0f,
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

    if (playerPad.locked == false)
      updateTriggers(world, frame);
    else if (time >= timeToReset) {
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

    static Menu     menu;
    static Frame    frame;
    static Pristine firstFrame;

    if (firstFrame) {
      AssetService::LoadTextureLow(*menu.door, "menu.png");
      Text::FillBuffer(L"R U I N E", menu.title,
                       OpenGLDataInstance::Instance().textAtlas, 3.0f);
      Text::FillBuffer(L"APPUYEZ POUR COMMENCER", menu.newGame,
                       OpenGLDataInstance::Instance().textAtlas, 1.0f);

      const glm::vec2 scale =
        glm::vec2(2.0f) / glm::vec2(viewportWidth, viewportHeight);
      menu.doorTransformation =
        glm::scale(glm::translate(glm::mat4(), glm::vec3(-0.6f, -1.0f, 0.0f)),
                   glm::vec3(1024.0f * scale.x, 1024.0f * scale.y, 0.0f));

      menu.titleTransformation =
        glm::translate(glm::mat4(), glm::vec3(-0.5f, 0.6f, 0.0f));
      menu.newGameTransformation =
        glm::translate(glm::mat4(), glm::vec3(-0.45f, -0.8f, 0.0f));

      frame.updateViewProjectionMatrices(
        glm::mat4(),
        glm::ortho(0.0f, (float)viewportWidth, 0.0f, (float)viewportHeight));
    }

    DrawImage(*menu.door, menu.doorTransformation);
    DrawText(menu.title, menu.titleTransformation, gval::textLabelColor);
    DrawText(menu.newGame, menu.newGameTransformation, gval::textLabelColor);

    if (ControllerService::GetPush().active & PushState::Down &&
        ControllerService::GetPush().active & PushState::Fresh) {
      state  = State::StateFadingIn;
      fading = FadeTimer(time, Timer(1000));
    }
  }

  void Ruine::renderDialogue(const Timer&)
  {
    const BoundingBox bounds(glm::vec3(-0.45f, -0.8f, 0.0f),
                             glm::vec3(1.0f, 1.0f, 0.0f));

    static const std::wstring text[] = {
      L"TU ES MAINTENANT ENFERME DANS CETTE RUINE",
      L"APPUIE SUR TON ECRAN POUR AVANCER",
      L"DEPLACE TON DOIGT VERS LA GAUCHE OU LA DROITE POUR TOURNER. APPUIE "
      L"DEUX FOIS POUR UN DEMI-TOUR",
      L"TROUVE LA CLEF POUR SORTIR DE CE LIEU",
      L"BON COURAGE MON AMI ET FAIS ATTENTION A MES CONGENERES"};

    ControllerService::GetPlayerController().locked = true;

    if (dialogueQueue.size() < 1) {
      if (sentence == 0) {
        for (const std::wstring& sentence : text) {
          dialogueQueue.push(sentence);
        }
      } else {
        dialogueQueue.push(text[2]);
      }
      sentence = 1;
      dirty    = true;
    }

    if (dirty) {
      Text::FillBufferWithDimensions(dialogueQueue.front(), dialogueLabel,
                                     OpenGLDataInstance::Instance().textAtlas,
                                     1.0f, glm::vec2(1.00f, 1.0f));
      dirty = false;
    }
    DrawImage(*OpenGLDataInstance::Instance().textureBlack,
              dialogueBackgroundTransformation);
    DrawText(dialogueLabel, dialogueTransformation, gval::textLabelColor);
    const Push& push = ControllerService::GetPush();

    if (push.active & PushState::Release &&
        bounds.containsFlat(ControllerService::GetPush().position)) {
      dialogueQueue.pop();
      dirty = true;
      SoundService::FireSound("psasasa.wav", SoundProperties(80));
    }

#if 1 // TODO: require work on triggers
    if (dialogueQueue.size() < 1) {
      state ^= StateDialogue;
      ControllerService::GetPlayerController().locked = false;
    }
#endif
  }

  void Ruine::renderCredits(const Timer&)
  {
    static Pristine first;

    if (first) {
      // TODO: Replace this by an image, it will be better
      Text::FillBuffer(
        L"      RUINE\n       -----\n\n\nPar Florian "
        L"GOLESTIN\n\n\n   REMERCIEMENTS\n    "
        L"---------------\nAntoine TALLON\nJulien GERBIER\nHoracio "
        L"GOLDBERG",
        credits, OpenGLDataInstance::Instance().textAtlas, 1.0f);
      creditsTransformation =
        glm::translate(glm::mat4(), glm::vec3(-0.30f, 0.0f, 0.0f));
    }

    glClearColor(0.0f, 0.05f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    creditsTransformation =
      glm::translate(creditsTransformation, glm::vec3(0.0f, 0.001f, 0.0f));
    DrawText(credits, creditsTransformation, gval::textLabelColor);

    if (ControllerService::GetPush().active & PushState::Down &&
        ControllerService::GetPush().active & PushState::Fresh) {
      ControllerService::GetPush().active = PushState::Inactive;
      state                               = State::StateMenu;
    }
  }

} // Soleil
