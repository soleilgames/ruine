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

#include "SoundService.hpp"

#include <cassert>

namespace Soleil {

  std::shared_ptr<SoundService> SoundService::Instance;

  void SoundService::PlayMusic(const std::string& trackName)
  {
    assert(SoundService::Instance != nullptr &&
           "Instance has to be set once at the program start-up");

    SoundService::Instance->playMusic(trackName);
  }

  void SoundService::FireSound(const std::string&     sound,
                               const SoundProperties& properties)
  {
    assert(SoundService::Instance != nullptr &&
           "Instance has to be set once at the program start-up");

    SoundService::Instance->fireSound(sound, properties);
  }

  bool SoundService::PauseMusic(void)
  {
    assert(SoundService::Instance != nullptr &&
           "Instance has to be set once at the program start-up");

    return SoundService::Instance->pauseMusic();
  }
  bool SoundService::ResumeMusic(void)
  {
    assert(SoundService::Instance != nullptr &&
           "Instance has to be set once at the program start-up");

    return SoundService::Instance->resumeMusic();
  }

} // Soleil
