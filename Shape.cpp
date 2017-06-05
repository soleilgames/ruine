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

  Shape::Shape(const std::vector<Vertex>& vertices)
    : Object(GetType(), GetClassName())
    , vertices(vertices)
    , buffer()
  {
    gl::BindBuffer bindBuffer(GL_ARRAY_BUFFER, *buffer);
    glBindBuffer(GL_ARRAY_BUFFER, *buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(),
                 vertices.data(), GL_STATIC_DRAW);

    throwOnGlError();
  }

  Shape::Shape(const std::vector<Vertex>&   vertices,
               const std::vector<GLushort>& indices)
    : Object(GetType(), GetClassName())
    , vertices(vertices)
    , indices(indices)
    , buffer()
  {
    gl::BindBuffer bindBuffer(GL_ARRAY_BUFFER, *buffer);
    glBindBuffer(GL_ARRAY_BUFFER, *buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(),
                 vertices.data(), GL_STATIC_DRAW);

    throwOnGlError();
  }

  Shape::~Shape() {}

  const std::vector<Vertex>& Shape::getVertices(void) const noexcept
  {
    return vertices;
  }

  GLuint Shape::getBuffer() noexcept { return *buffer; }

  const std::vector<GLushort>& Shape::getIndices(void) const noexcept
  {
#ifdef SOLEIL__DRAWARRAYS
    throw std::runtime_error("No indices when built with SOLEIL__DRAWARRAYS");
#endif
    return indices;
  }

} // Soleil
