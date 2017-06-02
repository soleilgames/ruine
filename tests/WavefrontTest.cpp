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

#include "mcut.hpp"

#include "WavefrontLoader.hpp"

using namespace Soleil;

void
fullLoad()
{
  const std::string content =
    "# Blender v2.76 (sub 0) OBJ File: 'wallcube.blend'\n"
    "# www.blender.org\n"
    "mtllib wallcube.mtl\n"
    "o Cube\n"
    "v 1.000000 -1.000000 -1.000000\n"
    "v 1.000000 -1.000000 1.000000\n"
    "v -1.000000 -1.000000 1.000000\n"
    "v -1.000000 -1.000000 -1.000000\n"
    "v 1.000000 1.000000 -0.999999\n"
    "v 0.999999 1.000000 1.000001\n"
    "v -1.000000 1.000000 1.000000\n"
    "v -1.000000 1.000000 -1.000000\n"
    "vt 1.001884 0.998109\n"
    "vt 0.001891 1.001884\n"
    "vt -0.001884 0.001891\n"
    "vt 0.998109 -0.001884\n"
    "vn 0.000000 -1.000000 0.000000\n"
    "vn 0.000000 1.000000 0.000000\n"
    "vn 1.000000 0.000000 0.000000\n"
    "vn -0.000000 0.000000 1.000000\n"
    "vn -1.000000 -0.000000 -0.000000\n"
    "vn 0.000000 0.000000 -1.000000\n"
    "usemtl Material\n"
    "s off\n"
    "f 2/1/1 3/2/1 4/3/1\n"
    "f 8/1/2 7/2/2 6/3/2\n"
    "f 5/1/3 6/2/3 2/3/3\n"
    "f 6/1/4 7/2/4 3/3/4\n"
    "f 3/4/5 7/1/5 8/2/5\n"
    "f 1/3/6 4/4/6 8/1/6\n"
    "f 1/4/1 2/1/1 4/3/1\n"
    "f 5/4/2 8/1/2 6/3/2\n"
    "f 1/4/3 5/1/3 2/3/3\n"
    "f 2/4/4 6/1/4 3/3/4\n"
    "f 4/3/5 3/4/5 8/2/5\n"
    "f 5/2/6 1/3/6 8/1/6\n";

  // std::shared_ptr<Shape> shape = WavefrontLoader::fromContent(content);
  // TODO: Cannot do that test yet, need to create an OpenGL Context
}

int
main(int, char* [])
{
  mcut::TestSuite basics("TDD");
  basics.add(fullLoad);
  basics.run();
  return 0;
}
