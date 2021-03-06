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

#include "Object.hpp"

#include "Logger.hpp"

#include <iostream>

namespace Soleil {

  Object::Object(HashType type, const std::string& className,
                 const std::string& name)
    : type(type)
    , className(className)
    , name(name)
  {
    SOLEIL__LOGGER_DEBUG(toString("Constructing: ", *this));
  }

  Object::~Object() { SOLEIL__LOGGER_DEBUG(toString("~Destructing: ", *this)); }

  HashType Object::getType() const noexcept { return type; }

  const std::string& Object::getClassName() const noexcept { return className; }

  const std::string& Object::getName(void) const noexcept { return name; }

  void Object::setName(const std::string& name) { this->name = name; }

  std::ostream& operator<<(std::ostream& os, const Object& object)
  {
    os << "Object(" << object.getClassName();
    if (object.getName().empty() == false) {
      os << ": " << object.getName();
    }
    os << ")";
    return os;
  }

} // Soleil
