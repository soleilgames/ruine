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

  struct DrawCommand
  {
    GLuint                       buffer;
    glm::mat4                    transformation;
    const std::vector<SubShape>& sub;

    DrawCommand(GLuint buffer, const glm::mat4& transformation,
                const std::vector<SubShape>& sub)
      : buffer(buffer)
      , transformation(transformation)
      , sub(sub)
    {
    }
  };

  typedef std::vector<DrawCommand> RenderInstances;

  struct GhostData
  {
    glm::mat4* transformation;
    glm::vec3* lightPosition;

    GhostData(glm::mat4* transformation, glm::vec3* lightPosition)
      : transformation(transformation)
      , lightPosition(lightPosition)
    {
    }
  };

  ShapePtr               wallShape;
  ShapePtr               floorShape;
  ShapePtr               torchShape;
  ShapePtr               ghostShape;
  RenderInstances        statics;
  std::vector<GhostData> moving;

  static void InitializeWorld(std::vector<BoundingBox>& wallBoundingBox,
                              Frame& frame, Camera& camera)
  {
    wallShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("wallcube.obj"));
    floorShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("floor.obj"));
    torchShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("brazero.obj"));
    ghostShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("ghost.obj"));
    std::vector<glm::mat4> scene;

    frame.pointLights.push_back(PointLight());
    frame.pointLights[0].color = glm::vec3(0.25f);

    RenderInstances lateComer;
    // Prepare vertices for the bounding box:
    std::vector<glm::vec4> wallVertices;
    for (const auto& sub : wallShape->getSubShapes()) {
      for (const auto& vertex : sub.vertices) {
        wallVertices.push_back(vertex.position);
      }
    }

    std::string level;
#if 1
    level += "xxxxxxxxxxxxxx\n";
    level += "xD.xx......xxx\n";
    level += "x..xxxxxx..xxx\n";
    level += "x..xx......xxx\n";
    level += "x..xxg.....xx\n";
    level += "x..xx.......xx\n";
    level += "x..xxxxxxx..xx\n";
    level += "x..xxxxxxx..xx\n";
    level += "x.......xx..xx\n";
    level += "x......xx..xx\n";
    level += "xg......xx..xx\n";
    level += "x......xxx..xx\n";
    level += "x..xxxxxxx..xx\n";
    level += "x.......g....x\n";
    level += "x............x\n";
    level += "xxxxxxxxxxxxxx\n";
#else
    level += "xxxx\n";
    level += "xD.x\n";
    level += "x..x\n";
    level += "x..x\n";
    level += "x..x\n";
    level += "x..x\n";
    level += "x..x\n";
    level += "x..x\n";
    level += "x..x\n";
    level += "x..x\n";
    level += "x..x\n";
    level += "x..x\n";
    level += "x..x\n";
    level += "x..x\n";
    level += "xg.x\n";
    level += "xxxx\n";
#endif

    std::istringstream     s(level);
    std::string            line;
    float                  x     = 0.0f;
    float                  z     = 0.0f;
    const static glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(1.0f));
    while (std::getline(s, line)) {
      for (auto const& c : line) {
        const glm::vec3 position(2.0f * x, 0.0f, 2.0f * z);

        if (c == 'x') {
          const glm::mat4 transformation = glm::translate(scale, position);

          statics.push_back(DrawCommand(wallShape->getBuffer(), transformation,
                                        wallShape->getSubShapes()));

          BoundingBox bbox;
          for (const auto& v : wallVertices) {
            bbox.expandBy(glm::vec3(transformation * v));
          }
          wallBoundingBox.push_back(bbox);

        } else if (c == 'D') {
          camera.position = glm::vec3(scale * glm::vec4(position, 1.0f));
        } else {
          if (c == 'l') {
            PointLight p;

            p.color    = glm::vec3(0.20f, 0.15f, 0.0f);
            p.position = glm::vec3(scale * glm::vec4(position, 1.0f));
            frame.pointLights.push_back(p);

            glm::mat4 transformation =
              glm::translate(glm::mat4(),
                             glm::vec3(position.x, -1.0f, position.z)) *
              glm::scale(glm::mat4(), glm::vec3(0.5f));
            statics.push_back(DrawCommand(torchShape->getBuffer(),
                                          transformation,
                                          torchShape->getSubShapes()));
          } else if (c == 'g') {
            PointLight p;

            p.color    = glm::vec3(0.20f, 0.15f, 0.0f);
            p.position = glm::vec3(scale * glm::vec4(position, 1.0f));
            frame.pointLights.push_back(p);

            glm::mat4 transformation =
              glm::translate(glm::mat4(),
                             glm::vec3(position.x, -1.0f, position.z)) *
              glm::scale(glm::mat4(), glm::vec3(.5f));
            lateComer.push_back(DrawCommand(ghostShape->getBuffer(),
                                            transformation,
                                            ghostShape->getSubShapes()));
          }

          glm::mat4 groundTransformation =
            glm::translate(scale, glm::vec3(2.0f * x, -1.0f, 2.0f * z));

          glm::mat4 ceilTransformation =
            glm::translate(scale, glm::vec3(2.0f * x, 1.0f, 2.0f * z)) *
            glm::rotate(glm::mat4(), glm::pi<float>(),
                        glm::vec3(0.0f, 0.0f, 1.0f));

          statics.push_back(DrawCommand(floorShape->getBuffer(),
                                        groundTransformation,
                                        floorShape->getSubShapes()));
          statics.push_back(DrawCommand(floorShape->getBuffer(),
                                        ceilTransformation,
                                        floorShape->getSubShapes()));
        }
        x += 1.0f;
      }
      z += 1.0f;
      x = 0.0f;
    }

    for (size_t i = 0; i < lateComer.size(); ++i) {
      const auto& o = lateComer[i];

      statics.push_back(o);

      // TODO: Warning - There may be more ligth than ghost
      moving.push_back(GhostData(&(statics.back().transformation),
                                 &(frame.pointLights[i + 1].position)));
    }

    SoundService::PlayMusic("farlands.ogg");
  }

  static inline void RenderDrawCommands(const RenderInstances instances,
                                        const Frame&          frame)
  {
    throwOnGlError();
    constexpr GLsizei         stride    = sizeof(Vertex);
    const OpenGLDataInstance& instance  = OpenGLDataInstance::Instance();
    const Program&            rendering = instance.drawable;
    glUseProgram(rendering.program);

    // Setting Lights
    // ----------------------------------------------------------
    glUniform3fv(instance.drawableAmbiantLight, 1,
                 glm::value_ptr(glm::vec3(.25f)));

    glUniform3fv(instance.drawableEyeDirection, 1,
                 glm::value_ptr(frame.cameraPosition));
    glUniform1i(instance.drawableNumberOfLights, frame.pointLights.size());
    throwOnGlError();

    int i = 0;
    for (const auto& pointLight : frame.pointLights) {
      // TODO: Protect to avoid array verflow

      glUniform3fv(instance.drawablePointLights[i].position, 1,
                   glm::value_ptr(pointLight.position));
      glUniform3fv(instance.drawablePointLights[i].color, 1,
                   glm::value_ptr(pointLight.color));
#if 0
      glUniform3fv(instance.drawablePointLights[i].color, 1,
                   glm::value_ptr(glm::vec3(0.196f, 0.092f, 0.0f)));
#endif
      glUniform1f(instance.drawablePointLights[i].linearAttenuation, 0.22);
      glUniform1f(instance.drawablePointLights[i].quadraticAttenuation, 0.07);

      ++i;
    }

    for (const auto& drawCommand : instances) {

      gl::BindBuffer bindBuffer(GL_ARRAY_BUFFER, drawCommand.buffer);

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

#if 0
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#else
      if (ControllerService::GetPlayerController().option1)
        glEnable(GL_BLEND);
      else
        glDisable(GL_BLEND);
#endif
      for (const auto& sub : drawCommand.sub) {
        // Setting Materials
        // -------------------------------------------------------
        glUniform3fv(instance.drawableMaterial.ambiantColor, 1,
                     glm::value_ptr(sub.material.ambiantColor));
        glUniform1f(instance.drawableMaterial.shininess,
                    sub.material.shininess);
        glUniform3fv(instance.drawableMaterial.emissiveColor, 1,
                     glm::value_ptr(sub.material.emissiveColor));
        glUniform3fv(instance.drawableMaterial.diffuseColor, 1,
                     glm::value_ptr(sub.material.diffuseColor));
        glUniform3fv(instance.drawableMaterial.specularColor, 1,
                     glm::value_ptr(sub.material.specularColor));

        // Setting Textures
        // --------------------------------------------------------
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sub.material.diffuseMap);
        glUniform1i(instance.drawableMaterial.diffuseMap, 0);

        auto ViewProjectionModel =
          frame.ViewProjection * drawCommand.transformation;
        glUniformMatrix4fv(instance.drawableMVPMatrix, 1, GL_FALSE,
                           glm::value_ptr(ViewProjectionModel));

/* The book has a mistake, it says using a MVMatrix while only using the Model
 * Matrix*/
#if 0
    auto ModelView = frame.View * getTransformation();
#endif
        glUniformMatrix4fv(
          instance.drawableMVMatrix, 1, GL_FALSE,
          glm::value_ptr(drawCommand.transformation)); // ModelView

/* TODO: If only rotation and isometric (nonshape changing) scaling was
 * performed, the Mat3 is should be fine: */
#if 0
    auto NormalMatrix = glm::mat3(ModelView);
#endif

        auto NormalMatrix =
          glm::transpose(glm::inverse(glm::mat3(drawCommand.transformation)));

        glUniformMatrix3fv(instance.drawableNormalMatrix, 1, GL_FALSE,
                           glm::value_ptr(NormalMatrix));
        throwOnGlError();

        const std::vector<GLushort>& indices = sub.indices;
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT,
                       indices.data());
      }
    }
  }

  static void UpdateTorchColor(std::vector<PointLight>& lights)
  {
    for (unsigned int i = 1; i < lights.size(); ++i) {
      int r                          = std::rand() % 65;
      int doit                       = std::rand() % 100;
      if (doit > 50) lights[i].color = glm::vec3(r / 255.0f);
    }
  }

  static void UpdateGhost(std::vector<GhostData>& ghosts)
  {
    static int   frame = 0;
    static float dir   = -1;
    frame++;

    // TODO: Just a quick way to move ghost. Should not be frame fixed ;)
    if (frame > 60 * 10) {
      frame = 0;
      dir   = -dir;
    }

    for (GhostData& ghost : ghosts) {
      const glm::mat4 transformation = glm::translate(
        *(ghost.transformation), glm::vec3(0.0f, 0.0f, dir * 0.1f));
      *(ghost.transformation) = transformation;

      *(ghost.lightPosition) = glm::vec3(transformation[3]);
    }
  }

  static void RenderScene(Frame& frame)
  {
    UpdateGhost(moving);
#if 0
    UpdateTorchColor(frame.pointLights);
    RenderDrawable(ground, frame, floorShape.get());
    RenderDrawable(wall, frame, wallShape.get());
#endif
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    RenderDrawCommands(statics, frame);
    DrawPad();
  }

  glm::mat4 updateCamera(Camera* camera, const glm::vec3& translation,
                         const float                     yaw,
                         const std::vector<BoundingBox>& wallBoundingBox)
  {
    camera->yaw += yaw;

    // There is no pitch nor Roll, to the equation is simplified:
    glm::vec3 direction = glm::normalize(glm::vec3(
      sin(glm::radians(camera->yaw)), 0.0f, cos(glm::radians(camera->yaw))));
    glm::vec3 newPosition = camera->position + (translation.z * direction);
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

    static Frame frame;

    static std::vector<BoundingBox> wallBoundingBox;

    static Pristine FirstLoop;
    if (FirstLoop) {
      InitializeWorld(wallBoundingBox, frame, camera);
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

    const Controller& playerPad = ControllerService::GetPlayerController();
    const glm::vec3   translation(0.0f, 0.0f, playerPad.dpad.z / 20.0f);
    const float       yaw = playerPad.dpad.x;
    this->view = updateCamera(&camera, translation, yaw, wallBoundingBox);

    frame.time           = time;
    frame.cameraPosition = camera.position;
    frame.updateViewProjectionMatrices(view, projection);
    frame.pointLights[0].position = camera.position;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    RenderScene(frame);
  }

} // Soleil
