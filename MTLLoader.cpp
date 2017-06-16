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

#include "stb_image.h"

namespace Soleil {

  typedef std::function<void(const std::string&)> Command;
  typedef std::map<std::string, std::function<void(const std::string&)>>
    CommandMap;

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
    const ImageAsset image(argument);

    OpenGLDataInstance::Instance().textures.emplace_back();
    SOLEIL__LOGGER_DEBUG(toString("After emplace"));

    gl::Texture&    texture = OpenGLDataInstance::Instance().textures.back();
    gl::BindTexture bindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    *textureMaterial = *texture;
  }

  static void commandPushMaterial(std::map<std::string, Material>* materials,
                                  CommandMap*        commandMap,
                                  const std::string& argument)
  {
    using std::placeholders::_1;

    materials->emplace(argument, Material());

    commandMap->clear();

    commandMap->emplace(
      "newmtl", std::bind(commandPushMaterial, materials, commandMap, _1));

    commandMap->emplace(
      "Ns",
      std::bind(commandParseFloat, &(materials->at(argument).shininess), _1));
    commandMap->emplace(
      "Ka",
      std::bind(commandParseVec3, &(materials->at(argument).ambiantColor), _1));
    commandMap->emplace(
      "Kd",
      std::bind(commandParseVec3, &(materials->at(argument).diffuseColor), _1));
    commandMap->emplace(
      "Ks", std::bind(commandParseVec3,
                      &(materials->at(argument).specularColor), _1));
    commandMap->emplace(
      "Ke", std::bind(commandParseVec3,
                      &(materials->at(argument).emissiveColor), _1));
    commandMap->emplace(
      "map_Kd",
      std::bind(commandLoadTexture, &(materials->at(argument).diffuseMap), _1));
    commandMap->emplace("Ni", std::bind(commandNOOP, "Ni", _1));
    commandMap->emplace("d", std::bind(commandNOOP, "d", _1));
    commandMap->emplace("illum", std::bind(commandNOOP, "illum", _1));
    commandMap->emplace("map_Ks", std::bind(commandNOOP, "map_Ks", _1));
    commandMap->emplace("map_Bump", std::bind(commandNOOP, "map_Bump", _1));
  }

  class MTLCommands
  {
  public:
    MTLCommands()
    {
      using std::placeholders::_1;

      map.emplace("newmtl",
                  std::bind(commandPushMaterial, &materials, &map, _1));
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
    std::map<std::string, Material> materials;

  private:
    CommandMap map;
  };

  std::map<std::string, Material> MTLLoader::fromContent(
    const std::string& content)
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
