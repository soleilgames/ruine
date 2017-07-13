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

#ifndef SOLEIL__BOUNDINGBOX_HPP_
#define SOLEIL__BOUNDINGBOX_HPP_

#include <iostream>
#include <limits>

#include <glm/gtc/constants.hpp>
#include <glm/vec3.hpp>

#include "Logger.hpp"
#include "TypesToOStream.hpp"

namespace Soleil {

  class BoundingBox
  {
  public:
    BoundingBox(void)
      : min(glm::vec3(0.0f))
      , max(glm::vec3(0.0f))
      , initialized(false)
    {
    }

    BoundingBox(const glm::vec3& min, const glm::vec3& max)
      : min(min)
      , max(max)
      , initialized(true)
    {
    }

    BoundingBox(const glm::mat4& transformation, const float volume)
      : initialized(false)
    {
      const glm::vec3 point = glm::vec3(transformation[3]);

      expandBy(point);
      expandBy(point - volume);
      expandBy(point + volume);
    }

    virtual ~BoundingBox() {}

    inline bool contains(const glm::vec3& point) const noexcept
    {
      return point.x >= min.x && point.x <= max.x && point.y >= min.y &&
             point.y <= max.y && point.z >= min.z && point.z <= max.z;
    }

    inline bool contains(const glm::vec3& point, const glm::vec3& direction,
                         glm::vec3& intersection) const noexcept
    {
      float tmin = 0.0f;
      float tmax = std::numeric_limits<float>::max();

      for (int i = 0; i < 3; i++) {
        if (std::abs(direction[i]) < glm::epsilon<float>()) {
          if (point[i] < min[i] || point[i] > max[i]) return false;
        } else {
          const float ood = 1.0f / direction[i];
          float       t1  = (min[i] - point[i]) * ood;
          float       t2  = (max[i] - point[i]) * ood;

          if (t1 > t2) std::swap(t1, t2);
          if (t1 > tmin) tmin = t1;
          if (t2 > tmax) tmax = t2;

          if (tmin > tmax) return false;
        }
      }
      intersection = point * direction * tmin;
      return true;
    }

    bool intersect(const BoundingBox& other) const noexcept
    {
      return !(max.x <= other.min.x || other.max.x <= min.x ||
               max.y <= other.min.y || other.max.y <= min.y ||
               max.z <= other.min.z || other.max.z <= min.z);
    }

    inline bool containsFlat(const glm::vec2& point) const noexcept
    {
      return point.x >= min.x && point.x <= max.x && point.y >= min.y &&
             point.y <= max.y;
    }

    void expandBy(const glm::vec3& point) noexcept
    {
      if (initialized == false) {
        min         = point;
        max         = point;
        initialized = true;

        return;
      }

      if (point.x < min.x) min.x = point.x;
      if (point.x > max.x) max.x = point.x;

      if (point.y < min.y) min.y = point.y;
      if (point.y > max.y) max.y = point.y;

      if (point.z < min.z) min.z = point.z;
      if (point.z > max.z) max.z = point.z;
    }

    void expandBy(const BoundingBox& other) noexcept
    {
      this->expandBy(other.min);
      this->expandBy(other.max);
    }

    const glm::vec3& getMin(void) const noexcept { return min; }

    const glm::vec3& getMax(void) const noexcept { return max; }

    void transform(const glm::mat4& transformation)
    {
      glm::vec4 Min(min, 1.0f);
      glm::vec4 Max(max, 1.0f);

      initialized = false;

      expandBy(glm::vec3(transformation * Min));
      expandBy(glm::vec3(transformation * Max));
    }

  protected:
    glm::vec3 min;
    glm::vec3 max;
    bool      initialized;

    friend std::ostream& operator<<(std::ostream& os, const BoundingBox& box)
    {
      os << "BoundingBox: MIN: " << box.min << ", MAX: " << box.max;
      return os;
    }
  };

} // Soleil

#endif /* SOLEIL__BOUNDINGBOX_HPP_ */
