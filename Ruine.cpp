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
#include "Logger.hpp"

#include <cmath>
#include <iostream>
#include <vector>

namespace Soleil {

  Ruine::Ruine(AssetService* assetService, SoundService* soundService)
    : triangle(0)
    , buffer(0)
    , assetService(assetService)
    , soundService(soundService)
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
      Logger::info(fin);
    }
  }

  void Ruine::render(Timer /*time*/)
  {
    // clang-format off
    //                               xxxx, yyyy, zzz, www, rrr, ggg, bbb, aaa
    static const float vertices[] = {-1.0, -1.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0,
				      1.0, -1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0,
				      0.0,  1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0
    };
    // clang-format on

    /* Minimal gles2 test */
    if (triangle < 1) {
      const std::string src = assetService->asString("triangle.vert");
      const GLchar*     vertexShaderSource[]   = {src.c_str()};
      const GLchar*     fragmentShaderSource[] = {"#version 100\n"
                                              "\n"
                                              "precision mediump float;\n"
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
      glBindAttribLocation(triangle, 0, "positionAttribute");
      glBindAttribLocation(triangle, 1, "colorAttribute");
      glAttachShader(triangle, vertexShader);
      glAttachShader(triangle, fragmentShader);
      glLinkProgram(triangle);
      throwOnGlError();

      glDeleteShader(vertexShader);
      glDeleteShader(fragmentShader);

      glGenBuffers(1, &buffer);
      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0],
                   GL_STATIC_DRAW);
      throwOnGlError();
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      soundService->playMusic("fabulous.wav");

      SOLEIL__LOGGER_DEBUG("Initialization done");
    }

    glClearColor(0.0f, 0.3f, 0.7f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    throwOnGlError();
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8,
                          nullptr);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8,
                          (GLvoid*)(4 * sizeof(vertices[0])));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glUseProgram(triangle);
    GLint uniformTime = glGetUniformLocation(triangle, "time");
    if (uniformTime < 0) throw std::runtime_error("Cannot find uniform time");

    static float length    = 0.0f;
    static float direction = 0.01f;
    length += direction;
    if (length > 1.0f) {
      length    = 1.0f;
      direction = -0.01f;

      soundService->fireSound("woot.pcm");
    }
    if (length < 0.0f) {
      length    = 0.0f;
      direction = 0.01f;

      soundService->fireSound("woot.pcm");
    }
    glUniform1f(uniformTime, length);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    throwOnGlError();
  }
} // Soleil
