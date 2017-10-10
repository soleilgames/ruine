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
#include <climits>
#include <vector>

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
  typedef glm::vec3 Position;
  typedef glm::vec4 Color;
  typedef glm::vec3 RGB;
  typedef glm::vec4 RGBA;
  typedef glm::vec3 Normal;
  typedef glm::vec2 Coord2D;

  struct PointLight
  {
    glm::vec3 position;
    glm::vec3 color;
    float     linear;
    float     quadratic;
  };

  /**
   * Provide information on the current frame
   */
  struct Frame
  {
    /**
     * Time since beginning of the application.
     */
    Timer time;

    /**
     * Time elapsed since last frame.
     *
     * At 60fps it should always be 16ms.
     */
    Timer delta;

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
     * the Projection without the view
     * @see updateViewProjectionMatrices(const glm::mat4& view, const glm::mat4
     * projection)
     */
    glm::mat4 Projection;

    /**
     * Update related matrices and set the MVP one.
     */
    inline void updateViewProjectionMatrices(const glm::mat4& view,
                                             const glm::mat4  projection)
    {
      this->View           = view;
      this->Projection     = projection;
      this->ViewProjection = projection * view;
    }

    // TODO: Should be in world structure
    std::vector<PointLight> pointLights;
  };

  struct Camera
  {
    glm::vec3 position;
    float     yaw;
  };

  /**
   * Convert a float to an int with the specified precision. Do not check for
   * overflow
   */
  template <int decimals> constexpr int trail_point(const double f)
  {
    constexpr int decimalTable[] = {1,         10,        100,     1000,
                                    10000,     100000,    1000000, 10000000,
                                    100000000, 1000000000};
    const double r = f * decimalTable[decimals];
    return static_cast<int>(r);
  }

  /**
   * Combine current hash with new value
   */
  template <class T> void hash_combine(std::size_t& h, const T& v)
  {
    std::hash<T> hasher;
    h ^= hasher(v) + 0x9e3779b9 + (h << 6) + (h >> 2);
  }

#if 0
    // Not used yet
    constexpr bool
    is_big_endian(void)
  {
    union
    {
      uint32_t i;
      char     c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
  }

  template <typename T> T constexpr swap_endian(T u)
  {
    static_assert(CHAR_BIT == 8, "CHAR_BIT != 8");

    union
    {
      T             u;
      unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
      dest.u8[k]  = source.u8[sizeof(T) - k - 1];

    return dest.u;
  }
#endif

  namespace gval {
    // static const glm::vec3 ambiantLight(0.01f);
    static const glm::vec3 ambiantLight(1.0f);
    static const glm::vec3 cameraLight(0.35f);
    static const glm::vec3 ghostColor(0.20f, 0.20f, 0.30f);
    static const float     ghostSpeed     = 0.005f;
    static const float     cameraYawSpeed = 0.002f;
    static const float     cameraSpeed    = 0.05f;
    static const char*     firstLevel     = "0";
    static const Timer     timeToFadeText(3500);
    static const float     textLabelSize = 0.35f;
    static const Color     textLabelColor(0.8f);
    static const Timer     timeBeforeWhisper(6000);

#if 0 // Temp
    static GLuint bezierTex;
#endif

  } // gval

} // Soleil

#endif /* SOLEIL__TYPES_HPP_ */
