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

#include "World.hpp"

#include "AssetService.hpp"
#include "WavefrontLoader.hpp"
#include "stringutils.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <functional>

namespace Soleil {

  void InitializeWorldModels(World& world)
  {
    auto ghost = Soleil::WavefrontLoader::fromContent(
      AssetService::LoadAsString("ghost.obj"));
    world.shapes = {
      Soleil::WavefrontLoader::fromContent(
        AssetService::LoadAsString("wallcube.obj")),
      Soleil::WavefrontLoader::fromContent(
        AssetService::LoadAsString("barrel.obj")),
      Soleil::WavefrontLoader::fromContent(
        AssetService::LoadAsString("floor.obj")),
      Soleil::WavefrontLoader::fromContent(
        AssetService::LoadAsString("gate.obj")),
      Soleil::WavefrontLoader::fromContent(
        AssetService::LoadAsString("key.obj")),
      Soleil::WavefrontLoader::fromContent(
        AssetService::LoadAsString("coin.obj")),
      ghost, // The bad One
      ghost  // The Good one
    };
  }

  void InitializeWorldDoors(World& world, const std::string& assetName)
  {
    std::istringstream file(AssetService::LoadAsString(assetName));
    world.doors.clear();

    std::string line;
    while (std::getline(file, line)) {
      if (line[0] == '#') continue;

      std::istringstream sline(line);
      glm::vec3          min, max;

      Door d;

      std::string id;
      sline >> d.id; // TODO: to Check

      sline >> d.level;
      sline >> min.x;
      sline >> min.y;
      sline >> min.z;
      sline >> max.x;
      sline >> max.y;
      sline >> max.z;
      // sline >> d.aoe.x;
      // sline >> d.aoe.y;
      sline >> d.output;
      // sline >> d.start.x;
      // sline >> d.start.y;
      d.triggerZone = BoundingBox(min, max);

      d.uid = std::hash<std::string>{}(d.id);

      std::string name;
      sline.get(); // Consume \t
      std::getline(sline, name);
      d.name = StringToWstring(name);
      world.doors.push_back(d);

      SOLEIL__LOGGER_DEBUG(toString("d.id:", d.id));
      SOLEIL__LOGGER_DEBUG(toString("d.level:", d.level));
      // SOLEIL__LOGGER_DEBUG(toString("d.aoe", d.aoe));
      SOLEIL__LOGGER_DEBUG(toString("d.triggerZone:", d.triggerZone));
      SOLEIL__LOGGER_DEBUG(toString("d.output:", d.output));
      // SOLEIL__LOGGER_DEBUG(toString("d.start", d.start));
    }
  }

  void World::resetLevel()
  {
    bounds = BoundingBox();
    objects.clear();
    statics.clear();
    sentinels.clear();
    hardSurfaces.clear();

    ghosts.clear();
    items.clear();
    elements.clear();
    triggers.clear();
  }

  void pushCoin(DrawElement& draw, World& world)
  {
    auto pos = std::find(std::begin(world.coinPickedUp),
                         std::end(world.coinPickedUp), draw.id);
    if (pos != std::end(world.coinPickedUp)) {
      return;
    }

    world.items.push_back(draw);
    world.triggers.push_back(
      {BoundingBox(draw.transformation, 0.25f), // TODO: use gval
       TriggerState::NeverTriggered, TriggerType::Coin, draw.id});
  }

  void setKey(DrawElement& draw, World& world)
  {
    BoundingBox box = world.shapes[ShapeType::Key]->makeBoundingBox();

    box.transform(draw.transformation);
    world.items.push_back(draw);
    world.triggers.push_back(
      {box, TriggerState::NeverTriggered, TriggerType::Key, draw.id});
  }

  void pushGhost(DrawElement& draw, World& world, Frame& frame)
  {
    world.ghosts.push_back(draw);

    BoundingBox box = world.shapes[ShapeType::Ghost]->makeBoundingBox();
    box.transform(draw.transformation);
    world.triggers.push_back(
      {box, TriggerState::NeverTriggered, TriggerType::Ghost, 0});

    PointLight p;
    p.color     = gval::ghostColor;
    p.position  = glm::vec3(draw.transformation[3]);
    p.linear    = 0.09f;
    p.quadratic = 0.032f;
    frame.pointLights.push_back(p);

    world.sentinels.push_back(GhostData(&(world.ghosts.back().transformation),
                                        frame.pointLights.size() - 1,
                                        glm::vec3(0, 0, 1)));
  }

  /**
   * Count the number of line till the next section
   */
  static int CountLine(std::istringstream& s)
  {
    auto pos = s.tellg();

    int         count = 0;
    std::string line;
    while (std::getline(s, line)) {
      if (line[0] == '#') continue; // Skip comments
      if (line.size() < 1) {
        break;
      }

      count++;
    }

    s.seekg(pos);

    return count;
  }

  enum Step
  {
    Statics,
    Coins,
    Ghosts,
  };

  static void ReserveArrays(World& world, std::istringstream& s, Step step)
  {
    switch (step) {
      case Statics: world.elements.reserve(CountLine(s)); break;
      case Ghosts: {
        // +1 for player ghost
        auto count = CountLine(s) + 1;
        world.ghosts.reserve(count);
        world.sentinels.reserve(count);
      } break;
      case Coins: world.items.reserve(CountLine(s)); break;
    }
  }

  void loadMap(World& world, Frame& frame, std::istringstream& s)
  {
    std::string line;

    world.elements.clear();
    world.items.clear();
    world.ghosts.clear();
    world.triggers.clear();
    int step = Step::Statics;
    while (std::getline(s, line)) {
      if (line[0] == '#') continue; // Skip comments
      if (line.size() < 1) {
        step++;

        ReserveArrays(world, s, static_cast<Step>(step));
        continue;
      }

      std::istringstream drawStr(line);
      DrawElement        draw;

      drawStr >> draw.shapeIndex;

      glm::mat4& t = draw.transformation;
      for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
          drawStr >> t[x][y];
        }
      }
      draw.id = DrawElement::Hash(draw);

      switch (step) {
        case Step::Statics: {
          world.elements.push_back(draw);

          BoundingBox box = world.shapes[draw.shapeIndex]->makeBoundingBox();
          box.transform(draw.transformation);

          world.bounds.expandBy(box);
          world.hardSurfaces.push_back(box);
        } break;
        case Step::Coins:
          if (draw.shapeIndex == ShapeType::Key)
            setKey(draw, world);
          else
            pushCoin(draw, world);
          break;
        case Step::Ghosts: pushGhost(draw, world, frame); break;
      }
    }
  }

  static const Door getDoor(const std::vector<Door>& doors,
                            const std::string&       doorId)
  {
    for (const Door& door : doors) {
      SOLEIL__LOGGER_DEBUG(toString("'", door.id, "'"));

      if (door.id == doorId) return door;
    }

    throw std::runtime_error(toString("No door found with id ='", doorId, "'"));
  }

  void InitializeLevel(World& world, const std::string& doorId, Frame& frame,
                       Camera& camera, PopUp& caption)
  {
    frame.pointLights.push_back(
      {Position(0.0f), gval::cameraLight, .000010, .00301f});

    world.lastDoor = doorId;

    const Door         start = getDoor(world.doors, doorId);
    const std::string  level = AssetService::LoadAsString(start.level);
    std::istringstream s(level);
    loadMap(world, frame, s);

    caption.fillText(start.name, 1.f);
    caption.activate(gval::timeToFadeText, frame.time);

    camera.position =
      (start.triggerZone.getMax() + start.triggerZone.getMin()) / 2.0f;
    camera.position.y = 1.0f;

    for (const Door& door : world.doors) {
      int state = (door.id == doorId) ? TriggerState::CurrentlyActive
                                      : TriggerState::NeverTriggered;

      if (start.level == door.level)
        world.triggers.push_back({door.triggerZone, state, TriggerType::Door,
                                  std::hash<std::string>{}(door.output)});
    }

#if 0
    // Test: Render Bézier to image
    const int width     = 800;
    const int height    = 600;
    const int imageSize = width * height;

    float               A = 0.0f;
    float               B = 0.5f;
    float               C = 0.9f;
    float               D = 1.0f;
    std::vector<GLuint> image(width * height, 0xFFFFFFFF);
    for (int x = 0; x < width; ++x) {
      {
        int y = bezier(A, B, C, D, ((float)x / width)) / 2.0f * height;
        if (y >= height) y = height - 1;

        int index = width * y + x;
        assert(index < imageSize && "Overflowing image");
        image[index] = 0xFF0000FF;
      }

      {
	int y = glm::mix(0.0f, (float)(height / 2), ((float)x / width));
        int index = width * y + x;
        assert(index < imageSize && "Overflowing image");
        image[index] = 0xFF00FF00;
      }

      {
        int index = width * (height / 2) + x;
        assert(index < imageSize && "Overflowing image");
        image[index] = 0xFFFF0000;
      }
    }
    gl::Texture&    texture = OpenGLDataInstance::Instance().textures.back();
    gl::BindTexture bindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, image.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gval::bezierTex = *texture;

#endif
  }

  std::string DoorUIDToId(const std::vector<Door>& doors, const std::size_t uid)
  {
    for (const auto& d : doors) {
      if (d.uid == uid) return d.id;
    }
    assert(false && "No Doors found");
    return doors[0].id;
  }

  Door* GetDoorByUID(std::vector<Door>& doors, const std::size_t uid)
  {
    for (auto& d : doors) {
      if (d.uid == uid) return &d;
    }
    assert(false && "No Doors found for given UID");
    return nullptr;
  }

  GhostData::GhostData(glm::mat4* transformation, size_t lightPosition,
                       const glm::vec3& direction)
    : transformation(transformation)
    , lightPosition(lightPosition)
    , direction(direction)
  {
  }

  void GhostData::updateBounds(void) noexcept
  {
    bounds       = BoundingBox(*this->transformation, 0.35f);
    stressBounds = BoundingBox(*this->transformation, 5.5f);
  }

} // Soleil
