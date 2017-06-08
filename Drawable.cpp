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

#include "Drawable.hpp"
#include "AssetService.hpp"
#include "Logger.hpp"
#include "OpenGLDataInstance.hpp"
#include "Shader.hpp"
#include "TypesToOStream.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Soleil {

  Drawable::Drawable(ShapePtr shape)
    : Node(typeid(Drawable).hash_code(), "Drawable")
    , shape(shape)
  {
  }

  Drawable::~Drawable() {}

  void Drawable::render(const Frame& frame)
  {
    constexpr GLsizei         stride    = sizeof(Vertex);
    const OpenGLDataInstance& instance  = OpenGLDataInstance::Instance();
    const Program&            rendering = instance.drawable;
    const Material&           material  = shape->getMaterial();

    gl::BindBuffer bindBuffer(GL_ARRAY_BUFFER, shape->getBuffer());
    glBindBuffer(GL_ARRAY_BUFFER, shape->getBuffer());
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          (const GLvoid*)offsetof(Vertex, normal));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride,
                          (const GLvoid*)offsetof(Vertex, color));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glUseProgram(rendering.program);

    auto ViewProjectionModel = frame.ViewProjection * getTransformation();
    glUniformMatrix4fv(instance.drawableMVPMatrix, 1, GL_FALSE,
                       glm::value_ptr(ViewProjectionModel));

    auto ModelView = frame.View * getTransformation();
    glUniformMatrix4fv(instance.drawableMVMatrix, 1, GL_FALSE,
                       glm::value_ptr(getTransformation())); // ModelView

    throwOnGlError();

/* TODO: If only rotation and isometric (nonshape changing) scaling was
 * performed, the Mat3 is should be fine: */
#if 0
    auto NormalMatrix = glm::mat3(ModelView);
#endif

    auto NormalMatrix = glm::transpose(glm::inverse(glm::mat3(getTransformation())));

    glUniformMatrix3fv(instance.drawableNormalMatrix, 1, GL_FALSE,
                       glm::value_ptr(NormalMatrix));
    throwOnGlError();

    glUniform3fv(instance.drawableMaterialAmbiant, 1,
                 glm::value_ptr(material.ambiantColor));
    glUniform1f(instance.drawableMaterialShininess, material.shininess);

    glUniform3fv(instance.drawableAmbiantLight, 1,
                 glm::value_ptr(glm::vec3(0.f, 0.f, 0.f)));

    glUniform3fv(instance.drawableEyeDirection, 1,
                 glm::value_ptr(frame.cameraPosition));
    throwOnGlError();

#if 1
    static float angle = 0.1f;

    glm::vec4 lpos =
      glm::rotate(glm::mat4(), angle, glm::vec3(0.0f, 1.0f, 0.0f)) *
      glm::vec4(2.0f, 0.55f, 0.0f, 1.0f);

    angle += 0.0050f;

    glUniform3fv(instance.drawablePointLightPosition, 1, glm::value_ptr(lpos));
#else
    glUniform3fv(instance.drawablePointLightPosition, 1,
                 glm::value_ptr(frame.cameraPosition));

#endif

    glUniform3fv(instance.drawablePointLightColor, 1,
                 glm::value_ptr(glm::vec3(0.8f)));
    glUniform1f(instance.drawablePointLightLinearAttenuation, 0.014f);
    glUniform1f(instance.drawablePointLightQuadraticAttenuation, 0.0007);

    throwOnGlError();

#ifdef SOLEIL__DRAWARRAYS
    const std::vector<Vertex>& vertices = shape->getVertices();
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
#else
    const std::vector<GLushort>& indices = shape->getIndices();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT,
                   indices.data());
#endif

    throwOnGlError();
  }

} // Soleil
