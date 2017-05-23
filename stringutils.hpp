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

#ifndef SOLEIL__STRINGUTILS_HPP_
#define SOLEIL__STRINGUTILS_HPP_

#include <iostream>
#include <sstream>
#include <string>

namespace Soleil {
  template <typename... T> std::string toCommaString(T&&... t)
  {
    std::stringstream ss;
    bool              noComma = true;
    (void)std::initializer_list<bool>{
      (ss << (noComma ? "" : ", ") << t, noComma = false)...};
    return ss.str();
  }

  template <typename... T> std::string toString(T&&... t)
  {
    std::stringstream ss;

    (void)std::initializer_list<bool>{(ss << t, true)...};

    return ss.str();
  }

  template <typename... T> std::wstring toWString(T&&... t)
  {
    std::wstringstream ss;

    (void)std::initializer_list<bool>{(ss << t, true)...};

    return ss.str();
  }
}

#endif /* SOLEIL__STRINGUTILS_HPP_ */
