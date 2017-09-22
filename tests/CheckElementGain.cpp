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

#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>
#include <cmath>

#include "mcut.hpp"

typedef std::map<std::string, int> Counter;

static int
getMatch(const std::string& line, Counter* count = nullptr)
{
  if (line.substr(0, 2) != "f ") return 0;

  std::string             faces = line.substr(2);
  static const std::regex faceRegex("[0-9]+(/([0-9]*)(/[0-9]+)?)?");

  std::smatch vertexMatch;
  int         vertexCount = 0;
  while (std::regex_search(faces, vertexMatch, faceRegex)) {
    vertexCount++;

    if (count) {
      (*count)[std::string(vertexMatch[0])]++;

#ifndef NDEBUG
      std::cout << vertexMatch[0]
                << ": count=" << (*count)[std::string(vertexMatch[0])]
                << ". (size=" << vertexMatch.size() << ")"
                << "\n";
#endif
    }

    faces = vertexMatch.suffix().str();
  }

  return vertexCount;
}

/**
 * Count the number of vertex that is duplicated in a Wavefront file (.obj) and
 * print out the gain to use indices. Gain is between 0% (no duplicate vertex)
 * and 100%.
 *
 * I consider only the following format:
 * clang-format off
 * f 1 2 3
 * f 3/1 4/2 5/3
 * f 6/4/1 3/5/3 7/6/5
 * f 7//1 8//2 9//3
 * clang-format on
 */
static int
checkGain(const std::string fileName)
{
  std::ifstream file(fileName);

  int     vertexCount = 0;
  Counter counter;
  for (std::string line; getline(file, line);) {
    vertexCount += getMatch(line, &counter);
  }

  if (vertexCount == 0) return 0;
  return 100 - (int)std::round((float)counter.size() * 100.0f / (float)vertexCount);
}

/**
 * Usage: ./checkgain file-1 [file-2 [...]]
 */
int
main(int argc, char* argv[])
{
#ifndef NDEBUG
  // A (TDD) unit test :)

  mcut::assertEquals(3, getMatch("f 1 2 3"));

  mcut::assertEquals(3, getMatch("f 3/1 4/2 5/3"));
  mcut::assertEquals(3, getMatch("f 6/4/1 3/5/3 7/6/5"));

  mcut::assertEquals(3, getMatch("f 7//1 8//2 9//3"));

  mcut::assertEquals(0, getMatch("blah f 7//1 8//2 9//3 9//3"));

#endif

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " file-1 [file-2 [...]]" << std::endl;
  }

  std::vector<std::string> arguments(argv + 1, argv + argc);

  for (const auto& file : arguments) {
    std::cout << "File:" << file << "\t\t Gain: " << checkGain(file) << "%\n";
  }

  return 0;
}
