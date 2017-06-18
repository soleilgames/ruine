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
#include "ControllerService.hpp"
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
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>

namespace Soleil {

  static void DrawPad()
  {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const OpenGLDataInstance& ogl = OpenGLDataInstance::Instance();
    throwOnGlError();

    glUseProgram(ogl.imageProgram.program);
    throwOnGlError();
    glBindBuffer(GL_ARRAY_BUFFER, *(ogl.imageBuffer));
    throwOnGlError();

    constexpr GLsizei stride = sizeof(GLfloat) * 2;

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)0);
    glEnableVertexAttribArray(0);
    throwOnGlError();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *ogl.texturePad);
    glUniform1i(ogl.imageImage, 0);

    const glm::mat4 modelMatrix =
      glm::scale(glm::translate(glm::mat4(), glm::vec3(-1.0f, -1.0f, 0.0f)),
                 glm::vec3(0.5625f, 1.0f, 1.0f)); // TODO: Static
    glUniformMatrix4fv(ogl.imageModelMatrix, 1, GL_FALSE,
                       glm::value_ptr(modelMatrix));
    throwOnGlError();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    throwOnGlError();
  }

#ifdef SOLEIL__OOP_SG
  static void InitializeWorld(Group& group, Frame& frame, Camera& camera)
  {
    const std::string content = AssetService::LoadAsString("wallcube.obj");
    ShapePtr          shape   = WavefrontLoader::fromContent(content);
    ShapePtr          floorShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("floor.obj"));

    std::string level;
    level += "xxxxxxxxxxxxxx\n";
    level += "xD.xx......xxx\n";
    level += "x..xxxxxx..xxx\n";
    level += "x..xx......xxx\n";
    level += "x..xx.......xx\n";
    level += "x..xx.......xx\n";
    level += "x..xxxxxxx..xx\n";
    level += "x..xxxxxxx..xx\n";
    level += "x.......xx..xx\n";
    level += "x.......xx..xx\n";
    level += "x.......xx..xx\n";
    level += "x......xxx..xx\n";
    level += "x..xxxxxxx..xx\n";
    level += "x............x\n";
    level += "x............x\n";
    level += "xxxxxxxxxxxxxx\n";

    std::istringstream     s(level);
    std::string            line;
    float                  x     = 0.0f;
    float                  z     = 0.0f;
    const static glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(0.4f));
    while (std::getline(s, line)) {
      for (auto const& c : line) {
        const glm::vec3 position(2.0f * x, 0.0f, 2.0f * z);

        if (c == 'x') {
          auto wall = std::make_shared<Drawable>(shape);
          wall->setTransformation(glm::translate(scale, position));
          group.addChild(wall);
        } else if (c == 'D') {
          camera.position = glm::vec3(scale * glm::vec4(position, 1.0f));
        } else {
          auto ground = std::make_shared<Drawable>(floorShape);
          ground->setTransformation(
            glm::translate(scale, glm::vec3(2.0f * x, -1.0f, 2.0f * z)));
          group.addChild(ground);

          auto ceil = std::make_shared<Drawable>(floorShape);
          ceil->setTransformation(
            glm::translate(scale, glm::vec3(2.0f * x, 1.0f, 2.0f * z)) *
            glm::rotate(glm::mat4(), glm::pi<float>(),
                        glm::vec3(0.0f, 0.0f, 1.0f)));
          group.addChild(ceil);
        }
        x += 1.0f;
      }
      z += 1.0f;
      x = 0.0f;
    }

    SoundService::PlayMusic("farlands.ogg");
    frame.pointLights.push_back(PointLight());
  }

  static void RenderScene(const Group& group, Soleil::Frame& frame)
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

    DrawPad();
  }

#else

  typedef std::vector<glm::mat4> ObjectInstances;

  // TODO: Set in struct such as:
  // struct
  // {
  //   Shape shape;
  //   ObjectInstances
  // };
  ShapePtr wallShape;
  ShapePtr floorShape;

  static void InitializeWorld(ObjectInstances& ground, ObjectInstances& wall,
                              Frame& frame, Camera& camera)
  {
    wallShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("wallcube.obj"));
    floorShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("floor.obj"));
    std::vector<glm::mat4> scene;

    std::string level;
    level += "xxxxxxxxxxxxxx\n";
    level += "xD.xx......xxx\n";
    level += "x..xxxxxx..xxx\n";
    level += "x..xx......xxx\n";
    level += "x..xx.......xx\n";
    level += "x..xx.......xx\n";
    level += "x..xxxxxxx..xx\n";
    level += "x..xxxxxxx..xx\n";
    level += "x.......xx..xx\n";
    level += "x.......xx..xx\n";
    level += "x.......xx..xx\n";
    level += "x......xxx..xx\n";
    level += "x..xxxxxxx..xx\n";
    level += "x............x\n";
    level += "x............x\n";
    level += "xxxxxxxxxxxxxx\n";

    std::istringstream     s(level);
    std::string            line;
    float                  x     = 0.0f;
    float                  z     = 0.0f;
    const static glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(0.4f));
    while (std::getline(s, line)) {
      for (auto const& c : line) {
        const glm::vec3 position(2.0f * x, 0.0f, 2.0f * z);

        if (c == 'x') {
          wall.push_back(glm::translate(scale, position));
        } else if (c == 'D') {
          camera.position = glm::vec3(scale * glm::vec4(position, 1.0f));
        } else {
          ground.push_back(
            glm::translate(scale, glm::vec3(2.0f * x, -1.0f, 2.0f * z)));

          ground.push_back(
            glm::translate(scale, glm::vec3(2.0f * x, 1.0f, 2.0f * z)) *
            glm::rotate(glm::mat4(), glm::pi<float>(),
                        glm::vec3(0.0f, 0.0f, 1.0f)));
        }
        x += 1.0f;
      }
      z += 1.0f;
      x = 0.0f;
    }

    SoundService::PlayMusic("farlands.ogg");
    frame.pointLights.push_back(PointLight());
  }

  static inline void RenderDrawable(const ObjectInstances instances,
                                    const Frame& frame, const Shape* shape)
  {
    constexpr GLsizei         stride    = sizeof(Vertex);
    const OpenGLDataInstance& instance  = OpenGLDataInstance::Instance();
    const Program&            rendering = instance.drawable;

    gl::BindBuffer bindBuffer(GL_ARRAY_BUFFER, shape->getBuffer());
    glBindBuffer(GL_ARRAY_BUFFER, shape->getBuffer());
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          (const GLvoid*)offsetof(Vertex, normal));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride,
                          (const GLvoid*)offsetof(Vertex, color));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride,
                          (const GLvoid*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glUseProgram(rendering.program);

    // Setting Lights ----------------------------------------------------------
    glUniform3fv(instance.drawableAmbiantLight, 1,
                 glm::value_ptr(glm::vec3(.0f)));

    glUniform3fv(instance.drawableEyeDirection, 1,
                 glm::value_ptr(frame.cameraPosition));
    glUniform1i(instance.drawableNumberOfLights, 1);

#if 0    
    static float angle = 0.0f;

    glm::vec4 lpos =
      glm::rotate(glm::mat4(), angle, glm::vec3(0.0f, 1.0f, 0.0f)) *
      glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

    angle += 0.0050f;

        glUniform3fv(instance.drawablePointLights[0].position, 1,
                 glm::value_ptr(lpos));
#else

    glUniform3fv(instance.drawablePointLights[0].position, 1,
                 glm::value_ptr(frame.pointLights[0].position));
#endif

    glUniform3fv(instance.drawablePointLights[0].color, 1,
                 glm::value_ptr(glm::vec3(0.2f)));
    glUniform1f(instance.drawablePointLights[0].linearAttenuation, 0.22);
    glUniform1f(instance.drawablePointLights[0].quadraticAttenuation, 0.07);

    // Second Light (on the camera):
    glUniform3fv(instance.drawablePointLights[1].position, 1,
                 glm::value_ptr(frame.cameraPosition));

    glUniform3fv(instance.drawablePointLights[1].color, 1,
                 glm::value_ptr(glm::vec3(0.2f)));
    glUniform1f(instance.drawablePointLights[1].linearAttenuation, 0.22);
    glUniform1f(instance.drawablePointLights[1].quadraticAttenuation, 0.20);

    throwOnGlError();

    // glEnable(GL_POLYGON_SMOOTH);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (const auto& sub : shape->getSubShapes()) {
      // Setting Materials
      // -------------------------------------------------------
      glUniform3fv(instance.drawableMaterial.ambiantColor, 1,
                   glm::value_ptr(sub.material.ambiantColor));
      glUniform1f(instance.drawableMaterial.shininess, sub.material.shininess);
      glUniform3fv(instance.drawableMaterial.emissiveColor, 1,
                   glm::value_ptr(sub.material.emissiveColor));
      glUniform3fv(instance.drawableMaterial.diffuseColor, 1,
                   glm::value_ptr(sub.material.diffuseColor));
      glUniform3fv(instance.drawableMaterial.specularColor, 1,
                   glm::value_ptr(sub.material.specularColor));

      throwOnGlError();

      // Setting Textures
      // --------------------------------------------------------
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, sub.material.diffuseMap);
      glUniform1i(instance.drawableMaterial.diffuseMap, 0);

      for (const auto& transformation : instances) {

        auto ViewProjectionModel = frame.ViewProjection * transformation;
        glUniformMatrix4fv(instance.drawableMVPMatrix, 1, GL_FALSE,
                           glm::value_ptr(ViewProjectionModel));

/* The book has a mistake, it says using a MVMatrix while only using the Model
 * Matrix*/
#if 0
    auto ModelView = frame.View * getTransformation();
#endif
        glUniformMatrix4fv(instance.drawableMVMatrix, 1, GL_FALSE,
                           glm::value_ptr(transformation)); // ModelView

        throwOnGlError();

/* TODO: If only rotation and isometric (nonshape changing) scaling was
 * performed, the Mat3 is should be fine: */
#if 0
    auto NormalMatrix = glm::mat3(ModelView);
#endif

        auto NormalMatrix =
          glm::transpose(glm::inverse(glm::mat3(transformation)));

        glUniformMatrix3fv(instance.drawableNormalMatrix, 1, GL_FALSE,
                           glm::value_ptr(NormalMatrix));
        throwOnGlError();

        const std::vector<GLushort>& indices = sub.indices;
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT,
                       indices.data());
      }
    }

    throwOnGlError();
  }

  static void RenderScene(ObjectInstances& ground, ObjectInstances& wall,
                          Frame& frame)
  {
    RenderDrawable(ground, frame, floorShape.get());
    RenderDrawable(wall, frame, wallShape.get());
    DrawPad();
  }

#endif

  Ruine::Ruine(AssetService* assetService, SoundService* soundService,
               int viewportWidth, int viewportHeight)
    : assetService(assetService)
    , soundService(soundService)
    , viewportWidth(viewportWidth)
    , viewportHeight(viewportHeight)
  {
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

    static Frame frame;

#ifdef SOLEIL__OOP_SG
    static Group group;

    static Pristine FirstLoop;
    if (FirstLoop) {
      InitializeWorld(group, frame, camera);
    }
#else
    static ObjectInstances ground;
    static ObjectInstances wall;

    static Pristine FirstLoop;
    if (FirstLoop) {
      InitializeWorld(ground, wall, frame, camera);
    }
#endif

#if 0
    glClearColor(0.0f, 0.3f, 0.7f, 1.0f);
#else
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#endif
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static const glm::mat4 projection = glm::perspective(
      glm::radians(50.0f), (float)viewportWidth / (float)viewportHeight, 0.1f,
      50.0f);

    const Controller& playerPad = ControllerService::GetPlayerController();
    const glm::vec3   translation(0.0f, 0.0f, playerPad.dpad.z / 100.0f);
    const float       yaw = playerPad.dpad.x;
    this->view            = updateCamera(&camera, translation, yaw);

    frame.time           = time;
    frame.cameraPosition = camera.position;
    frame.updateViewProjectionMatrices(view, projection);
    frame.pointLights[0].position = camera.position;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

#ifdef SOLEIL__OOP_SG
    /* Way to render the scene-graph. Optimization may follow */
    RenderScene(group, frame);
#else
    RenderScene(ground, wall, frame);
#endif
  }

  glm::mat4 updateCamera(Camera* camera, const glm::vec3& translation,
                         const float yaw)
  {
    camera->yaw += yaw;

    // There is no pitch nor Roll, to the equation is simplified:
    glm::vec3 direction = glm::normalize(glm::vec3(
      sin(glm::radians(camera->yaw)), 0.0f, cos(glm::radians(camera->yaw))));
    camera->position += (translation.z * direction);
    return glm::lookAt(camera->position, camera->position + direction,
                       glm::vec3(0, 1, 0));
  }

} // Soleil
