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
#ifndef _SOLEIL__MCUT_HPP_
#define _SOLEIL__MCUT_HPP_

#include <cassert>
#include <csignal>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace mcut {

  constexpr char static redColor[]     = "\x1b[31m";
  constexpr char static greenColor[]   = "\x1b[32m";
  constexpr char static whiteColor[]   = "\x1b[37m";
  constexpr char static defaultColor[] = "\x1b[0m";

#define MCUT__ASSERT(condition, message)                                       \
  if (condition) return;                                                       \
  std::cerr << redColor << message << defaultColor << "\n";                    \
  std::raise(SIGINT);

  template <typename... T> std::string toString(T&&... t)
  {
    std::stringstream ss;

    (void)std::initializer_list<bool>{(ss << t, true)...};

    return ss.str();
  }

  template <typename T, typename U> void assertEquals(T expected, U actual)
  {
    // if (expected == actual) return;
    // throw std::runtime_error(
    //   toString("Expected: ", expected, ". Got: ", actual, "."));
    MCUT__ASSERT(expected == actual,
                 toString("Expected: ", expected, ". Got: ", actual, "."));
  }

  template <typename T> void assertTrue(T actual)
  {
    if (actual) return;
    throw std::runtime_error(toString("Expected: ", actual, " to be true."));
  }

  template <typename T> void assertFalse(T actual)
  {
    if (actual == false) return;
    throw std::runtime_error(toString("Expected: ", actual, " to be false."));
  }

  typedef std::function<void(void)> TestMethod;

  class WasRun
  {
  public:
    WasRun(TestMethod testMethod)
      : wasRun(false)
      , testMethod(testMethod)
    {
    }
    virtual ~WasRun() {}

    void run()
    {
      testMethod();
      wasRun = true;
    }

    bool       wasRun;
    TestMethod testMethod;
  };

  class TestSuite
  {
  public:
    TestSuite(const std::string& name)
      : name(name)
    {
    }
    virtual ~TestSuite() {}

    void add(TestMethod testMethod) { testMethods.push_back(testMethod); }

    int run(void)
    {
      int failed = 0;

      for (WasRun& test : testMethods) {
        try {
          test.run();
          std::cout << whiteColor << "." << defaultColor << "\n";
        } catch (const std::runtime_error& e) {
          std::cout << redColor << e.what() << defaultColor << "\n";
          failed++;
        }
      }

      const char* color = (failed > 0) ? redColor : greenColor;
      std::cout << "Suite " << name << ": " << color
                << testMethods.size() - failed << defaultColor << " of "
                << testMethods.size() << " tests passed (" << color << failed
                << defaultColor << " failed)"
                << "\n";

      return failed;
    }

  private:
    std::string         name;
    std::vector<WasRun> testMethods;
  };

} // mcut

#endif /* _SOLEIL__MCUT_HPP_ */
