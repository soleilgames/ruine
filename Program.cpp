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

#include "Program.hpp"
#include "Logger.hpp"

namespace Soleil {

  Program::Program()
    : program(glCreateProgram())
  {
  }

  Program::~Program() { glDeleteProgram(program); }

  void Program::attachShader(const Shader& shader)
  {
    glAttachShader(program, *shader);
  }

  void Program::compile(void)
  {
    glLinkProgram(program);

    GLint infoLen    = 0;
    GLint isCompiled = 0;

    glGetProgramiv(program, GL_LINK_STATUS, &isCompiled);
    std::string state = (isCompiled == GL_FALSE) ? "FAILED" : "SUCCESS";

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);

    std::string logs;
    if (infoLen > 1) {
      std::vector<GLchar> errorLog(infoLen);
      glGetProgramInfoLog(program, infoLen, nullptr, &errorLog[0]);
      logs = ":" + std::string(errorLog.begin(), errorLog.end());
    }

    std::string fin =
      toString("[COMPILATION] Program compilation ", state, logs);

    if (isCompiled == GL_FALSE) {
      glDeleteProgram(program); // Destructor whill never be reached
      throw std::runtime_error(fin);
    } else {
      Logger::info(fin);
    }

    throwOnGlError();
  }

  GLint Program::getUniform(const GLchar* name)
  {
    GLint location = glGetUniformLocation(program, name);
    if (location < 0)
      throw std::runtime_error(toString("Cannot find location: ", name));
    return location;
  }

} // Soleil
