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
    world.shapes = {Soleil::WavefrontLoader::fromContent(
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
                    Soleil::WavefrontLoader::fromContent(
                      AssetService::LoadAsString("ghost.obj")),
                    Soleil::WavefrontLoader::fromContent(
                      AssetService::LoadAsString("couloir.obj"))};
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
    nextZoneTriggers.clear();
    coinTriggers.clear();

    ghosts.clear();
    items.clear();
    elements.clear();
    triggers.clear();
  }

#if 0
  static void parseMaze(World& world, std::istringstream& s, Frame& frame)
  {
    std::string            line;
    float                  x     = 0.0f;
    float                  z     = 0.0f;
    const static glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(1.0f));

    RenderInstances lateComer;

    while (std::getline(s, line) && line.size() > 0) {
      for (auto const& c : line) {
        const glm::vec3 position(2.0f * x, 0.0f, 2.0f * z);

        if (c == 'x') {
          const glm::mat4 transformation = glm::translate(scale, position);

          world.statics.push_back(
            DrawCommand(*world.wallShape, transformation));

          BoundingBox box = world.wallShape->makeBoundingBox();
          box.transform(transformation);
          world.hardSurfaces.push_back(box);

          world.bounds.x = glm::max(world.bounds.x, box.getMax().x);
          world.bounds.z = glm::max(world.bounds.z, box.getMax().z);
        } else if (c == 'G') {
          const glm::mat4 transformation = glm::translate(scale, position);

          world.statics.push_back(
            DrawCommand(*world.gateShape, transformation));

          BoundingBox box = world.wallShape->makeBoundingBox();
          box.transform(transformation);
          world.hardSurfaces.push_back(box);

          world.bounds.x = glm::max(world.bounds.x, box.getMax().x);
          world.bounds.z = glm::max(world.bounds.z, box.getMax().z);
        } else {
          if (c == 'l') {
            PointLight p;

            p.color    = glm::vec3(0.5f, 0.0f, 0.0f);
            p.position = glm::vec3(scale * glm::vec4(position, 1.0f));
            frame.pointLights.push_back(p);

            glm::mat4 transformation =
              glm::translate(glm::mat4(),
                             glm::vec3(position.x, -1.0f, position.z)) *
              glm::scale(glm::mat4(), glm::vec3(0.5f));
            world.statics.push_back(
              DrawCommand(*world.torchShape, transformation));
          } else if (c == 'g') {
            glm::mat4 transformation =
              glm::translate(glm::mat4(),
                             glm::vec3(position.x, -0.60f, position.z)) *
              glm::scale(glm::mat4(), glm::vec3(.3f));
            lateComer.push_back(DrawCommand(*world.ghostShape, transformation));
          } else if (c == 'p') {
            const glm::vec3 coinPosition =
              glm::vec3(position.x, -1.0f, position.z);
            const glm::mat4 transformation =
              glm::translate(scale, coinPosition);
            auto pos = std::find(std::begin(world.coinPickedUp),
                                 std::end(world.coinPickedUp), transformation);
            if (pos == std::end(world.coinPickedUp)) {
              // TODO: Do put in a specific vector;
              world.objects.push_back(
                DrawCommand(*world.purseShape, transformation));

              BoundingBox bbox(transformation, 0.3f);
              world.coinTriggers.push_back({bbox, transformation});
            }
          } else if (c == 'k') {
            const glm::vec3 keyPosition =
              glm::vec3(position.x, -1.0f, position.z);
            const glm::mat4 transformation =
              glm::scale(glm::translate(scale, keyPosition), glm::vec3(4.0f));

            world.objects.push_back(
              DrawCommand(*world.keyShape, transformation));
            world.theKey = transformation;
          } else if (c == 'b' || c == 'B') {
            const glm::vec3 barrelPosition =
              glm::vec3(position.x, -1.0f, position.z);
            const glm::mat4 transformation =
              glm::translate(scale, barrelPosition);

            BoundingBox box = world.barrelShape->makeBoundingBox();
            box.transform(transformation);
            world.hardSurfaces.push_back(box);
#if 0
	    if (c == 'b')
            world.statics.push_back(
              DrawCommand(*world.barrel2Shape, transformation));
	    else
#endif
            world.statics.push_back(
              DrawCommand(*world.barrelShape, transformation));
          } else if (c == 'A') {
            glm::mat4 transformation =
              glm::rotate(
                glm::translate(glm::mat4(),
                               glm::vec3(position.x, -0.60f, position.z)),
                glm::half_pi<float>(), glm::vec3(0, 1, 0)) *
              glm::scale(glm::mat4(), glm::vec3(.3f));

            world.statics.push_back(
              DrawCommand(*world.ghostShape, transformation));
          }

          glm::mat4 groundTransformation =
            glm::translate(scale, glm::vec3(2.0f * x, -1.0f, 2.0f * z));

          glm::mat4 ceilTransformation =
            glm::translate(scale, glm::vec3(2.0f * x, 1.0f, 2.0f * z)) *
            glm::rotate(glm::mat4(), glm::pi<float>(),
                        glm::vec3(0.0f, 0.0f, 1.0f));

          world.statics.push_back(
            DrawCommand(*world.floorShape, groundTransformation));
          world.statics.push_back(
            DrawCommand(*world.floorShape, ceilTransformation));
        }
        x += 1.0f;
      }
      z += 1.0f;
      x = 0.0f;
    }

    // TODO: In a future release move the ghost in a specific vector instead of
    // using the static one. +1 is for the player ghost.
    world.statics.reserve(world.statics.size() + lateComer.size() + 1);

    for (size_t i = 0; i < lateComer.size(); ++i) {
      const auto& o = lateComer[i];

      world.statics.push_back(o);

      PointLight p;

      p.color     = gval::ghostColor;
      p.position  = glm::vec3(world.statics.back().transformation[3]);
      p.linear    = 0.09f;
      p.quadratic = 0.032f;
      frame.pointLights.push_back(p);

      world.sentinels.push_back(
        GhostData(&(world.statics.back().transformation),
                  frame.pointLights.size() - 1, glm::vec3(0, 0, 1)));
    }
  }
#endif

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
    BoundingBox box = world.shapes[ShapeType::Ghost]->makeBoundingBox();
    box.transform(draw.transformation);
    world.ghosts.push_back(draw);

    // TODO: Will replace ghost data?
    world.triggers.push_back({box, // TODO: use gval
                              TriggerState::NeverTriggered, TriggerType::Ghost,
                              0});

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

    caption.fillText(start.name, 0.45f);
    caption.activate(gval::timeToFadeText, frame.time);

#if 0    
    // The '2 Times (*= 2.0f)' works because position are in 'lines and
    // columns space'
    // and everything has the same size. It will not work anymore if blocks
    // may have different size and shape.
    const static glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(1.0f));
    const glm::vec3 position(2.0f * start.start.x, 0.5f, 2.0f * start.start.y);
    camera.position = glm::vec3(scale * glm::vec4(position, 1.0f));
#endif
    camera.position =
      (start.triggerZone.getMax() + start.triggerZone.getMin()) / 2.0f;
    camera.position.y = 0.6f;

    for (const Door& door : world.doors) {
      int state = (door.id == doorId) ? TriggerState::CurrentlyActive
                                      : TriggerState::NeverTriggered;

      if (start.level == door.level)
        world.triggers.push_back({door.triggerZone, state, TriggerType::Door,
                                  std::hash<std::string>{}(door.output)});
    }

#if 0    
    // Parse the triggers
    for (const Door door : world.doors) {
      if (door.level == start.level) {

        // The '2 Times (*= 2.0f)' works because position are in 'lines and
        // columns space'
        // and everything has the same size. It will not work anymore if blocks
        // may have different size and shape.
        glm::vec3 aoe(door.aoe.x, 0.0f, door.aoe.y);
        aoe *= 2.0f;

        BoundingBox bbox;
        bbox.expandBy(aoe);
        bbox.expandBy(aoe + 1.1f);
        bbox.expandBy(aoe - 1.1f);

        SOLEIL__LOGGER_DEBUG(toString("AOE: ", bbox));

        world.nextZoneTriggers.push_back({bbox, door.output});
      }
    }
#endif

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
