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
#include "Drawable.hpp"
#include "Logger.hpp"
#include "Pristine.hpp"
#include "Shape.hpp"
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
    : triangle(0)
    , buffer(0)
    , assetService(assetService)
    , soundService(soundService)
    , viewportWidth(viewportWidth)
    , viewportHeight(viewportHeight)
  {
  }

  Ruine::~Ruine()
  {
    if (triangle > 0) {
      glDeleteProgram(triangle);
    }
  }

  void Ruine::render(Timer time)
  {
    static std::shared_ptr<Drawable> triangle;

    static Pristine FirstLoop;
    if (FirstLoop) {
      std::vector<Vertex> vertices = {
        Vertex(Point(-1.0, -1.0, 0.5, 1.0), Normal(1.0f),
               Color(1.0, 0.0, 0.0, 1.0)),
        Vertex(Point(1.0, -1.0, 0.5, 1.0), Normal(1.0f),
               Color(0.0, 1.0, 0.0, 1.0)),
        Vertex(Point(-1.0, 1.0, 0.5, 1.0), Normal(1.0f),
               Color(1.0, 1.0, 1.0, 1.0)),

	Vertex(Point(1.0, -1.0, 0.5, 1.0), Normal(1.0f),
               Color(0.0, 1.0, 0.0, 1.0)),
        Vertex(Point(1.0, 1.0, 0.5, 1.0), Normal(1.0f),
               Color(0.0, 0.0, 1.0, 1.0)),
        Vertex(Point(-1.0, 1.0, 0.5, 1.0), Normal(1.0f),
               Color(1.0, 1.0, 1.0, 1.0)),
      };

      ShapePtr shape = std::make_shared<Shape>(vertices);

      triangle = std::make_shared<Drawable>(shape);
    }

    glClearColor(0.0f, 0.3f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    static const glm::mat4 projection = glm::perspective(
      glm::radians(50.0f), (float)viewportWidth / (float)viewportHeight, 0.1f,
      10.0f);
    static const glm::mat4 view =
      glm::lookAt(glm::vec3(0.0f, 2.0f, 2.0f), glm::vec3(0.0f),
                  glm::vec3(0.0f, 1.0f, 0.0f));

    static Frame frame;
    frame.time           = time;
    frame.ViewProjection = projection * view;

    triangle->render(frame);

    static float angle = 0.1f;

    static const glm::mat4 translation =
      glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -0.5f));
    static const glm::mat4 inverseTranslation =
      glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.5f));

    // Make it rotate on its own axis
    glm::mat4 transformation =
      glm::rotate(inverseTranslation, angle, glm::vec3(0.0f, 1.0f, 0.0f)) *
      translation;
    angle += 0.1f;

    if (angle >= glm::two_pi<float>()) {
      SoundService::FireSound("woot.pcm", SoundProperties(100));
      angle = 0.0f;
    }

    triangle->setTransformation(transformation);
  }
} // Soleil
