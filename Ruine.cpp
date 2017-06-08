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
#include "Drawable.hpp"
#include "Group.hpp"
#include "Logger.hpp"
#include "OpenGLDataInstance.hpp"
#include "Pristine.hpp"
#include "Shape.hpp"
#include "TypesToOStream.hpp"
#include "WavefrontLoader.hpp"
#include "types.hpp"

#include <cmath>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>

namespace Soleil {

  Ruine::Ruine(AssetService* assetService, SoundService* soundService,
               int viewportWidth, int viewportHeight)
    : assetService(assetService)
    , soundService(soundService)
    , viewportWidth(viewportWidth)
    , viewportHeight(viewportHeight)
  {
    OpenGLDataInstance::Initialize();
  }

  Ruine::~Ruine() {}

  static void renderSceneGraph(const Group& group, const Soleil::Frame& frame)
  {
    /* This is not the most efficient way to Render a Scene Graph but it will
     * make the job for two elements. Let's optimize that afterward. */
    for (NodePtr node : group) {
      auto drawable = std::dynamic_pointer_cast<Drawable>(node);
      if (drawable) {
        drawable->render(frame);
      }

      auto childGroup = std::dynamic_pointer_cast<Group>(node);
      if (childGroup) {
        renderSceneGraph(*childGroup, frame);
      }
    }
  }

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

    // static std::shared_ptr<Drawable> triangle;
    static Group group;

    static Pristine FirstLoop;
    if (FirstLoop) {
      // std::vector<Vertex> vertices = {
      //   Vertex(Point(-1.0, -1.0, 0.5, 1.0), Normal(1.0f),
      //          Color(1.0, 0.0, 0.0, 1.0)),
      //   Vertex(Point(1.0, -1.0, 0.5, 1.0), Normal(1.0f),
      //          Color(0.0, 1.0, 0.0, 1.0)),
      //   Vertex(Point(-1.0, 1.0, 0.5, 1.0), Normal(1.0f),
      //          Color(1.0, 1.0, 1.0, 1.0)),

      //   Vertex(Point(1.0, -1.0, 0.5, 1.0), Normal(1.0f),
      //          Color(0.0, 1.0, 0.0, 1.0)),
      //   Vertex(Point(1.0, 1.0, 0.5, 1.0), Normal(1.0f),
      //          Color(0.0, 0.0, 1.0, 1.0)),
      //   Vertex(Point(-1.0, 1.0, 0.5, 1.0), Normal(1.0f),
      //          Color(1.0, 1.0, 1.0, 1.0)),
      // };
      // ShapePtr shape = std::make_shared<Shape>(vertices);

      const std::string content = AssetService::LoadAsString("wallcube.obj");
      ShapePtr          shape   = WavefrontLoader::fromContent(content);
      ShapePtr          ball    = WavefrontLoader::fromContent(
        AssetService::LoadAsString("colored-ball.obj"));

      std::shared_ptr<Drawable> platform = std::make_shared<Drawable>(shape);
      std::shared_ptr<Group>    platformGroup = std::make_shared<Group>();
      // std::shared_ptr<Drawable> cube     = std::make_shared<Drawable>(shape);

      platform->setName("Platform");
      // cube->setName("Cube");
      platformGroup->setName("PlatformGroup");

      platform->setTransformation(
        glm::scale(glm::mat4(), glm::vec3(6.0f, 0.1f, 6.0f)));

      // platformGroup->addChild(cube);
      platformGroup->addChild(std::make_shared<Drawable>(ball));
      group.addChild(platform);
      group.addChild(platformGroup);

      SoundService::PlayMusic("fabulous.wav");
    }

    glClearColor(0.0f, 0.3f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static const glm::mat4 projection = glm::perspective(
      glm::radians(50.0f), (float)viewportWidth / (float)viewportHeight, 0.1f,
      50.0f);

    static float cameraRotationAngle = 0.1f;
    glm::vec3 cameraPosition = glm::vec3(
      glm::rotate(glm::mat4(), cameraRotationAngle, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::vec4(0.0f, 5.0f, 5.0f, 1.0f));
#if 0
    cameraRotationAngle += 0.0055f;
#endif
    
    glm::mat4 view =
      glm::lookAt(cameraPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    static Frame frame;
    frame.time           = time;
    frame.cameraPosition = cameraPosition;
    frame.updateViewProjectionMatrices(view, projection);

    glEnable(GL_DEPTH_TEST);
    // triangle->render(frame);

    /* Way to render the scene-graph. Optimization may follow */
    renderSceneGraph(group, frame);

    static float angle = 0.55f;

    /* The triangle is not centered */
    // static const glm::mat4 translation =
    //   glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -0.5f));
    // static const glm::mat4 inverseTranslation =
    //   glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.5f));

    /* The cube is already centered */
    static const glm::mat4 translation;
    static const glm::mat4 inverseTranslation;

    // Make it rotate on its own axis
    glm::mat4 transformation =
      glm::rotate(inverseTranslation, angle, glm::vec3(0.0f, 1.0f, 0.0f)) *
      translation;
#if 0

    angle += 0.1f;
    if (angle >= glm::two_pi<float>()) {
      SoundService::FireSound("woot.pcm", SoundProperties(100));
      angle = 0.0f;
    }
#endif

    group.setTransformation(transformation);
  }
} // Soleil
