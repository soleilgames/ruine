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

#ifndef SOLEIL__TEXT_HPP_
#define SOLEIL__TEXT_HPP_

#include "Draw.hpp"
#include "OpenGLInclude.hpp"
#include "stb_truetype.h"

#include <map>
#include <string>

#include <glm/vec2.hpp>

namespace Soleil {
  namespace Text {

    struct GlyphSlot
    {
      glm::vec2 uvOffset;
      glm::vec2 uvSize;

      GlyphSlot(glm::vec2 uvOffset, glm::vec2 uvSize);
      ~GlyphSlot(void);
    };

    struct FontAtlas
    {
      std::map<wchar_t, GlyphSlot> glyphs;
    };

    FontAtlas InitializeAtlasMap(const std::wstring& charMap,
                                 const std::string& assetFont, GLuint texture);
    void FillBuffer(const std::wstring& text, TextCommand& textCommand,
                    const FontAtlas& atlas, glm::vec2 viewport);
  } // Text

} // Soleil

#endif /* SOLEIL__TEXT_HPP_ */
