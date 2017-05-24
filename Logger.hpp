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

#ifndef SOLEIL__LOGGER_HPP_
#define SOLEIL__LOGGER_HPP_

#include <cassert>
#include <string>

namespace Soleil {

  class Logger
  {
  public:
    Logger();
    virtual ~Logger();

  public:
    enum Level
    {
      Debug,
      Info,
      Warning,
      Error
    };
    void log(const Level level, const std::string& message) noexcept;

  public:
    static void debug(const std::string& message) noexcept;
    static void info(const std::string& message) noexcept;
    static void warning(const std::string& message) noexcept;
    static void error(const std::string& message) noexcept;
  };
} // Soleil

#ifdef NDEBUG
#define SOLEIL__LOGGER_DEBUG(...) ((void)0)
#else
#include "stringutils.hpp"
#define SOLEIL__LOGGER_DEBUG(...)                                              \
  ::Soleil::Logger::debug(::Soleil::toString(__VA_ARGS__))
#endif // NDEBUG

#endif /* SOLEIL__LOGGER_HPP_ */
