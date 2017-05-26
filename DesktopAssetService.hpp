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

#ifndef SOLEIL__DESKTOPASSETSERVICE_HPP_
#define SOLEIL__DESKTOPASSETSERVICE_HPP_

#include "AssetService.hpp"

namespace Soleil {

  class DesktopAssetService : public AssetService
  {
  public:
    DesktopAssetService(const std::string& path);
    virtual ~DesktopAssetService();

    AssetDescriptorPtr asDescriptor(const std::string& assetName) override;
    std::vector<uint8_t> asDataVector(const std::string& assetName) override;

  public:
    std::string asString(const std::string& assetName) override;

  private:
    std::string path;
  };

} // Soleil

#endif /* SOLEIL__DESKTOPASSETSERVICE_HPP_ */
