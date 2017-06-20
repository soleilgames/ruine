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

#include "AssetService.hpp"

#include "Logger.hpp"
#include "stringutils.hpp"

#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

namespace Soleil {

  ImageAsset::ImageAsset(const std::string& assetName)
  {
    stbi_set_flip_vertically_on_load(1);
    const auto encodedImageData = AssetService::LoadAsDataVector(assetName);
    image =
      stbi_load_from_memory(encodedImageData.data(), encodedImageData.size(),
                            &imageWidth, &imageHeight, &channelsInFile, 4);

    if (image == nullptr) {
      throw std::runtime_error(toString("Failed to decode image '", assetName,
                                        "': ", stbi_failure_reason()));
      stbi_image_free(image);
    }
    SOLEIL__LOGGER_DEBUG(
      toString("Loaded image: ", assetName, " -> ", imageWidth, "x",
               imageHeight, ". Original number of chanels: ", channelsInFile));
  }

  ImageAsset::~ImageAsset() { stbi_image_free(image); }

  int ImageAsset::width() const noexcept { return imageWidth; }

  int ImageAsset::height() const noexcept { return imageHeight; }

  const uint8_t* ImageAsset::data(void) const noexcept { return image; }

  std::shared_ptr<AssetService> AssetService::Instance;

  std::string AssetService::LoadAsString(const std::string& assetName)
  {
    assert(Instance != nullptr &&
           "Instance has to be set once at the program start-up");

    return Instance->asString(assetName);
  }

  std::vector<uint8_t> AssetService::LoadAsDataVector(
    const std::string& assetName)
  {
    return Instance->asDataVector(assetName);
  }

} // Soleil
