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

#ifndef SOLEIL__SOUNDSERVICE_HPP_
#define SOLEIL__SOUNDSERVICE_HPP_

#include <string>

namespace Soleil {

  typedef int8_t Percent;
  struct SoundProperties
  {
    Percent volume; // TODO: Enforce percentage using litterals

    SoundProperties(Percent volume)
      : volume(volume)
    {
    }
  };

  class SoundService
  {
  public:
    virtual void playMusic(const std::string& trackName) = 0;
    virtual bool pauseMusic(void)                        = 0;
    virtual bool resumeMusic(void)                       = 0;

    virtual void fireSound(const std::string&     sound,
                           const SoundProperties& properties) = 0;
  };

} // Soleil

#endif /* SOLEIL__SOUNDSERVICE_HPP_ */
