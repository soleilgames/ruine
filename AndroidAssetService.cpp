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

#include "AndroidAssetService.hpp"
#include "Logger.hpp"
#include "stringutils.hpp"

namespace Soleil {

  namespace {
    class AssetFile
    {
    public:
      AssetFile(AAssetManager* assetManager, const std::string& assetName)
        : assetName(assetName)
        , asset(AAssetManager_open(assetManager, assetName.c_str(),
                                   AASSET_MODE_BUFFER))
      {
        if (asset == nullptr) {
          throw std::runtime_error(
            toString("Asset not found: '", assetName, "'"));
        }
      }

      virtual ~AssetFile() { AAsset_close(asset); }

      const void* getBuffer(void) const
      {
        const void* buffer = AAsset_getBuffer(asset);

        if (buffer == nullptr) {
          throw std::runtime_error(
            toString("Cannot read asset: '", assetName, "'"));
        }
        return buffer;
      }

      const off_t getLength(void) const noexcept
      {
        return AAsset_getLength(asset);
      }

    private:
      const std::string assetName;
      AAsset*           asset;
    };

    class AssetGuard
    {
    public:
      AssetGuard(AAssetManager* assetManager, const std::string& assetName)
        : asset(AAssetManager_open(assetManager, assetName.c_str(),
                                   AASSET_MODE_UNKNOWN))
      {
        if (asset == nullptr) {
          throw std::runtime_error(
            Soleil::toString("Cannot locate asset ", assetName));
        }
      }

      virtual ~AssetGuard()
      {
        if (asset != nullptr) {
          AAsset_close(asset);
          asset = nullptr;
        }
      }

    public:
      AAsset* asset;
    };

  } //

  AndroidAssetService::AndroidAssetService(AAssetManager* assetManager)
    : assetManager(assetManager)
  {
  }

  AndroidAssetService::~AndroidAssetService() {}

  std::string AndroidAssetService::asString(const std::string& assetName)
  {
    AssetFile asset(assetManager, assetName);

    std::string fileAsString(static_cast<const char*>(asset.getBuffer()),
                             asset.getLength());

    SOLEIL__LOGGER_DEBUG("From asset: ", fileAsString);

    return fileAsString;
  }

  AssetDescriptorPtr AndroidAssetService::asDescriptor(
    const std::string& assetName)
  {
    AssetGuard asset(assetManager, assetName);

    off_t start;
    off_t length;
    int   fd = AAsset_openFileDescriptor(asset.asset, &start, &length);

    if (fd < 0) {
      throw std::runtime_error(
        toString("Cannot get fd for resource: ", assetName));
    }

    return std::make_shared<AssetDescriptor>(fd, start, length);
  }

  std::vector<uint8_t> AndroidAssetService::asDataVector(
    const std::string& assetName)
  {
    AssetGuard asset(assetManager, assetName);

    const off_t    length = AAsset_getLength(asset.asset);
    const uint8_t* buffer =
      static_cast<const uint8_t*>(AAsset_getBuffer(asset.asset));

    return std::vector<uint8_t>(buffer, buffer + length);
  }

} // Soleil
