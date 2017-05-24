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

#include "Logger.hpp"

#if defined(__ANDROID__)
#include <android/log.h>
#else
#include <iostream>
#endif

namespace Soleil {
  static Logger GlobalLogger;

  Logger::Logger() {}

  Logger::~Logger() {}

  void Logger::log(const Soleil::Logger::Level level,
                   const std::string&          message) noexcept
  {
    try {
#if defined(__ANDROID__)
      static constexpr int levelLabels[] = {ANDROID_LOG_DEBUG, ANDROID_LOG_INFO,
                                            ANDROID_LOG_WARN,
                                            ANDROID_LOG_ERROR};
      __android_log_print(levelLabels[level], "com.soleilgames", "%s",
                          message.c_str());
#else
      static constexpr const char* levelLabels[] = {"DEBUG", "INFO", "WARNING",
                                                    "ERROR"};
      std::clog << "[" << levelLabels[level] << "] " << message << "\n";
#endif
    } catch (...) {
      // TODO: Raise sigabort ?
    }
  }

  void Logger::debug(const std::string& message) noexcept
  {
    GlobalLogger.log(Level::Debug, message);
  }

  void Logger::info(const std::string& message) noexcept
  {
    GlobalLogger.log(Level::Info, message);
  }

  void Logger::warning(const std::string& message) noexcept
  {
    GlobalLogger.log(Level::Warning, message);
  }

  void Logger::error(const std::string& message) noexcept
  {
    GlobalLogger.log(Level::Error, message);
  }

} // Soleil
