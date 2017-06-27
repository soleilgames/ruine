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

  static void InitializeWorld(World&                    world,
                              std::vector<BoundingBox>& wallBoundingBox,
                              Frame& frame, Camera& camera)
  {
    std::string level;
    level += "xGxxxxxxxxxxxx\n";
    level += "xD.xGg.....xxx\n";
    level += "x..xxxxxx..xxx\n";
    level += "x..xx......xxx\n";
    level += "x..xx.......xx\n";
    level += "x..xx.......xx\n";
    level += "xg.xxxxxxx..xx\n";
    level += "x..xxxxxxx..xx\n";
    level += "x.......xx..xx\n";
    level += "x.......xx..xx\n";
    level += "x.......xxg.xx\n";
    level += "x......xxx..xx\n";
    level += "x..xxxxxxx..xx\n";
    level += "x............x\n";
    level += "x............x\n";
    level += "xxxxxxxxxxxxxx\n";

    InitializeWorldModels(world);
    InitializeLevel(world, level, wallBoundingBox, frame, camera);

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
    DrawPad();
  }

  static glm::mat4 updateCamera(Camera* camera, const glm::vec3& translation,
                                const float                     yaw,
                                const std::vector<BoundingBox>& wallBoundingBox,
                                const Timer&                    deltaTime)
  {
    camera->yaw += yaw * gval::cameraYawSpeed * deltaTime.count();

    const float speed = gval::cameraSpeed * deltaTime.count();

    // There is no pitch nor Roll, to the equation is simplified:
    glm::vec3 direction = glm::normalize(glm::vec3(
      sin(glm::radians(camera->yaw)), 0.0f, cos(glm::radians(camera->yaw))));
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
    bboxForward.expandBy(positionForward);
    BoundingBox bboxSideward;
    bboxSideward.expandBy(positionSideward);
    bboxSideward.expandBy(positionSideward + 0.15f);
    bboxSideward.expandBy(positionSideward - 0.15f);
    bboxSideward.expandBy(positionSideward);

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
#ifndef NDEBUG
    {
      /* --- Peformance log --- */
      static Timer      firstTime = time;
      static unsigned   frames    = 0;
      static const auto oneSec    = std::chrono::milliseconds(1000);

      frames++;
      if (time - firstTime > oneSec) {
        SOLEIL__LOGGER_DEBUG(toString("Time to draw previous frame: ",
                                      (time - firstTime) / frames),
                             " (FPS=", frames, ")");
        firstTime = time;
        frames    = 0;
      }
      /* ---------------------- */
      static Pristine FirstLoop;
      if (FirstLoop) {
        SOLEIL__LOGGER_DEBUG(toString("SIZEOF OpenGLData: ",
                                      sizeof(OpenGLDataInstance::Instance())));
      }
    }
#endif

    static Frame                    frame;
    static World                    world;
    static std::vector<BoundingBox> wallBoundingBox;
    static Pristine                 FirstLoop;

    if (FirstLoop) {
      InitializeWorld(world, wallBoundingBox, frame, camera);
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

    const Controller& playerPad = ControllerService::GetPlayerController();
    const glm::vec3   translation(0.0f, 0.0f, playerPad.dpad.z / 20.0f);
    const float       yaw = playerPad.dpad.x;
    this->view =
      updateCamera(&camera, translation, yaw, wallBoundingBox, frame.delta);

    frame.cameraPosition = camera.position;
    frame.updateViewProjectionMatrices(view, projection);
    frame.pointLights[0].position = camera.position;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    RenderScene(world, frame);
#if 0
    glm::mat4 transformation =
      glm::scale(glm::translate(glm::mat4(), glm::vec3(-1.0f, -1.0f, 0.0f)),
                 glm::vec3(2.0f));
    DrawImage(gval::bezierTex, transformation);
#endif
  }

} // Soleil
