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

namespace Soleil {

  Drawable::Drawable(ShapePtr shape)
    : Node(typeid(Drawable).hash_code(), "Drawable")
    , shape(shape)
    , triangle()
  {

    triangle.attachShader(Shader(GL_VERTEX_SHADER, "triangle.vert"));
    triangle.attachShader(Shader(GL_FRAGMENT_SHADER, "triangle.frag"));

    glBindAttribLocation(triangle.program, 0, "positionAttribute");
    glBindAttribLocation(triangle.program, 1, "colorAttribute");

    triangle.compile();
    
    SoundService::PlayMusic("fabulous.wav");
  }

  Drawable::~Drawable() {}

  void Drawable::render()
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
    glUseProgram(triangle.program);
    GLint uniformTime = triangle.getUniform("time");

    static float length    = 0.0f;
    static float direction = 0.01f;
    length += direction;
    if (length > 1.0f) {
      length    = 1.0f;
      direction = -0.01f;

      SoundService::FireSound("woot.pcm", SoundProperties(100));
    }
    if (length < 0.0f) {
      length    = 0.0f;
      direction = 0.01f;

      SoundService::FireSound("woot.pcm", SoundProperties(50));
    }
    glUniform1f(uniformTime, length);

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    throwOnGlError();
  }

} // Soleil
