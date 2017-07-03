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

#ifndef SOLEIL__WORLD_HPP_
#define SOLEIL__WORLD_HPP_

#include <vector>

#include "BoundingBox.hpp"
#include "Draw.hpp"
#include "types.hpp"

namespace Soleil {

  struct GhostData
  {
    glm::mat4*  transformation;
    size_t      lightPosition;
    glm::vec3   direction;
    BoundingBox bounds;

    GhostData(glm::mat4* transformation, size_t lightPosition,
              const glm::vec3& direction);

    void updateBounds(void) noexcept;
  };

  struct NextZoneTrigger
  {
    BoundingBox aoe;
    std::string nextZone;
  };

  struct Door
  {
    const std::string id;
    const std::string level;
    const glm::vec2   aoe;
    const std::string output;
    const glm::vec2   start;
  };

  struct World
  {
    // TODO:  std::vector<ShapePtr>  models;
    ShapePtr wallShape;
    ShapePtr floorShape;
    ShapePtr torchShape;
    ShapePtr ghostShape;
    ShapePtr gateShape;

    std::vector<Door> doors;

    glm::vec3                    bounds;
    RenderInstances              statics;
    std::vector<GhostData>       sentinels;
    std::vector<BoundingBox>     hardSurfaces;
    std::vector<NextZoneTrigger> nextZoneTriggers;

    World() {}
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    void   resetLevel(void);
  };

  void InitializeWorldModels(World& world);
  void InitializeLevel(World& world, const std::string& level, Frame& frame,
                       Camera& camera);
  void InitializeLevelFromAsset(World& world, const std::string& asset,
                                Frame& frame, Camera& camera);

} // Soleil

#endif /* SOLEIL__WORLD_HPP_ */
