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

#include "Node.hpp"
#include "Logger.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

namespace Soleil {

  Node::Node(HashType type, const std::string& name)
    : Object(type, name)
    , transformation()
    , parent(nullptr)
  {
  }

  Node::~Node() {}

  void Node::setTransformation(const glm::mat4& transformation)
  {
    this->nodeTransformation = transformation;
    if (parent) {
      this->transformation = parent->getTransformation() * nodeTransformation;
    } else {
      this->transformation = this->nodeTransformation;
    }
  }

  const glm::mat4& Node::getTransformation(void) const noexcept
  {
    return transformation;
  }

  const glm::mat4& Node::getLocalTransformation(void) const noexcept
  {
    return nodeTransformation;
  }

  Node* Node::getParent(void) const noexcept { return parent; }

  void Node::setParent(Node* parent) { this->parent = parent; }

  void Node::translate(glm::vec3 translation)
  {
    setTransformation(glm::translate(nodeTransformation, translation));
  }

  void Node::updatePositionFromParent(void)
  {
    assert(parent != nullptr && "Method should be used only for a child");
    this->transformation = parent->getTransformation() * nodeTransformation;
  }

  std::ostream& operator<<(std::ostream& os, const Node& n)
  {
    os << "Node(" << n.getClassName();
    if (n.getName().empty() == false) {
      os << ": " << n.getName();
    }
    os << ")";
    return os;
  }

} // Soleil
