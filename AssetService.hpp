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

#ifndef SOLEIL__ASSETSERVICE_HPP_
#define SOLEIL__ASSETSERVICE_HPP_

#include <memory>
#include <string>
#include <vector>

#include "OpenGLInclude.hpp"

namespace Soleil {

  class AssetDescriptor
  {
  public:
    AssetDescriptor(int fd, off_t start, off_t length)
      : fd(fd)
      , start(start)
      , length(length)
    {
    }

    virtual ~AssetDescriptor() {}

  public:
    int   getFd(void) const { return fd; }
    off_t getStart(void) const { return start; }
    off_t getLength(void) const { return length; }

  private:
    int   fd;
    off_t start;
    off_t length;
  };

  typedef std::shared_ptr<AssetDescriptor> AssetDescriptorPtr;

  class ImageAsset
  {
  public:
    ImageAsset(const std::string& assetName);
    ImageAsset(const ImageAsset&) = delete;
    virtual ~ImageAsset();

  public:
    int            width(void) const noexcept;
    int            height(void) const noexcept;
    const uint8_t* data(void) const noexcept;

  private:
    uint8_t* image;
    int      imageWidth;
    int      imageHeight;
    int      channelsInFile;
  };

  class AssetService
  {
  public:
    virtual std::string asString(const std::string& assetName)              = 0;
    virtual AssetDescriptorPtr asDescriptor(const std::string& assetName)   = 0;
    virtual std::vector<uint8_t> asDataVector(const std::string& assetName) = 0;

    /**
     * I Was against the Singletons pattern for a long time, but after watched
     * different (and a lot of) C++ Con. I cannot tell anymore what is good or
     * not. I think that the most important is to try and to see with experience
     * what is the most valuable. Here I try to work on my GongFu code. Let see
     * if I will take a slap. Just the important thing is: Keep it simple and
     * stable.
     */
  public:
    static std::shared_ptr<AssetService> Instance;
    static std::string LoadAsString(const std::string& assetName);
    static std::vector<uint8_t> LoadAsDataVector(const std::string& assetName);
    static void LoadTextureHigh(GLuint texture, const std::string& assetName);
    static void LoadTextureLow(GLuint texture, const std::string& assetName);
  };

} // Soleil

#endif /* SOLEIL__ASSETSERVICE_HPP_ */
