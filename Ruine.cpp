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
#include "Draw.hpp"
#include "Group.hpp"
#include "Logger.hpp"
#include "OpenGLDataInstance.hpp"
#include "Pristine.hpp"
#include "Shape.hpp"
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

  static void InitializeWorld(World& world, Frame& frame, Camera& camera)
  {
    InitializeWorldModels(world);
    InitializeLevelFromAsset(world, gval::firstLevel, frame, camera);

    SoundService::PlayMusic("farlands.ogg");
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

#if 0
        UpdateTorchColor(frame.pointLights);
#endif
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
#if 0
    RenderDrawCommands(statics, frame);
#else
    RenderFlatShape(world.statics, frame);
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

    BoundingBox bboxForward;
    bboxForward.expandBy(positionForward);
    bboxForward.expandBy(positionForward + 0.15f);
    bboxForward.expandBy(positionForward - 0.15f);
    BoundingBox bboxSideward;
    bboxSideward.expandBy(positionSideward);
    bboxSideward.expandBy(positionSideward + 0.15f);
    bboxSideward.expandBy(positionSideward - 0.15f);

    for (const auto& boundingBox : wallBoundingBox) {
      if (boundingBox.intersect(bboxForward))
        newPosition.z = camera->position.z;
      if (boundingBox.intersect(bboxSideward))
        newPosition.x = camera->position.x;
    }

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

  static void UpdateTriggers(Camera& camera, Frame& frame, World& world,
                             Timer& timeToReset)
  {
    BoundingBox playerbox;
    playerbox.expandBy(camera.position);
    playerbox.expandBy(camera.position + 0.25f);
    playerbox.expandBy(camera.position - 0.25f);

    for (const auto& trigger : world.nextZoneTriggers) {
      if (playerbox.intersect(trigger.aoe)) {
        world.resetLevel();
        frame.pointLights.clear();
        InitializeLevelFromAsset(world, trigger.nextZone, frame, camera);
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
  }

  Ruine::Ruine(AssetService* assetService, SoundService* soundService,
               int viewportWidth, int viewportHeight)
    : assetService(assetService)
    , soundService(soundService)
    , viewportWidth(viewportWidth)
    , viewportHeight(viewportHeight)
  {
    warnOnGlError();
    OpenGLDataInstance::Initialize();

    camera.yaw      = 0.0f;
    camera.position = glm::vec3(0.0f);
  }

  Ruine::~Ruine() {}

  void Ruine::render(Timer time)
  {
    static Frame    frame;
    static World    world;
    static Pristine FirstLoop;

    if (FirstLoop) {
      InitializeWorld(world, frame, camera);
      frame.time = time;
    }

#if 0
    glClearColor(0.0f, 0.3f, 0.7f, 1.0f);
#else
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#endif
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static const glm::mat4 projection = glm::perspective(
      glm::radians(50.0f), (float)viewportWidth / (float)viewportHeight, 0.1f,
      50.0f);

    frame.delta = time - frame.time;
    frame.time  = time;

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

    RenderScene(world, frame);
#if 0
    glm::mat4 transformation =
      glm::scale(glm::translate(glm::mat4(), glm::vec3(-1.0f, -1.0f, 0.0f)),
                 glm::vec3(2.0f));
    DrawImage(gval::bezierTex, transformation);
#endif

    if (playerPad.locked == false)
      UpdateTriggers(camera, frame, world, timeToReset);
    else if (time >= timeToReset) {
      // TODO: Use different function for each scene or camera
      playerPad.locked = false;
      world.resetLevel();
      frame.pointLights.clear();
      InitializeLevelFromAsset(world, gval::firstLevel, frame, camera);
    }

#ifndef NDEBUG
    {
      /* --- Peformance log --- */
      static Timer       firstTime = time;
      static unsigned    frames    = 0;
      static const auto  oneSec    = std::chrono::milliseconds(1000);
      static gl::Buffer  textBuffer;
      static TextCommand textCommand = {*textBuffer, {}};

      /* ---------------------- */
      static Pristine FirstLoop;
      if (FirstLoop) {
        FillBuffer(toWString(frames, "FPS"), textCommand,
                   OpenGLDataInstance::Instance().textAtlas,
                   glm::vec2(viewportWidth, viewportHeight));
        SOLEIL__LOGGER_DEBUG(toString("SIZEOF OpenGLData: ",
                                      sizeof(OpenGLDataInstance::Instance())));
      }
      /* ---------------------- */

      frames++;
      if (time - firstTime > oneSec) {
        const auto duration = (time - firstTime) / frames;
        SOLEIL__LOGGER_DEBUG("Time to draw previous frame: ", duration,
                             " (FPS=", frames, ")");
        FillBuffer(toWString("TIME TO DRAW PREVIOUS FRAME: ", duration,
                             " (FPS=", frames, ")"),
                   textCommand, OpenGLDataInstance::Instance().textAtlas,
                   glm::vec2(viewportWidth, viewportHeight));
        firstTime = time;
        frames    = 0;
      }
      DrawText(
        textCommand,
        glm::scale(glm::translate(glm::mat4(), glm::vec3(-1.0f, 0.9f, 0.0f)),
                   glm::vec3(0.5f)),
        Color(0.8f));
#if 0
      glm::mat4 transformation =
        glm::scale(glm::translate(glm::mat4(), glm::vec3(-1.0f, -1.0f, 0.0f)),
                   glm::vec3(2.0f));
      DrawImage(*OpenGLDataInstance::Instance().textDefaultFontAtlas,
                transformation);
#endif
    }
#endif
  }

} // Soleil
