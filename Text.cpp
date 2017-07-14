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

#include "Text.hpp"

#include "AssetService.hpp"
#include "OpenGLDataInstance.hpp"

#define STB_RECT_PACK_IMPLEMENTATION
#define STBRP_LARGE_RECTS
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace Soleil {
  namespace Text {

    GlyphSlot::GlyphSlot(const glm::vec2& uvOffset, const glm::vec2& uvSize,
                         const glm::vec2& pointMin, const glm::vec2& pointMax,
                         const float advance)
      : uvOffset(uvOffset)
      , uvSize(uvSize)
      , pointMin(pointMin)
      , pointMax(pointMax)
      , advance(advance)
    {
    }

    GlyphSlot::~GlyphSlot() {}

    class CharBitmap
    {
    public:
      CharBitmap(const stbtt_fontinfo* font, float scaleX, float scaleY,
                 int codePoint)
        : scaleX(scaleX)
        , scaleY(scaleY)
        , codePoint(codePoint)
      {
        buffer = stbtt_GetCodepointBitmap(font, scaleX, scaleY, codePoint,
                                          &width, &height, &xoffset, &yoffset);

        int advanceWidth;
        int leftSideBearing;

        int x0, x1, y0, y1;
        int lsb, iadvance;

        stbtt_GetCodepointHMetrics(font, codePoint, &advanceWidth,
                                   &leftSideBearing);
        stbtt_GetCodepointBox(font, codePoint, &x0, &y0, &x1, &y1);
        stbtt_GetCodepointHMetrics(font, codePoint, &iadvance, &lsb);
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        SOLEIL__LOGGER_DEBUG(
          toString("Viewport: ", viewport[2], ", ", viewport[3]));

        // const glm::vec2& viewport = OpenGLDataInstance::Instance().viewport;
        const float sx = 2.0f / viewport[2];
        const float sy = 2.0f / viewport[3];
        pointMin.x     = (float)x0 * scaleY * sx;
        pointMin.y     = (float)y0 * scaleY * sy;
        pointMax.x     = (float)x1 * scaleY * sx - pointMin.x;
        pointMax.y     = (float)y1 * scaleY * sy - pointMin.y;
        advance        = (float)iadvance * scaleY * sx;

        SOLEIL__LOGGER_DEBUG(
          "Metrics for `", WstringToString(std::wstring({(wchar_t)codePoint})),
          "`: Scale=", scaleY, ". advanceWidth=", advanceWidth,
          ", leftSideBearing=", leftSideBearing, ". Box=[", x0, ",", y0,
          "]\t- [", x1, ",", y1, "],\tPoint: ", pointMin, ", ", pointMax);
      }

      virtual ~CharBitmap()
      {
        if (buffer) stbtt_FreeBitmap(buffer, nullptr);
      }

#if 1
      CharBitmap(const CharBitmap&) = delete;
      CharBitmap& operator=(const CharBitmap&) = delete;
      CharBitmap(CharBitmap&& other)
        : scaleX(other.scaleX)
        , scaleY(other.scaleY)
        , codePoint(other.codePoint)
        , buffer(other.buffer)
      {
        other.buffer = nullptr;
      }
#else
      CharBitmap()
        : scaleX(0)
        , scaleY(0)
        , codePoint(0)
        , buffer(nullptr)
      {
      }
#endif

    public:
      float scaleX;
      float scaleY;
      int   codePoint;

    public:
      unsigned char* buffer;
      int            width;
      int            height;
      int            xoffset;
      int            yoffset;

    public:
      glm::vec2 pointMin;
      glm::vec2 pointMax;
      float     advance;
    };

    FontAtlas InitializeAtlasMap(const std::wstring& charMap,
                                 const std::string& assetFont, GLuint texture)
    {
      std::vector<uint8_t> ttf = AssetService::LoadAsDataVector(assetFont);
      const unsigned char* ttfBuffer = ttf.data();

      stbtt_fontinfo font;
      int            resultInitFont = stbtt_InitFont(
        &font, ttfBuffer, stbtt_GetFontOffsetForIndex(ttfBuffer, 0));

      // TODO: Throw exception
      assert(resultInitFont && "Failed to init the font");

      int                     fontSize      = 64;   // TODO: in parameter
      int                     textureWidth  = 1024; // TODO: in parameter
      int                     textureHeight = 1024; // TODO: in parameter
      std::vector<CharBitmap> bitmaps;
      bitmaps.reserve(charMap.size());
      std::vector<stbrp_rect> rects;
      int                     id = 0;

      float scale = stbtt_ScaleForPixelHeight(&font, fontSize);
      int   ascent;
      int   descent;
      int   lineGap;
      stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
      float baseline = (int)(ascent * scale);

      for (wchar_t c : charMap) {
        bitmaps.emplace_back(&font, 0, scale, c);
        const CharBitmap& bmp = bitmaps.back();
        rects.push_back({id, bmp.width, bmp.height, 0, 0, 0});
        id++;
      }

      stbrp_context           context;
      std::vector<stbrp_node> nodes(textureWidth * textureHeight);
      stbrp_init_target(&context, textureWidth, textureHeight, nodes.data(),
                        nodes.size());
      int allPacked = stbrp_pack_rects(&context, rects.data(), rects.size());

      // This is not clear yet if this function will be used by game designers
      // only to produce data that will be packed with the game, or to generate
      // asset at start-up that cannot fails or if it can fails due to the scale
      // size. For now just assert it worked.
      assert(allPacked && "All chars does not fit into the atlas texture");

      gl::BindTexture bindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, textureWidth, textureHeight, 0,
                   GL_ALPHA, GL_UNSIGNED_BYTE, nullptr);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glGenerateMipmap(GL_TEXTURE_2D);
      throwOnGlError();

      FontAtlas atlas;

      int viewport[4];
      glGetIntegerv(GL_VIEWPORT, viewport);
      const float sx        = 2.0f / viewport[2];
      const float sy        = 2.0f / viewport[3];
      atlas.verticalAdvance = (ascent - descent + lineGap) * scale * sy;
      SOLEIL__LOGGER_DEBUG(
        toString("Vertical Advance ===>", atlas.verticalAdvance));

      for (const auto& rect : rects) {
#if 0
      for (int j = 0; j < bmp.height; ++j) {
        for (int i = 0; i < bmp.width; ++i)
          putchar(" .:ovum@"[bmp.buffer[j * bmp.width + i] >> 5]);
        putchar('\n');
      }
#endif

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.w, rect.h,
                        GL_ALPHA, GL_UNSIGNED_BYTE, bitmaps[rect.id].buffer);

        glm::vec2 uvOffset((float)rect.x / (float)textureWidth,
                           (float)rect.y / (float)textureHeight);
        glm::vec2 uvSize((float)rect.w / (float)textureWidth,
                         (float)rect.h / (float)textureHeight);

        const auto& g = bitmaps[rect.id];
        atlas.glyphs.emplace(
          g.codePoint,
          GlyphSlot(uvOffset, uvSize, g.pointMin, g.pointMax, g.advance));
      }
      throwOnGlError();

      return atlas;
    }

    void FillBuffer(const std::wstring& text, TextCommand& textCommand,
                    const FontAtlas& atlas, float em, BoundingBox* bounds)
    {
      // 200 = 2.0 (OpenGL unit) * 100 (% of the em)
      const glm::vec2& viewport = OpenGLDataInstance::Instance().viewport;
      const float      sx       = em * 2.0f / viewport.x;
      const float      sy       = em * 2.0f / viewport.y;
      int              elemId   = 0;
      glm::vec2        offset(0.0f);
      std::vector<CharVertex> vertices;

      // em = em * 10.0f;

      textCommand.elements.clear();

      for (wchar_t c : text) {
        if (c == '\n') {
          offset.x = 0;
          offset.y -= atlas.verticalAdvance;
          continue;
        }

#ifndef NDEBUG
        if (atlas.glyphs.find(c) == atlas.glyphs.end())
          throw std::runtime_error(
            toString("cannot find char: ", WstringToString(std::wstring({c}))));
#endif
        const GlyphSlot& g        = atlas.glyphs.at(c);
        const glm::vec2  uvOffset = g.uvOffset;
        const glm::vec2  uvSize(uvOffset.x + g.uvSize.x,
                               uvOffset.y + g.uvSize.y);
        const glm::vec2 size(offset.x + g.pointMin.x, offset.y + g.pointMin.y);

        vertices.push_back(
          {glm::vec3(offset.x + g.pointMin.x, offset.y + g.pointMin.y, 0.0f) *
             em,
           glm::vec2(uvOffset.x, uvSize.y)});
        vertices.push_back(
          {glm::vec3(size.x + g.pointMax.x, offset.y + g.pointMin.y, 0.0f) * em,
           glm::vec2(uvSize.x, uvSize.y)});
        vertices.push_back(
          {glm::vec3(size.x + g.pointMax.x, size.y + g.pointMax.y, 0.0f) * em,
           glm::vec2(uvSize.x, uvOffset.y)});
        vertices.push_back(
          {glm::vec3(offset.x + g.pointMin.x, size.y + g.pointMax.y, 0.0f) * em,
           glm::vec2(uvOffset.x, uvOffset.y)});

        offset.x += g.advance;
	
        textCommand.elements.push_back(0 + elemId);
        textCommand.elements.push_back(1 + elemId);
        textCommand.elements.push_back(2 + elemId);
        textCommand.elements.push_back(2 + elemId);
        textCommand.elements.push_back(3 + elemId);
        textCommand.elements.push_back(0 + elemId);
        elemId += 4;
      }

      if (bounds) {
        for (const auto& v : vertices) {
          bounds->expandBy(v.position);
        }
      }

      glBindBuffer(GL_ARRAY_BUFFER, *textCommand.buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(),
                   vertices.data(), GL_DYNAMIC_DRAW);

      throwOnGlError();
    }
  } // Text
} // Soleil
