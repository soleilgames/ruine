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

#include "Draw.hpp"

#include "ControllerService.hpp"
#include "OpenGLDataInstance.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Soleil {

  void DrawImage(GLuint texture, const glm::mat4& transformation,
                 const glm::vec4& color)
  {
    throwOnGlError();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    throwOnGlError();

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
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(ogl.imageImage, 0);

    glUniformMatrix4fv(ogl.imageModelMatrix, 1, GL_FALSE,
                       glm::value_ptr(transformation));
    glUniform4fv(ogl.imageColor, 1, glm::value_ptr(color));
    throwOnGlError();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    throwOnGlError();
  }

  void DrawText(const TextCommand& textCommand, const glm::mat4& transformation,
                const glm::vec4& color)
  {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const OpenGLDataInstance& ogl = OpenGLDataInstance::Instance();

    glUseProgram(ogl.textProgram.program);
    glBindBuffer(GL_ARRAY_BUFFER, *textCommand.buffer);

    const GLsizei stride = sizeof(CharVertex);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                          (const GLvoid*)offsetof(CharVertex, uv));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *(ogl.textDefaultFontAtlas));
    glUniform1i(ogl.textTexture, 0);

    glUniformMatrix4fv(ogl.textModelMatrix, 1, GL_FALSE,
                       glm::value_ptr(transformation));

    glUniform4fv(ogl.textColor, 1, glm::value_ptr(color));

    glDrawElements(GL_TRIANGLES, textCommand.elements.size(), GL_UNSIGNED_SHORT,
                   textCommand.elements.data());
    throwOnGlError();
  }

  void RenderPhongShape(const RenderInstances& instances, const Frame& frame)
  {
    throwOnGlError();
    constexpr GLsizei         stride    = sizeof(Vertex);
    const OpenGLDataInstance& instance  = OpenGLDataInstance::Instance();
    const Program&            rendering = instance.drawable;
    glUseProgram(rendering.program);

    // Setting Lights
    // ----------------------------------------------------------
    glUniform3fv(instance.drawableAmbiantLight, 1,
                 glm::value_ptr(glm::vec3(.05f)));

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
      glUniform1f(instance.drawablePointLights[i].linearAttenuation, 0.7f);
      glUniform1f(instance.drawablePointLights[i].quadraticAttenuation,
                  0.2f);

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
      for (const auto& sub : *drawCommand.sub) {
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

  void RenderFlatShape(const RenderInstances& instances, const Frame& frame)
  {
    throwOnGlError();
    constexpr GLsizei         stride    = sizeof(Vertex);
    const OpenGLDataInstance& instance  = OpenGLDataInstance::Instance();
    const Program&            rendering = instance.flat.program;
    glUseProgram(rendering.program);

    // Setting Lights
    // ----------------------------------------------------------
    glUniform3fv(instance.flat.AmbiantLight, 1,
                 glm::value_ptr(gval::ambiantLight));

    glUniform3fv(instance.flat.EyeDirection, 1,
                 glm::value_ptr(frame.cameraPosition));
    glUniform1i(instance.flat.NumberOfLights, frame.pointLights.size());
    throwOnGlError();

    int i = 0;
    assert(frame.pointLights.size() < DefinedMaxLights &&
           "Max number of light exceeded");
    for (const auto& pointLight : frame.pointLights) {
      // TODO: Protect to avoid array verflow

      glUniform3fv(instance.flat.PointLights[i].position, 1,
                   glm::value_ptr(pointLight.position));
      glUniform3fv(instance.flat.PointLights[i].color, 1,
                   glm::value_ptr(pointLight.color));
      glUniform1f(instance.flat.PointLights[i].linearAttenuation,
                  pointLight.linear);
      glUniform1f(instance.flat.PointLights[i].quadraticAttenuation,
                  pointLight.quadratic);

      ++i;
    }
    throwOnGlError();

    for (const auto& drawCommand : instances) {
      gl::BindBuffer bindBuffer(
        GL_ARRAY_BUFFER, drawCommand.buffer); // TODO: use glBind (check perf)
      throwOnGlError();

      glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)0);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                            (const GLvoid*)offsetof(Vertex, normal));
      glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride,
                            (const GLvoid*)offsetof(Vertex, color));
      glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride,
                            (const GLvoid*)offsetof(Vertex, uv));
      throwOnGlError();
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      glEnableVertexAttribArray(2);
      glEnableVertexAttribArray(3);
      throwOnGlError();

      if (ControllerService::GetPlayerController().option1)
        glEnable(GL_BLEND);
      else
        glDisable(GL_BLEND);
      for (const auto& sub : *drawCommand.sub) {
        // Setting Materials
        // -------------------------------------------------------
        glUniform3fv(instance.flat.Material.ambiantColor, 1,
                     glm::value_ptr(sub.material.ambiantColor));
        glUniform1f(instance.flat.Material.shininess, sub.material.shininess);
        glUniform3fv(instance.flat.Material.emissiveColor, 1,
                     glm::value_ptr(sub.material.emissiveColor));
        glUniform3fv(instance.flat.Material.diffuseColor, 1,
                     glm::value_ptr(sub.material.diffuseColor));
        glUniform3fv(instance.flat.Material.specularColor, 1,
                     glm::value_ptr(sub.material.specularColor));
        throwOnGlError();

        // Setting Textures
        // --------------------------------------------------------
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sub.material.diffuseMap);
        glUniform1i(instance.flat.Material.diffuseMap, 0);
        throwOnGlError();

        auto ViewProjectionModel =
          frame.ViewProjection * drawCommand.transformation;
        glUniformMatrix4fv(instance.flat.MVPMatrix, 1, GL_FALSE,
                           glm::value_ptr(ViewProjectionModel));

/* The book has a mistake, it says using a MVMatrix while only using the Model
 * Matrix*/
#if 0
    auto ModelView = frame.View * drawCommand.transformation;
#endif
        glUniformMatrix4fv(
          instance.flat.MVMatrix, 1, GL_FALSE,
          glm::value_ptr(drawCommand.transformation)); // ModelView

/* TODO: If only rotation and isometric (nonshape changing) scaling was
 * performed, the Mat3 is should be fine: */
#if 0
    auto NormalMatrix = glm::mat3(ModelView);
#endif

        auto NormalMatrix =
          glm::transpose(glm::inverse(glm::mat3(drawCommand.transformation)));
        // TODO: Allow Draw command to have its transpose to save this
        // computaiton

        glUniformMatrix3fv(instance.flat.NormalMatrix, 1, GL_FALSE,
                           glm::value_ptr(NormalMatrix));
        const std::vector<GLushort>& indices = sub.indices;
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT,
                       indices.data());
        throwOnGlError();
      }
    }
  }

  void RenderFlatShape(const glm::mat4& transformation, const Shape& shape,
                       const Frame& frame)
  {
    // TODO: Merge the code with method above
    throwOnGlError();
    constexpr GLsizei         stride    = sizeof(Vertex);
    const OpenGLDataInstance& instance  = OpenGLDataInstance::Instance();
    const Program&            rendering = instance.flat.program;
    glUseProgram(rendering.program);

    // Setting Lights
    // ----------------------------------------------------------
    glUniform3fv(instance.flat.AmbiantLight, 1,
                 glm::value_ptr(gval::ambiantLight));

    glUniform3fv(instance.flat.EyeDirection, 1,
                 glm::value_ptr(frame.cameraPosition));
    glUniform1i(instance.flat.NumberOfLights, frame.pointLights.size());
    throwOnGlError();

    int i = 0;
    assert(frame.pointLights.size() < DefinedMaxLights &&
           "Max number of light exceeded");
    for (const auto& pointLight : frame.pointLights) {
      // TODO: Protect to avoid array verflow

      glUniform3fv(instance.flat.PointLights[i].position, 1,
                   glm::value_ptr(pointLight.position));
      glUniform3fv(instance.flat.PointLights[i].color, 1,
                   glm::value_ptr(pointLight.color));
      glUniform1f(instance.flat.PointLights[i].linearAttenuation,
                  pointLight.linear);
      glUniform1f(instance.flat.PointLights[i].quadraticAttenuation,
                  pointLight.quadratic);

      ++i;
    }
    throwOnGlError();

    gl::BindBuffer bindBuffer(
      GL_ARRAY_BUFFER, shape.getBuffer()); // TODO: use glBind (check perf)
    throwOnGlError();

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          (const GLvoid*)offsetof(Vertex, normal));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride,
                          (const GLvoid*)offsetof(Vertex, color));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride,
                          (const GLvoid*)offsetof(Vertex, uv));
    throwOnGlError();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    throwOnGlError();

    if (ControllerService::GetPlayerController().option1)
      glEnable(GL_BLEND);
    else
      glDisable(GL_BLEND);
    for (const auto& sub : shape.getSubShapes()) {
      // Setting Materials
      // -------------------------------------------------------
      glUniform3fv(instance.flat.Material.ambiantColor, 1,
                   glm::value_ptr(sub.material.ambiantColor));
      glUniform1f(instance.flat.Material.shininess, sub.material.shininess);
      glUniform3fv(instance.flat.Material.emissiveColor, 1,
                   glm::value_ptr(sub.material.emissiveColor));
      glUniform3fv(instance.flat.Material.diffuseColor, 1,
                   glm::value_ptr(sub.material.diffuseColor));
      glUniform3fv(instance.flat.Material.specularColor, 1,
                   glm::value_ptr(sub.material.specularColor));
      throwOnGlError();

      // Setting Textures
      // --------------------------------------------------------
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, sub.material.diffuseMap);
      glUniform1i(instance.flat.Material.diffuseMap, 0);
      throwOnGlError();

      auto ViewProjectionModel = frame.ViewProjection * transformation;
      glUniformMatrix4fv(instance.flat.MVPMatrix, 1, GL_FALSE,
                         glm::value_ptr(ViewProjectionModel));

/* The book has a mistake, it says using a MVMatrix while only using the Model
 * Matrix*/
#if 0
    auto ModelView = frame.View * drawCommand.transformation;
#endif
      glUniformMatrix4fv(instance.flat.MVMatrix, 1, GL_FALSE,
                         glm::value_ptr(transformation)); // ModelView

/* TODO: If only rotation and isometric (nonshape changing) scaling was
 * performed, the Mat3 is should be fine: */
#if 0
    auto NormalMatrix = glm::mat3(ModelView);
#endif

      auto NormalMatrix =
        glm::transpose(glm::inverse(glm::mat3(transformation)));
      // TODO: Allow Draw command to have its transpose to save this computaiton

      glUniformMatrix3fv(instance.flat.NormalMatrix, 1, GL_FALSE,
                         glm::value_ptr(NormalMatrix));
      const std::vector<GLushort>& indices = sub.indices;
      glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT,
                     indices.data());
      throwOnGlError();
    }
  }

  void Fade(const float ratio)
  {

    glm::vec4 color(0.0f, 0.0f, 0.0f, ratio);

    DrawImage(
      *OpenGLDataInstance::Instance().textureBlack,
      glm::scale(glm::translate(glm::mat4(), glm::vec3(-1.0f, -1.0f, 0.0f)),
                 glm::vec3(2.0f, 2.0f, 1.0f)),
      color);
  }

  // ------ Popup ------

  void PopUp::fillText(const std::wstring& text, const float em)
  {
    Text::FillBuffer(text, label, OpenGLDataInstance::Instance().textAtlas, em);
  }

  void PopUp::activate(const Timer& timeToFadeOut, const Timer& startTime)
  {
    this->fadeOut   = timeToFadeOut.count();
    this->active    = true;
    this->startTime = startTime;
  }

  PopUp::PopUp(void)
    : label()
    , fadeOut(0)
    , active(false)
  {
  }

  void PopUp::render(Timer time)
  {
    long alpha = (time - startTime).count();

    if (alpha > fadeOut) {
      active = false;
      return;
    }

    // TODO: Use Bezier Curve
    Color color =
      glm::mix(Color(0.8f), Color(0.0f), (float)alpha / (float)fadeOut);

    DrawText(label, transformation, color);
  }

  void DrawBoundingBox(const BoundingBox& box, const Frame& frame,
                       const glm::vec4& color)
  {
    constexpr GLsizei       stride    = sizeof(GLfloat) * 3;
    const BoundingBoxShape& shape     = OpenGLDataInstance::Instance().box;
    const Program&          rendering = shape.program;

    glUseProgram(rendering.program);
    glBindBuffer(GL_ARRAY_BUFFER, *shape.buffer);
    glUniform3fv(shape.min, 1, glm::value_ptr(box.getMin()));
    glUniform3fv(shape.max, 1, glm::value_ptr(box.getMax()));
    glUniform4fv(shape.color, 1, glm::value_ptr(color)); // TODO: Add color;
    glUniformMatrix4fv(shape.VPMatrix, 1, GL_FALSE,
                       glm::value_ptr(frame.ViewProjection));
    throwOnGlError();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)0);
    throwOnGlError();

    glEnableVertexAttribArray(0);
    throwOnGlError();

    const std::vector<GLushort>& indices = shape.indices;
    assert(indices.size() == 36);
#if 0
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT,
                   indices.data());
    throwOnGlError();
  }

} // Soleil
