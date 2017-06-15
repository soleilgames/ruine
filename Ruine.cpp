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

  static void renderSceneGraph(const Group& group, Soleil::Frame& frame)
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
    static Frame frame;

    static Pristine FirstLoop;
    if (FirstLoop) {
      const std::string content = AssetService::LoadAsString("wallcube.obj");
      ShapePtr          shape   = WavefrontLoader::fromContent(content);
      ShapePtr          floorShape =
        WavefrontLoader::fromContent(AssetService::LoadAsString("floor.obj"));

      // for (int i = 0; i < 10; ++i) {
      //   const static glm::mat4 scale = glm::scale(glm::mat4(),
      //   glm::vec3(0.4f));

      //   {
      //     auto wall = std::make_shared<Drawable>(shape);
      //     wall->setTransformation(
      //       glm::translate(scale, glm::vec3(2.0f, 0.0f, -2.0f * i)));
      //     group.addChild(wall);
      //   }

      //   {
      //     auto wall = std::make_shared<Drawable>(shape);
      //     wall->setTransformation(
      //       glm::translate(scale, glm::vec3(-2.0f, 0.0f, -2.0f * i)));
      //     group.addChild(wall);
      //   }
      // }

      std::string level;
      level += "xxxxxx\n";
      level += "x....x\n";
      level += "x....x\n";
      level += "x....x\n";
      level += "x....x\n";
      level += "x..xxx\n";
      level += "x..xxx\n";
      level += "xxxxxx\n";
      level += "xxxxxx\n";
      level += "xxxxxx\n";

      std::istringstream     s(level);
      std::string            line;
      float                  x     = 0.0f;
      float                  z     = 0.0f;
      const static glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(0.4f));
      while (std::getline(s, line)) {
        for (auto const& c : line) {
          if (c == 'x') {
            auto wall = std::make_shared<Drawable>(shape);
            wall->setTransformation(
              glm::translate(scale, glm::vec3(x, 0.0f, 2.0f * z)));
            group.addChild(wall);
          } else {
            auto ground = std::make_shared<Drawable>(floorShape);
            ground->setTransformation(
              glm::translate(scale, glm::vec3(x, -1.0f, 2.0f * z)));
            group.addChild(ground);

            auto ceil = std::make_shared<Drawable>(floorShape);
            ceil->setTransformation(
              glm::translate(scale, glm::vec3(x, 1.0f, 2.0f * z)) *
              glm::rotate(glm::mat4(), glm::pi<float>(),
                          glm::vec3(0.0f, 0.0f, 1.0f)));
            group.addChild(ceil);
          }
          x += 1.0f;
        }
        z += 1.0f;
        x = 0.0f;
      }

      SoundService::PlayMusic("fabulous.wav");
      frame.pointLights.push_back(PointLight());
    }

    glClearColor(0.0f, 0.3f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static const glm::mat4 projection = glm::perspective(
      glm::radians(50.0f), (float)viewportWidth / (float)viewportHeight, 0.1f,
      50.0f);

    static float cameraRotationAngle = 0.0f;
    glm::vec3    cameraPosition =
      glm::vec3(glm::rotate(glm::mat4(), cameraRotationAngle,
                            glm::vec3(0.0f, 1.0f, 0.0f)) *
                glm::vec4(2.0f *0.4f, .10f, 2.0f * 0.4f, 1.0f));
#if 0
    cameraRotationAngle += 0.0155f;
#endif

    glm::mat4 view = glm::lookAt(cameraPosition, glm::vec3(2.0f *0.4f, .10f, 3.0f * 0.4f),
                                 glm::vec3(0.0f, 1.0f, 0.0f));

    frame.time           = time;
    frame.cameraPosition = cameraPosition;
    frame.updateViewProjectionMatrices(view, projection);

    glEnable(GL_DEPTH_TEST);

    /* Way to render the scene-graph. Optimization may follow */
    renderSceneGraph(group, frame);
  }
} // Soleil
