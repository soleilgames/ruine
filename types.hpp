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

#ifndef SOLEIL__TYPES_HPP_
#define SOLEIL__TYPES_HPP_

#include <chrono>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Soleil {

  /**
   * A timer in milliseconds
   *
   * Represent the default distance in time measurement
   */
  typedef std::chrono::milliseconds Timer;

  typedef glm::vec4 Point;
  typedef glm::vec4 Color;
  typedef glm::vec3 Normal;
  typedef glm::vec2 Coord2D;

  /**
   * Provide information on the current frame
   */
  struct Frame
  {
    /**
     * Time since beginning of the application.
     */
    Timer time;

    glm::vec2 viewportSize;

    /**
     * Camera position for rendering lights
     */
    glm::vec3 cameraPosition;
    
    /**
     * An up to date View * Projection Matrix
     * @see updateViewProjectionMatrices(const glm::mat4& view, const glm::mat4
     * projection)
     */
    glm::mat4 ViewProjection;

    /**
     * the View without the projection
     * @see updateViewProjectionMatrices(const glm::mat4& view, const glm::mat4
     * projection)
     */
    glm::mat4 View;

    /**
     * Update related matrices and set the MVP one.
     */
    inline void updateViewProjectionMatrices(const glm::mat4& view,
                                             const glm::mat4  projection)
    {
      this->View           = view;
      this->ViewProjection = projection * view;
    }
  };

} // Soleil

#endif /* SOLEIL__TYPES_HPP_ */
