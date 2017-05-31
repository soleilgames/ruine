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

#include "DesktopAssetService.hpp"
#include "stringutils.hpp"

#include <fstream>
#include <iostream>
#include <cassert>

namespace Soleil {

  DesktopAssetService::DesktopAssetService(const std::string& path)
    : path(path)
  {
  }

  DesktopAssetService::~DesktopAssetService() {}

  std::string DesktopAssetService::asString(const std::string& assetName)
  {
    const std::string fileName = path + assetName;
    std::ifstream     in(fileName);

    if (in.is_open() == false)
      throw std::runtime_error(
        toString("Failed to read file '", fileName, "'"));
    return std::string((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
  }

  AssetDescriptorPtr DesktopAssetService::asDescriptor(
    const std::string& /*assetName*/)
  {
    throw std::runtime_error(
      "I'm not using that right now"); // TODO: To implement on the need
  }

  std::vector<uint8_t> DesktopAssetService::asDataVector(const std::string& /*assetName*/)
  {
    throw std::runtime_error(
      "I'm not using that right now"); // TODO: To implement on the need
  }


  std::shared_ptr<AssetService> AssetService::Instance;
  
  std::string AssetService::LoadAsString(const std::string &assetName)
  {
    assert(Instance != nullptr && "Instance has to be set once at the program start-up");
    
    return Instance->asString(assetName);
  }

} // Soleil
