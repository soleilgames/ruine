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
  }

  void InitializeLevelFromAsset(World& world, const std::string& asset,
                                Frame& frame, Camera& camera)
  {
    InitializeLevel(world, AssetService::LoadAsString(asset), frame, camera);
  }

  void World::resetLevel()
  {
    bounds = glm::vec3(0.0f);
    statics.clear();
    sentinels.clear();
    hardSurfaces.clear();
    nextZoneTriggers.clear();
  }

  void InitializeLevel(World& world, const std::string& level, Frame& frame,
                       Camera& camera)
  {
    frame.pointLights.push_back(
      {Position(0.0f), gval::cameraLight, 0.027f, 0.0028f});

    RenderInstances lateComer;
    // Prepare vertices for the bounding box:
    std::vector<glm::vec4> wallVertices;
    for (const auto& sub : world.wallShape->getSubShapes()) {
      for (const auto& vertex : sub.vertices) {
        wallVertices.push_back(vertex.position);
      }
    }

    world.bounds.y =
      1.0f; // TODO: To change if the world become not flat anymore

    std::istringstream     s(level);
    std::string            line;
    float                  x           = 0.0f;
    float                  z           = 0.0f;
    bool                   positionSet = false;
    const static glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(1.0f));
    while (std::getline(s, line)) {
      for (auto const& c : line) {
        const glm::vec3 position(2.0f * x, 0.0f, 2.0f * z);

        if (c == 'x') {
          const glm::mat4 transformation = glm::translate(scale, position);

          world.statics.push_back(
            DrawCommand(*world.wallShape, transformation));

          BoundingBox bbox;
          for (const auto& v : wallVertices) {
            bbox.expandBy(glm::vec3(transformation * v));
          }
          world.hardSurfaces.push_back(bbox);

          world.bounds.x = glm::max(world.bounds.x, bbox.getMax().x);
          world.bounds.z = glm::max(world.bounds.z, bbox.getMax().z);
        } else if (c == 'G') {
          const glm::mat4 transformation = glm::translate(scale, position);

          world.statics.push_back(
            DrawCommand(*world.gateShape, transformation));

          BoundingBox bbox;
          for (const auto& v : wallVertices) {
            bbox.expandBy(glm::vec3(transformation * v));
          }
          world.hardSurfaces.push_back(bbox);

          world.bounds.x = glm::max(world.bounds.x, bbox.getMax().x);
          world.bounds.z = glm::max(world.bounds.z, bbox.getMax().z);

          world.nextZoneTriggers.push_back({bbox, "intro.level"});
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
          } else if (c == 'D') {
            assert(positionSet == false && "Position already set");

            camera.position = glm::vec3(scale * glm::vec4(position, 1.0f));
            positionSet     = true;
          } else if (c == 'd') {
            assert(positionSet == true && "Position not set yet");

            glm::vec3 dir = glm::normalize(position - camera.position);
            camera.yaw    = std::atan2(dir.x, dir.z);
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

} // Soleil
