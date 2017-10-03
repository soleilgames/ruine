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

#ifndef SOLEIL__RECORDER_HPP_
#define SOLEIL__RECORDER_HPP_

#include <functional>
#include <vector>

#include <glm/vec2.hpp>

#include "types.hpp"

namespace Soleil {

  class Recorder
  {
  public:
    struct Record
    {
      // Controller:
      int   pushState;
      float startx;
      float starty;
      float positionx;
      float positiony;

      // Frame
      std::size_t frameHash;
    };

  public:
    static std::vector<Record> records;
    static std::size_t currentFrameHash;

    static void recordInput(int active, const glm::vec2& start,
                            const glm::vec2& position);
    static void recordFrame(const glm::vec3& cameraPosition, int state,
                            int score, int keyPickedUp);
    static void dumpRecords(const std::string& fileName);
    static void loadRecords(const std::string& fileName);
    static std::size_t hashFrame(const glm::vec3& cameraPosition, int state,
                                 int score, int keyPickedUp);

#define SOLEIL__RECORD_INPUT(active, start, position)                          \
  ::Soleil::Recorder::recordInput(active, start, position)

#define SOLEIL__RECORD_FRAME(cameraPosition, state, score, keyPickedUp)        \
  ::Soleil::Recorder::recordFrame(cameraPosition, state, score, keyPickedUp)
  };

#define SOLEIL__RECORD_DUMP(fileName) ::Soleil::Recorder::dumpRecords(fileName);

} // Soleil

#endif /* SOLEIL__RECORDER_HPP_ */
