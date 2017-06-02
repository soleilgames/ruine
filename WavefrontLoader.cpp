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

#include <functional>
#include <istream>
#include <map>
#include <regex>
#include <set>

#include "Logger.hpp"
#include "WavefrontLoader.hpp"

namespace Soleil {

  // TODO: using iostream >> (float) skip parsing errors

  GLushort findOrEmplace(std::vector<Vertex>& vertexElements,
                         const Vertex&        vertex)
  {
    // TODO: Yep, might worth to find a better algorithm.
    GLushort index = 0;

    while (index < vertexElements.size()) {
      if (vertexElements[index] == vertex) return index;
      index++;
    }

    vertexElements.push_back(vertex);
    return vertexElements.size() - 1;
  }

  struct ObjectStore
  {
    std::vector<glm::vec4> vertices;
    std::vector<glm::vec2> textureCoords;
    std::vector<glm::vec3> normals;

    std::vector<Vertex>   vertexElements;
    std::vector<GLushort> indexElements;
  };

  void commandNOOP(const std::string& command, const std::string& arguments)
  {
    SOLEIL__LOGGER_DEBUG("Noop for command: ", command, " with arguments: ",
                         arguments);
  }

  void commandVector(std::vector<glm::vec4>* vertices,
                     const std::string&      arguments)
  {
    glm::vec4 v;

    std::istringstream s(arguments);
    s >> v.x;
    s >> v.y;
    s >> v.z;
    v.w = 1.0f;

    vertices->push_back(v);
  }

  void commandTextureCoords(std::vector<glm::vec2>* textureCoords,
                            const std::string&      arguments)
  {
    glm::vec2 v;

    std::istringstream s(arguments);
    s >> v.x;
    s >> v.y;

    textureCoords->push_back(v);
  }

  void commandNormals(std::vector<glm::vec3>* normals,
                      const std::string&      arguments)
  {
    glm::vec3 v;

    std::istringstream s(arguments);
    s >> v.x;
    s >> v.y;
    s >> v.z;

    normals->push_back(v);
  }

  void commandMatchFace(ObjectStore* store, const std::string& arguments)
  {
    static const std::regex faceVertex("([0-9]+)(/([0-9]*)(/([0-9]+))?)?");
    enum GROUPS
    {
      full            = 0,
      verticesElement = 1,
      textures        = 3,
      normals         = 5
    };

    std::string inprogress = arguments;
    std::smatch matched;

    // TODO: An ugly hack to skip the fact there is no lights nor materials
    // yet
    glm::vec4 color;
    {
      static int coco = 0;
      switch (coco) {
        case 0: color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); break;
        case 1: color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); break;
        case 2: color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); break;
        case 3: color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); break;
        case 4: color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f); break;
        case 5: color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); break;
      }
      coco++;
      if (coco > 5) coco = 0;
    }

    int i = 0;
    while (std::regex_search(inprogress, matched, faceVertex)) {
      if (i > 2) throw std::runtime_error("Only triangle faces are supported");

      glm::vec4 position;
      glm::vec3 normal(1.0f);
      glm::vec2 uv(-1.0f);

      {
        GLushort element = std::stoi(matched[GROUPS::verticesElement]);
        position         = store->vertices.at(element - 1);
      }

      if (matched[GROUPS::normals].length() > 0) {
        GLushort element = std::stoi(matched[GROUPS::normals]);
        normal           = store->normals.at(element - 1);
      }

      if (matched[GROUPS::textures].length() > 0) {
        GLushort element = std::stoi(matched[GROUPS::textures]);
        uv               = store->textureCoords.at(element - 1);
      }

      Vertex v(position, normal, color, uv);
      store->indexElements.push_back(findOrEmplace(store->vertexElements, v));

      inprogress = matched.suffix().str();
      i++;
    }
  }

  typedef std::function<void(const std::string&)> Command;

  class Commands
  {
  public:
    Commands()
    {
      using std::placeholders::_1;

      this->map.emplace("mtllib", std::bind(commandNOOP, "mtllib", _1));
      this->map.emplace("o", std::bind(commandNOOP, "o", _1));
      this->map.emplace("s", std::bind(commandNOOP, "s", _1));
      this->map.emplace("usemtl", std::bind(commandNOOP, "usemtl", _1));

      this->map.emplace("v", std::bind(commandVector, &(store.vertices), _1));
      this->map.emplace(
        "vt", std::bind(commandTextureCoords, &(store.textureCoords), _1));
      this->map.emplace("vn", std::bind(commandNormals, &(store.normals), _1));

      this->map.emplace("f", std::bind(commandMatchFace, &store, _1));
    }

  public:
    Command operator[](const std::string& command)
    {
      if (map.find(command) == map.end()) {
        throw std::runtime_error(toString("Unknown command: '", command, "'"));
      }
      return map[command];
    }

  public:
    ObjectStore store;

  private:
    std::map<std::string, std::function<void(const std::string&)>> map;
  };

  std::shared_ptr<Shape> WavefrontLoader::fromContent(
    const std::string& content)
  {
    std::istringstream in(content);
    Commands           commands;

    for (std::string line; getline(in, line);) {
      if (line[0] != '#') {
        auto pos = line.find(' ');
        if (pos == std::string::npos || pos < 1) {
          throw std::runtime_error(
            toString("Unable to understand line: ", line));
        }

        const std::string command   = line.substr(0, pos);
        const std::string arguments = line.substr(pos + 1);

        // SOLEIL__LOGGER_DEBUG("COMMAND='", command, "' ARGUMENT='", arguments,
        //                      "'");
        commands[command](arguments);
      }
    }

    std::vector<Vertex> vertices;

    for (GLushort index : commands.store.indexElements) {
      vertices.push_back(commands.store.vertexElements[index]);
    }

    return std::make_shared<Shape>(vertices);
  }

} // Soleil
