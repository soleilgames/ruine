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

#include "Group.hpp"
#include "Node.hpp"
#include "TypesToOStream.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Soleil;

static void
NodeTranslation()
{
  Node      n(0, "test");
  glm::mat4 transformation =
    glm::translate(glm::mat4(), glm::vec3(21.0f, 42.0f, 84.42f));
  n.translate(glm::vec3(21.0f, 42.0f, 84.42f));

  mcut::assertEquals(transformation, n.getTransformation());
}

static void
GroupAndChildren()
{
  Group   boat;
  NodePtr passenger1 = std::make_shared<Node>(0, "passenger1");

  boat.addChild(passenger1);

  // Sailing on our boat
  const glm::mat4 transformation =
    glm::translate(glm::mat4(), glm::vec3(10.0f, 0.0f, 0.0f));
  boat.setTransformation(transformation);

  // The passenger as also moved:
  mcut::assertEquals(transformation, passenger1->getTransformation());  
}

int
main(int, char* [])
{
  mcut::TestSuite basics("Basics");
  basics.add(NodeTranslation);
  basics.run();

  mcut::TestSuite groups("Groups");
  groups.add(GroupAndChildren);
  groups.run();

  return 0;
}
