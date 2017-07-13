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

#ifndef SOLEIL__TYPESTOOSTREAM_HPP_
#define SOLEIL__TYPESTOOSTREAM_HPP_

#include <chrono>
#include <iostream>

#include <utility>

#define GLM_ENABLE_EXPERIMENTAL // Required to compile with string_cast
#include <glm/gtx/string_cast.hpp>
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace glm {

  std::ostream& operator<<(std::ostream& os, const glm::mat4& mat);
  std::ostream& operator<<(std::ostream& os, const glm::vec4& vec);
  std::ostream& operator<<(std::ostream& os, const glm::vec3& vec);
  std::ostream& operator<<(std::ostream& os, const glm::vec2& vec);

  std::wostream& operator<<(std::wostream& os, const glm::vec2& vec);

} // glm

namespace std {
  namespace chrono {
    std::ostream& operator<<(std::ostream&                    os,
                             const std::chrono::milliseconds& ms);
    std::wostream& operator<<(std::wostream&                   os,
                             const std::chrono::milliseconds& ms);
  };
};

#endif /* SOLEIL__TYPESTOOSTREAM_HPP_ */
