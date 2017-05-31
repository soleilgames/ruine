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

#ifndef SOLEIL__SHAPE_HPP_
#define SOLEIL__SHAPE_HPP_

#include "Object.hpp"
#include "OpenGLInclude.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>
#include <memory>


namespace Soleil {

  struct Vertex
  {
    glm::vec4 position;
    glm::vec3 normal;
    glm::vec4 color;
    glm::vec2 uv;

    Vertex(glm::vec4 position, glm::vec3 normal,
           glm::vec4 color = glm::vec4(1.0f), glm::vec2 uv = glm::vec2(-1.0f))
      : position(position)
      , normal(normal)
      , color(color)
      , uv(uv)
    {
    }
  };

  /**
   * Shape holds informations on a 3D Model object.
   *
   * Each instances may refers to this class to draw the model
   */
  class Shape : public Object
  {
  public:
    Shape(const std::vector<Vertex>& vertices);
    virtual ~Shape();

  public:
    const std::vector<Vertex>& getVertices(void) const noexcept;
    GLuint getBuffer() noexcept;

  private:
    std::vector<Vertex> vertices;
    gl::Buffer          buffer;

  public:
    static HashType GetType(void) noexcept { return typeid(Shape).hash_code(); }
    static const std::string& GetClassName(void) noexcept
    {
      static std::string name = "Shape";
      return name;
    }
  };

  typedef std::shared_ptr<Shape> ShapePtr;

} // Soleil

#endif /* SOLEIL__SHAPE_HPP_ */
