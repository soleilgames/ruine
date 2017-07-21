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

namespace Soleil {

  void InitializeWorldModels(World& world)
  {
    world.wallShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("wallcube.obj"));
    world.floorShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("floor.obj"));
    world.torchShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("brazero.obj"));
    world.gateShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("gate.obj"));
#if 1
    world.ghostShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("ghost.obj"));
#else
    world.ghostShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("brazero.obj"));
#endif

#if 0    
    world.purseShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("purse01.obj"));
#else
    world.purseShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("coin.obj"));
#endif

    world.keyShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("key.obj"));

    world.barrelShape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("barrel2.obj"));

#if 0
        world.barrel2Shape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("barrel2.obj"));
#endif
  }

  void InitializeWorldDoors(World& world, const std::string& assetName)
  {
    std::istringstream file(AssetService::LoadAsString(assetName));
    world.doors.clear();

    std::string line;
    while (std::getline(file, line)) {
      if (line[0] == '#') continue;

      std::istringstream sline(line);
      Door               d;
      sline >> d.id;
      sline >> d.level;
      sline >> d.aoe.x;
      sline >> d.aoe.y;
      sline >> d.output;
      sline >> d.start.x;
      sline >> d.start.y;

      SOLEIL__LOGGER_DEBUG(toString("d.id", d.id));
      SOLEIL__LOGGER_DEBUG(toString("d.id", d.id));
      SOLEIL__LOGGER_DEBUG(toString("d.level", d.level));
      SOLEIL__LOGGER_DEBUG(toString("d.aoe", d.aoe));
      SOLEIL__LOGGER_DEBUG(toString("d.output", d.output));
      SOLEIL__LOGGER_DEBUG(toString("d.start", d.start));

      std::string name;
      sline.get(); // Consume \t
      std::getline(sline, name);
      d.name = StringToWstring(name);
      world.doors.push_back(d);
    }
  }

  void World::resetLevel()
  {
    bounds = glm::vec3(0.0f);
    objects.clear();
    statics.clear();
    sentinels.clear();
    hardSurfaces.clear();
    nextZoneTriggers.clear();
    coinTriggers.clear();
  }



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
              glm::scale(glm::translate(scale, keyPosition), glm::vec3(2.0f));

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

  static const Door getDoor(const std::vector<Door>& doors,
                            const std::string&       doorId)
  {
    for (const Door& door : doors) {
      if (door.id == doorId) return door;
    }

    throw std::runtime_error(toString("No door found with id =", doorId));
  }

  void InitializeLevel(World& world, const std::string& doorId, Frame& frame,
                       Camera& camera, PopUp& caption)
  {
    frame.pointLights.push_back(
      {Position(0.0f), gval::cameraLight, .000010, .00301f});
    world.bounds.y =
      1.0f; // TODO: To change if the world become not flat anymore

    const Door         start = getDoor(world.doors, doorId);
    const std::string  level = AssetService::LoadAsString(start.level);
    std::istringstream s(level);
    parseMaze(world, s, frame);

    caption.fillText(start.name, 0.45f);
    caption.activate(gval::timeToFadeText, frame.time);

    // The '2 Times (*= 2.0f)' works because position are in 'lines and
    // columns space'
    // and everything has the same size. It will not work anymore if blocks
    // may have different size and shape.
    const static glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(1.0f));
    const glm::vec3 position(2.0f * start.start.x, 0.0f, 2.0f * start.start.y);
    camera.position = glm::vec3(scale * glm::vec4(position, 1.0f));

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
