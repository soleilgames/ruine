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

#ifndef SOLEIL__DRAW_HPP_
#define SOLEIL__DRAW_HPP_

#include <functional>

#include "OpenGLInclude.hpp"
#include "Shape.hpp"
#include "types.hpp"

#ifndef NDEBUG
#include "BoundingBox.hpp"
#endif

#include <glm/mat4x4.hpp>

namespace Soleil {

  // Render element on screen
  struct DrawElement
  {
    size_t      shapeIndex;     // Index in the shape vector
    glm::mat4   transformation; // Current transfromation
    std::size_t id;             // An id that should be unique for picking

    DrawElement(size_t shapeIndex, const glm::mat4& transformation)
      : shapeIndex(shapeIndex)
      , transformation(transformation)
    {
      id = Hash(*this);
    }

    DrawElement()
      : id(0)
    {
    }

    // Generate an unique id for each node    
    static std::size_t Hash(const DrawElement& e)
    {
      size_t hash = std::hash<size_t>()(e.shapeIndex);

      const auto fx = std::hash<float>();
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          hash ^= fx(e.transformation[i][j]);
        }
      }
      return hash;
    }
  };

  struct DrawCommand
  {
    GLuint                       buffer;
    glm::mat4                    transformation;
    std::vector<SubShape> const* sub;

    DrawCommand(GLuint buffer, const glm::mat4& transformation,
                const std::vector<SubShape>& sub)
      : buffer(buffer)
      , transformation(transformation)
      , sub(&sub)
    {
    }

    DrawCommand(GLuint buffer, const glm::mat4& transformation,
                const std::vector<SubShape>* sub)
      : buffer(buffer)
      , transformation(transformation)
      , sub(sub)
    {
    }

    DrawCommand(const Shape& shape, const glm::mat4& transformation)
      : buffer(shape.getBuffer())
      , transformation(transformation)
      , sub(&shape.getSubShapes())
    {
    }

    bool operator==(const DrawCommand& other) const noexcept
    {
      return buffer == other.buffer && transformation == other.transformation &&
             sub == other.sub;
    }
  };

  struct TextCommand
  {
    gl::Buffer            buffer;
    std::vector<GLushort> elements;
  };

  class PopUp
  {
  public:
    PopUp();

  public:
    bool isActive(void) const noexcept { return active; }

  public:
    void fillText(const std::wstring& label, const float em);
    void activate(const Timer& timeToFadeOut, const Timer& startTime);
    void render(Timer time);

  public:
    glm::mat4 transformation;

  private:
    TextCommand label;
    long        fadeOut;
    bool        active;

  private:
    Timer startTime;
  };

  struct CharVertex
  {
    glm::vec3 position;
    glm::vec2 uv;
  };

  typedef std::vector<DrawCommand> RenderInstances;

  void DrawImage(GLuint texture, const glm::mat4& transformation,
                 const glm::vec4& color = glm::vec4(1.0f));
  void RenderPhongShape(const RenderInstances& instances, const Frame& frame);
  void RenderFlatShape(const RenderInstances& instances, const Frame& frame);
  void RenderFlatShape(const glm::mat4& transformation, const Shape& shape,
                       const Frame& frame);
  void DrawText(const TextCommand& textCommand, const glm::mat4& transformation,
                const glm::vec4& color);
  void Fade(const float ratio);
#ifndef NDEBUG
  void DrawBoundingBox(const BoundingBox& box, const Frame& frame,
                       const glm::vec4& color = glm::vec4(1.0f, 1.0f, 1.0f,
                                                          0.5f));
#endif
} // Soleil

#endif /* SOLEIL__DRAW_HPP_ */
