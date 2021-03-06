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

#ifndef SOLEIL__NODE_HPP_
#define SOLEIL__NODE_HPP_

#include "Object.hpp"

#include <memory>

#include <glm/mat4x4.hpp>

namespace Soleil {

  /**
   * 
   */
  //class Group;
  
  class Node : public Object
  {
  public:
    Node(HashType type, const std::string& name);
    virtual ~Node();

  public:
    virtual void setTransformation(const glm::mat4& transformation);
    const glm::mat4& getTransformation(void) const noexcept;
    const glm::mat4& getLocalTransformation(void) const noexcept;

  public:
    Node* getParent(void) const noexcept;
    void setParent(Node* parent);

  public:
    void translate(glm::vec3 translation);

  protected:
    virtual void updatePositionFromParent(void);

  private:
    glm::mat4 transformation;     // World transformation
    glm::mat4 nodeTransformation; // Transformation of this node
    Node*     parent;

    friend class Group;
  };

  typedef std::shared_ptr<Node> NodePtr;

  std::ostream& operator<<(std::ostream& os, const Node& node);

} // Soleil

#endif /* SOLEIL__NODE_HPP_ */
