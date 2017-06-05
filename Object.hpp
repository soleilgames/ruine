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

#ifndef SOLEIL__OBJECT_HPP_
#define SOLEIL__OBJECT_HPP_

#include <string>
#include <typeinfo>

namespace Soleil {

  typedef size_t HashType;

  /**
   * This is a base class to name and type every objects.
   *
   * Yep I coded in Java ;)
   */
  class Object
  {
  public:
    /**
     * @param type Represent the type of the class
     * @param className A human readable name for logging
     * @param name The name of the instance
     */
    Object(HashType type, const std::string& className,
           const std::string& name = "");
    virtual ~Object();

  public:
    /**
     * Represent the type of the class.
     *
     * It should be the same for all the instances. It's an helper to compare
     * quickly objects not a way to overwrite polymorphism.
     */
    HashType getType(void) const noexcept;

    /**
     * Represents the class Name of the object (human readable).
     *
     * It's mostly used for logging. It must never be empty and should be the
     * same for all the instances.
     */
    const std::string& getClassName(void) const noexcept;

    /**
     * Represents the name of the Instance.
     */
    const std::string& getName(void) const noexcept;

    /**
     * Set the name of the instance
     */
    void setName(const std::string& name);

  private:
    HashType    type;
    std::string className;
    std::string name;
  };

  std::ostream& operator<<(std::ostream& os, const Object& object);

} // Soleil

#endif /* SOLEIL__OBJECT_HPP_ */
