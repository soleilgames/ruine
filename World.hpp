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

#include <functional>
#include <vector>

#include "BoundingBox.hpp"
#include "Draw.hpp"
#include "types.hpp"

namespace Soleil {

  struct GhostData
  {
    glm::mat4*  transformation;
    std::size_t lightPosition;
    std::size_t deathBoxPosition;
    glm::vec3   direction;
    BoundingBox bounds;
    BoundingBox stressBounds;

    GhostData(glm::mat4* transformation, std::size_t lightPosition,
              std::size_t deathBoxPosition, const glm::vec3& direction);
  };

  struct Door
  {
    std::string  id;
    std::string  level;
    glm::vec2    aoe;
    std::string  output;
    glm::vec2    start;
    std::wstring name;
    std::size_t  uid;
    BoundingBox  triggerZone;
  };

  enum ShapeType
  {
    WallCube,
    Barrel,
    Floor,
    GateHeavy,
    Key,
    Coin,
    Ghost,
    GhostFriend,
    ST_BoundingBox,

    Size
  };
  // Position in shapes vector

  enum TriggerState
  {
    NeverTriggered   = 0,
    JustTriggered    = 1,
    CurrentlyActive  = 2,
    AlreadyTriggered = 4
  };

  enum class TriggerType
  {
    Coin,
    Door,
    Key
  };

  struct Trigger
  {
    BoundingBox aoe;
    int         state;
    TriggerType type;

    std::size_t link;
  };

  struct World
  {
    std::vector<Door> doors;

    BoundingBox              bounds;
    RenderInstances          statics;
    RenderInstances          objects;
    std::vector<GhostData>   sentinels;
    std::vector<GhostData>   hunters;
    std::vector<BoundingBox> hardSurfaces;
    glm::mat4                theKey;

    // Player's progression
    std::vector<std::size_t> coinPickedUp;
    bool                     keyPickedUp = false; // TODO: Put it in Ruine.hpp
    std::string              lastDoor;

    std::vector<ShapePtr> shapes;
    // All the Models
    std::vector<DrawElement> elements;
    // TODO: Should be named statics. Will have a fixed size
    std::vector<DrawElement> items;
    // Coins, keys, ... size will varry
    std::vector<Trigger> triggers;
    // Everything the player can walk on
    std::vector<BoundingBox> deathTriggers;
    // Change the player into ghost - game over!
    std::vector<BoundingBox> frighteningTriggers;
    // Change the player into ghost - game over!
    std::vector<DrawElement> ghosts;
    // All monsters

    World() {}
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    void   resetLevel(void);
  };

  void loadMap(World& world, Frame& frame, std::istringstream& s);
  void InitializeWorldModels(World& world);
  void InitializeWorldDoors(World& world, const std::string& assetName);
  void InitializeLevel(World& world, const std::string& level, Frame& frame,
                       Camera& camera, PopUp& caption);
  std::string DoorUIDToId(const std::vector<Door>& doors,
                          const std::size_t        uid);
  Door* GetDoorByUID(std::vector<Door>& doors, const std::size_t uid);

} // Soleil

#endif /* SOLEIL__WORLD_HPP_ */
