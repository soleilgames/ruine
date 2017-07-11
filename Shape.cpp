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

#include "Shape.hpp"

namespace Soleil {

  Shape::Shape(const std::vector<SubShape>& subShapes)
    : Object(GetType(), GetClassName())
    , subShapes(subShapes)
    , buffer()
  {
    // TODO: In case of GL Context reseted we need to renew the buffer
    gl::BindBuffer bindBuffer(GL_ARRAY_BUFFER, *buffer);
    glBindBuffer(GL_ARRAY_BUFFER, *buffer);

    GLsizeiptr size = 0;
    for (const auto& sub : subShapes) {
      size += sizeof(sub.vertices[0]) * sub.vertices.size();
    }

    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STREAM_DRAW);

    GLintptr offset = 0;
    for (const auto& sub : subShapes) {
      GLsizeiptr componentSize = sizeof(sub.vertices[0]) * sub.vertices.size();

      glBufferSubData(GL_ARRAY_BUFFER, offset, componentSize,
                      sub.vertices.data());
      offset += componentSize;
    }

    throwOnGlError();
  }

  Shape::~Shape() {}

  GLuint Shape::getBuffer() const noexcept { return *buffer; }

  const std::vector<SubShape>& Shape::getSubShapes(void) const noexcept
  {
    return this->subShapes;
  }

  BoundingBox Shape::makeBoundingBox(void) const noexcept
  {
    BoundingBox box;

    for (const auto& sub : subShapes) {
      for (const auto& vertex : sub.vertices) {
        box.expandBy(glm::vec3(vertex.position));
      }
    }
    return box;
  }

} // Soleil
