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
#include "Logger.hpp"
#include "TypesToOStream.hpp"

#include <functional>
#include <map>

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

  class MTLCommands
  {
  public:
    MTLCommands()
    {
      materials.push_back(Material());

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

      map.emplace("Ke", std::bind(commandNOOP, "ke", _1));
      map.emplace("Ni", std::bind(commandNOOP, "Ni", _1));
      map.emplace("d", std::bind(commandNOOP, "d", _1));
      map.emplace("illum", std::bind(commandNOOP, "illum", _1));
      map.emplace("map_Ks", std::bind(commandNOOP, "map_Ks", _1));
      map.emplace("map_Kd", std::bind(commandNOOP, "map_Kd", _1));
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
