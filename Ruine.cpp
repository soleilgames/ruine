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

#include <cmath>
#include <iostream>
#include <vector>

namespace Soleil {

  Ruine::Ruine()
    : triangle(0)
  {
  }

  Ruine::~Ruine()
  {
    if (triangle > 0) {
      glDeleteProgram(triangle);
    }
  }

  static void compileShader(GLuint shader, const std::string name)
  {
    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    std::string state = (isCompiled == GL_FALSE) ? "FAILED" : "SUCCESS";

    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

    std::string logs;
    if (maxLength > 1) {
      logs = std::string(errorLog.begin(), errorLog.end());
    }

    std::string fin = toString("[COMPILATION] Shader (", name, ") compilation ",
                               state, ": ", logs);

    if (isCompiled == GL_FALSE) {
      throw std::runtime_error(fin);
    } else {
      std::cout << fin << "\n";
    }
  }

  void Ruine::render(Timer time)
  {
    static const float vertexIDs[] = {0.0f, 1.0f, 2.0f};

    /* Minimal gles2 test */
    if (triangle < 1) {
      const GLchar* vertexShaderSource[] = {
        "#version 100\n"
        "\n"
        "uniform float time;\n"
        "attribute float vertexId;\n"
        "\n"
        "varying vec4 color;\n"
        "\n"
        "void main()\n"
        "{\n"
        "  vec4 vertices[3];\n"
        "  vertices[0] = vec4(-1.0, -1.0, 0.0, 1.0);\n"
        "  vertices[1] = vec4( 1.0, -1.0, 0.0, 1.0);\n"
        "  vertices[2] = vec4( 0.0,  1.0, 0.0, 1.0);\n"
        "\n"
        "  vec4 colors[3];\n"
        "  colors[0] = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "  colors[1] = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "  colors[2] = vec4(0.0, 0.0, 1.0, 1.0);\n"
        "\n"
        "int i = int(vertexId);\n"
        "  gl_Position = vertices[i] * vec4(time, time, time, 1.0f);\n"
        "  color = colors[i] * vec4(time, time, time, 1.0f);\n"
        "}\n"};
      const GLchar* fragmentShaderSource[] = {"#version 100\n"
                                              "\n"
                                              "precision lowp float;\n"
                                              "\n"
                                              "varying vec4 color;\n"
                                              "\n"
                                              "void main()\n"
                                              "{\n"
                                              "  gl_FragColor = color;\n"
                                              "}\n"};

      GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertexShader, 1, vertexShaderSource, nullptr);
      glCompileShader(vertexShader);
      throwOnGlError();
      compileShader(vertexShader, "vertexShader");

      GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragmentShader, 1, fragmentShaderSource, nullptr);
      glCompileShader(fragmentShader);
      throwOnGlError();
      compileShader(fragmentShader, "fragmentShader");

      triangle = glCreateProgram();
      glBindAttribLocation(triangle, 0, "vertexId");
      glAttachShader(triangle, vertexShader);
      glAttachShader(triangle, fragmentShader);
      glLinkProgram(triangle);
      throwOnGlError();

      glDeleteShader(vertexShader);
      glDeleteShader(fragmentShader);

      glGenBuffers(1, &buffer);
      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertexIDs), &vertexIDs[0],
                   GL_STATIC_DRAW);
      throwOnGlError();
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glClearColor(0.0f, 0.3f, 0.7f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    throwOnGlError();
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(vertexIDs[0]),
                          nullptr);
    glEnableVertexAttribArray(0);
    glUseProgram(triangle);
    GLint uniformTime = glGetUniformLocation(triangle, "time");
    if (uniformTime < 0) throw std::runtime_error("Cannot find uniform time");

    static float length    = 0.0f;
    static float direction = 0.01f;
    length += direction;
    if (length > 1.0f) {
      length    = 1.0f;
      direction = -0.01f;
    }
    if (length < 0.0f) {
      length    = 0.0f;
      direction = 0.01f;
    }
    glUniform1f(uniformTime, length);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    throwOnGlError();
  }
} // Soleil
