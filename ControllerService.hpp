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

#ifndef SOLEIL__CONTROLLERSERVICE_HPP_
#define SOLEIL__CONTROLLERSERVICE_HPP_

#include <memory>

#include <glm/vec3.hpp>

namespace Soleil {

  struct Controller
  {
    glm::vec3 dpad;

    bool option1;
    bool option2;

    Controller()
      : dpad()
      , option1(true)
      , option2(true)
    {
    }
  };

  class ControllerService
  {
  public:
    static Controller& GetPlayerController(void) noexcept;
    static glm::vec2&  GetPadPosition(void) noexcept;
  };

} // Soleil

#endif /* SOLEIL__CONTROLLERSERVICE_HPP_ */
