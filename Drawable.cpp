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
#include "Shader.hpp"
#include "SoundService.hpp"

#include <glm/gtc/type_ptr.hpp>

namespace Soleil {

  Drawable::Drawable(ShapePtr shape)
    : Node(typeid(Drawable).hash_code(), "Drawable")
    , shape(shape)
    , rendering()
  {

    rendering.attachShader(Shader(GL_VERTEX_SHADER, "shape.vert"));
    rendering.attachShader(Shader(GL_FRAGMENT_SHADER, "shape.frag"));

    glBindAttribLocation(rendering.program, 0, "positionAttribute");
    glBindAttribLocation(rendering.program, 1, "colorAttribute");

    rendering.compile();

    SoundService::PlayMusic("fabulous.wav");
  }

  Drawable::~Drawable() {}

  void Drawable::render(const Frame& frame)
  {
    const std::vector<Vertex>& vertices = shape->getVertices();
    constexpr GLsizei          stride   = sizeof(Vertex);

    gl::BindBuffer bindBuffer(GL_ARRAY_BUFFER, shape->getBuffer());
    glBindBuffer(GL_ARRAY_BUFFER, shape->getBuffer());
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride,
                          (const GLvoid*)offsetof(Vertex, color));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glUseProgram(rendering.program);

    GLint uniformModel = rendering.getUniform(
      "Model"); // TODO: Save uniform until OpenGL Context is reseted

    auto ViewProjectionModel = frame.ViewProjection * getTransformation();
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE,
                       glm::value_ptr(ViewProjectionModel));

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    throwOnGlError();
  }

} // Soleil
