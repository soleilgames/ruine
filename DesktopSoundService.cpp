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

#include "DesktopSoundService.hpp"
#include "Logger.hpp"

namespace Soleil {

  /**
   * I've not found a good 3D Sound library on Desktop that fullfill my needs:
   * Cross Platform, Free (Freedom&Price), Easy. And as my target is currently
   * only on Android, I provide an empty implementation on Desktop. My dream
   * would be to implement a quick OpenSL Implementation.
   **/

  DesktopSoundService::DesktopSoundService() {}

  DesktopSoundService::~DesktopSoundService() {}

  void DesktopSoundService::playMusic(const std::string& trackName)
  {
    SOLEIL__LOGGER_DEBUG("Beep beep playing music: ", trackName);
  }

  void DesktopSoundService::stopMusic(void)
  {
    SOLEIL__LOGGER_DEBUG("Stopping music");
  }

  void DesktopSoundService::fireSound(const std::string& sound)
  {
    SOLEIL__LOGGER_DEBUG("Beep beep playing sound: ", sound);
  }

} // Soleil
