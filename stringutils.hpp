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

#include <algorithm>
#include <codecvt>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

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

  inline std::string trim(const std::string& s)
  {
    std::string str = s;
    str.erase(str.begin(),
              std::find_if(str.begin(), str.end(),
                           std::not1(std::ptr_fun<int, int>(std::isspace))));
    return str;
  }

  inline std::wstring StringToWstring(const std::string& str)
  {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
  }

  inline std::string WstringToString(const std::wstring& wstr)
  {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
  }

  inline std::string RandomString(std::size_t length)
  {
    auto randchar = []() -> char {
      const char charset[] = "0123456789"
                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                             "abcdefghijklmnopqrstuvwxyz";
      const size_t max_index = (sizeof(charset) - 1);
      return charset[rand() % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
  }
}

#endif /* SOLEIL__STRINGUTILS_HPP_ */
