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

#include "MTLLoader.hpp"

#include "AssetService.hpp"
#include "Logger.hpp"
#include "OpenGLDataInstance.hpp" // TODO: Temporary
#include "TypesToOStream.hpp"

#include <functional>
#include <map>

// TODO: Export in a separate file
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

namespace Soleil {

  typedef std::function<void(const std::string&)> Command;

  static void commandNOOP(const std::string& command,
                          const std::string& arguments)
  {
    SOLEIL__LOGGER_DEBUG("Noop for command: ", command, " with arguments: ",
                         arguments);
  }

  static void commandParseVec3(glm::vec3* vec3, const std::string& arguments)
  {
    std::istringstream s(arguments);

    s >> vec3->r;
    s >> vec3->g;
    s >> vec3->b;
  }

  static void commandParseFloat(float* value, const std::string& arguments)
  {
    std::istringstream s(arguments);

    s >> *value;
  }

  static void commandLoadTexture(GLint*             textureMaterial,
                                 const std::string& argument)
  {
    // TODO: A solution could be to measure the size of the Textures array and
    // fix it on Release Compile time.

    int width          = -1;
    int height         = -1;
    int channelsInFile = 0;

    const auto     encodedImageData = AssetService::LoadAsDataVector(argument);
    stbi_set_flip_vertically_on_load(1);
    unsigned char* image =
      stbi_load_from_memory(encodedImageData.data(), encodedImageData.size(),
                            &width, &height, &channelsInFile, 4);

    if (image == nullptr) {
      throw std::runtime_error(toString("Failed to decode image '", argument,
                                        "': ", stbi_failure_reason()));
      stbi_image_free(image); // TODO: Use RAII
    }
    SOLEIL__LOGGER_DEBUG(toString("Loaded image: ", argument, " -> ", width,
                                  "x", height, ". Original number of chanels: ",
                                  channelsInFile));

    OpenGLDataInstance::Instance().textures.emplace_back();

    gl::Texture&    texture = OpenGLDataInstance::Instance().textures.back();
    gl::BindTexture bindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);


    *textureMaterial = *texture;


    stbi_image_free(image); // TODO: Use RAII
  }

  class MTLCommands
  {
  public:
    MTLCommands()
    {
      materials.push_back(Material());

#if 1 // Test with texture
      materials[0].diffuseMap = *OpenGLDataInstance::Instance().textureTest;
#endif

      using std::placeholders::_1;

      // TODO: Multiple Materials:
      map.emplace("newmtl", std::bind(commandNOOP, "newmtl", _1));

      map.emplace("Ns",
                  std::bind(commandParseFloat, &(materials[0].shininess), _1));
      map.emplace(
        "Ka", std::bind(commandParseVec3, &(materials[0].ambiantColor), _1));
      map.emplace(
        "Kd", std::bind(commandParseVec3, &(materials[0].diffuseColor), _1));
      map.emplace(
        "Ks", std::bind(commandParseVec3, &(materials[0].specularColor), _1));
      map.emplace(
        "Ke", std::bind(commandParseVec3, &(materials[0].emissiveColor), _1));
#if 1
      map.emplace("map_Kd", std::bind(commandLoadTexture,
                                      &(materials[0].diffuseMap), _1));
#else
      map.emplace("map_Kd", std::bind(commandNOOP, "map_Kd", _1));
#endif

      map.emplace("Ni", std::bind(commandNOOP, "Ni", _1));
      map.emplace("d", std::bind(commandNOOP, "d", _1));
      map.emplace("illum", std::bind(commandNOOP, "illum", _1));
      map.emplace("map_Ks", std::bind(commandNOOP, "map_Ks", _1));
      map.emplace("map_Bump", std::bind(commandNOOP, "map_Bump", _1));
    }

    virtual ~MTLCommands() {}

  public:
    Command operator[](const std::string& command)
    {
      if (map.find(command) == map.end()) {
        throw std::runtime_error(toString("Unknown command: '", command, "'"));
      }
      return map[command];
    }

  public:
    std::vector<Material> materials;

  private:
    std::map<std::string, std::function<void(const std::string&)>> map;
  };

  std::vector<Material> MTLLoader::fromContent(const std::string& content)
  {
    std::istringstream in(content);
    MTLCommands        commands;

    for (std::string line; getline(in, line);) {
      if (trim(line).empty() == false && line[0] != '#') {
        auto pos = line.find(' ');
        if (pos == std::string::npos || pos < 1) {
          throw std::runtime_error(
            toString("Unable to understand line: ", line));
        }

        const std::string command   = line.substr(0, pos);
        const std::string arguments = line.substr(pos + 1);

        commands[command](arguments);
      }
    }

    return commands.materials;
  }

} // Soleil
