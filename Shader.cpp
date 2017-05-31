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

#include "Shader.hpp"
#include "Logger.hpp"
#include "AssetService.hpp"

#include <vector>

namespace Soleil {

  Shader::Shader(GLenum shaderType, const std::string& fileName)
    : shader(glCreateShader(shaderType))
    , name(fileName)
  {
    const std::string source = AssetService::LoadAsString(fileName);
    const GLchar* sources[] = {static_cast<const GLchar*>(source.c_str())};

    glShaderSource(shader, 1, sources, nullptr);
  }

  Shader::~Shader() { glDeleteShader(shader); }

  GLuint Shader::operator*()
  {
    glCompileShader(shader);

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

    return shader;
  }

} // Soleil
