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

#ifndef SOLEIL__OPENGLDATAINSTANCE_HPP_
#define SOLEIL__OPENGLDATAINSTANCE_HPP_

#include "OpenGLInclude.hpp"
#include "Program.hpp"
#include "Shader.hpp"

#include <cassert>
#include <memory>

namespace Soleil {

  static constexpr int DefinedMaxLights = 10;

  struct DrawablePointLight
  {
    GLint position;
    GLint color;
    GLint linearAttenuation;
    GLint quadraticAttenuation;
  };

  struct DrawableMaterial
  {
    GLint ambiantColor;
    GLint shininess;
    GLint emissiveColor;
    GLint diffuseColor;
    GLint specularColor;
    GLint diffuseMap;
  };

  struct OpenGLDataInstance
  {
    // TODO: Create a struct for the Drawable Shader
    Program drawable;
    /* Uniforms */
    GLint drawableMVPMatrix;
    GLint drawableMVMatrix;
    GLint drawableNormalMatrix;
    GLint drawableAmbiantLight;
    GLint drawableEyeDirection;
    GLint drawableConstantAttenuation;
    GLint drawableNumberOfLights;

    DrawableMaterial   drawableMaterial;
    DrawablePointLight drawablePointLights[DefinedMaxLights];

    // 2D Image
    gl::Buffer imageBuffer;
    Program    imageProgram;
    GLint      imageModelMatrix;
    GLint      imageImage;

    // Textures
    gl::Texture              textureTest;
    gl::Texture              texturePad;
    std::vector<gl::Texture> textures; // TODO: Do not let the container extend.

    static OpenGLDataInstance& Instance(void) noexcept
    {
      assert(instance != nullptr &&
             "OpenGLDataInstance::Intialized not called yet");

      return *instance;
    }

    static void Initialize(void);

    OpenGLDataInstance()
      : textures(32)
    {
      //textures.reserve(32); // TODO: Fix this number
    };
    OpenGLDataInstance(const OpenGLDataInstance&) = delete;

  private:
    static std::unique_ptr<OpenGLDataInstance> instance;
  };

} // Soleil

#endif /* SOLEIL__OPENGLDATAINSTANCE_HPP_ */
