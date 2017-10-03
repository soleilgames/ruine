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
#include "types.hpp"

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
  Group    boat;
  GroupPtr boatBench  = std::make_shared<Group>();
  NodePtr  passenger1 = std::make_shared<Node>(0, "passenger1");
  NodePtr  passenger2 = std::make_shared<Node>(0, "passenger2");

  boat.addChild(passenger1);
  boat.addChild(boatBench);

  passenger1->translate(
    glm::vec3(0.0f, 1.0f, 0.0f)); // Passenger is on the bridge

  boatBench->addChild(passenger2);

  // Sailing on our boat
  for (int i = 1; i <= 10; ++i) {
    const glm::mat4 transformation =
      glm::translate(glm::mat4(), glm::vec3(i, 0.0f, 0.0f));
    boat.setTransformation(transformation);
  }

  // The passengers have also moved:
  {
    const glm::mat4 transformation =
      glm::translate(glm::mat4(), glm::vec3(10.0f, 1.0f, 0.0f));
    mcut::assertEquals(transformation, passenger1->getTransformation());
  }
  {
    const glm::mat4 transformation =
      glm::translate(glm::mat4(), glm::vec3(10.0f, 0.0f, 0.0f));
    mcut::assertEquals(transformation, passenger2->getTransformation());
  }

  // He saw an iceberg and move to the front of the boat:
  passenger1->translate(glm::vec3(1.0f, 0.0f, 0.0f));

  {
    const glm::mat4 transformation =
      glm::translate(glm::mat4(), glm::vec3(11.0f, 1.0f, 0.0f));
    mcut::assertEquals(transformation, passenger1->getTransformation());
  }
}

void
trail_point_test()
{  
  mcut::assertEquals(123, trail_point<2>(1.23456789));
  mcut::assertEquals(12, trail_point<2>(.123456789));
  mcut::assertEquals(1234567, trail_point<3>(1234.56789));
  mcut::assertEquals(10000, trail_point<4>(1));
  mcut::assertEquals(12340000, trail_point<4>(1234));
  mcut::assertEquals(12345678, trail_point<4>(1234.56789));
  mcut::assertEquals(123456789, trail_point<5>(1234.56789));
  mcut::assertFalse(trail_point<2>(1.23456789) == 12);
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

  mcut::TestSuite trail("Trail Point");
  trail.add(trail_point_test);
  trail.run();

  return 0;
}
