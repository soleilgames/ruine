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

#include "Recorder.hpp"
#include "Logger.hpp"

#include <fstream>

#include <glm/gtx/string_cast.hpp>

namespace Soleil {

  std::vector<Recorder::Record> Recorder::records;

  std::size_t Recorder::currentFrameHash;

  
  void Recorder::recordInput(int active, const glm::vec2& start,
                             const glm::vec2& position)
  {
    records.push_back({active, start.x, start.y, position.x, position.y, 0});
  }

  std::size_t Recorder::hashFrame(const glm::vec3& cameraPosition, int state,
                             int score, int keyPickedUp)
  {
    std::size_t h = 0;

    // hash_combine(h, trail_point<3>(cameraPosition.x));
    // hash_combine(h, trail_point<3>(cameraPosition.y));
    // hash_combine(h, trail_point<3>(cameraPosition.z));
    hash_combine(h, state);
    hash_combine(h, score);
    hash_combine(h, keyPickedUp);


    return h;
  }
 
  
  void Recorder::recordFrame(const glm::vec3& cameraPosition, int state,
                             int score, int keyPickedUp)
  {

    assert(records.size() > 0);

    records.back().frameHash = hashFrame(cameraPosition, state, score, keyPickedUp);

    SOLEIL__LOGGER_DEBUG(
      records.back().pushState, ", ", records.back().startx, ", ",
      records.back().starty, ", ", records.back().positionx, ", ",
      records.back().positiony, ", ", records.back().frameHash,
      "(camera=", glm::to_string(cameraPosition), ", state=", state,
      ", score=", score, ", key=", keyPickedUp

      );
  }

  void Recorder::dumpRecords(const std::string& fileName)
  {
    // TODO: Use Asset manager
    std::fstream dump(fileName,
                      std::ios::out | std::ios::binary | std::ios::trunc);
    dump.exceptions(std::ifstream::failbit);

    const Record* data = records.data();
    dump.write(reinterpret_cast<const char*>(records.data()),
               sizeof(*data) * records.size());

    dump.close();
  }

  void Recorder::loadRecords(const std::string& fileName)
  {
    const long fileSize = [fileName]() {
      std::ifstream in(fileName, std::ifstream::ate | std::ifstream::binary);
      return in.tellg();
    }();

    if (fileSize % sizeof(Record) != 0)
      throw std::runtime_error("Recorder load failed: Not a integer row count");

    std::fstream dump(fileName, std::ios::in);
    dump.exceptions(std::ifstream::failbit);

    records.clear();
    const std::size_t numberOfRecords = fileSize / sizeof(Record);
    records.resize(numberOfRecords);
    dump.read(reinterpret_cast<char*>(records.data()), fileSize);

    SOLEIL__LOGGER_DEBUG("Red: ", dump.gcount(), ". FileSize=", fileSize,
                         ". Record Size=", sizeof(Record));
    assert(dump.gcount() == fileSize);

    // TODO: Convert endianess if required
  }

} // Soleil
