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

#include "OpenGLDataInstance.hpp"

namespace Soleil {

  std::unique_ptr<OpenGLDataInstance> OpenGLDataInstance::instance;

  static inline void initializeDrawable()
  {
    OpenGLDataInstance& instance = OpenGLDataInstance::Instance();
    Program&            drawable = instance.drawable;

    drawable.attachShader(Shader(GL_VERTEX_SHADER, "shape.vert"));
    drawable.attachShader(Shader(GL_FRAGMENT_SHADER, "shape.frag"));

    glBindAttribLocation(drawable.program, 0, "positionAttribute");
    glBindAttribLocation(drawable.program, 1, "normalAttribute");
    glBindAttribLocation(drawable.program, 2, "colorAttribute");

    drawable.compile();

    instance.drawableMVPMatrix      = drawable.getUniform("MVPMatrix");
    instance.drawableMVMatrix       = drawable.getUniform("MVMatrix");
    instance.drawableNormalMatrix   = drawable.getUniform("NormalMatrix");
    instance.drawableNumberOfLights = drawable.getUniform("numberOfLights");

    instance.drawableMaterial.ambiantColor =
      drawable.getUniform("material.ambiantColor");
    instance.drawableMaterial.shininess =
      drawable.getUniform("material.shininess");
    instance.drawableMaterial.emissiveColor =
      drawable.getUniform("material.emissiveColor");
    instance.drawableMaterial.diffuseColor =
      drawable.getUniform("material.diffuseColor");
    instance.drawableMaterial.specularColor =
      drawable.getUniform("material.specularColor");
    instance.drawableMaterial.diffuseMap =
      drawable.getUniform("material.diffuseMap");

    instance.drawableAmbiantLight = drawable.getUniform("AmbiantLight");
    instance.drawableEyeDirection = drawable.getUniform("EyeDirection");
    for (int i = 0; i < DefinedMaxLights; ++i) {
      instance.drawablePointLights[i].position =
        drawable.getUniform(toString("pointLight[", i, "].position").data());
      instance.drawablePointLights[i].color =
        drawable.getUniform(toString("pointLight[", i, "].color").data());
      instance.drawablePointLights[i].linearAttenuation = drawable.getUniform(
        toString("pointLight[", i, "].linearAttenuation").data());
      instance.drawablePointLights[i].quadraticAttenuation =
        drawable.getUniform(
          toString("pointLight[", i, "].quadraticAttenuation").data());
    }
  }

  static inline void initializeTestResources()
  {
    OpenGLDataInstance& instance = OpenGLDataInstance::Instance();

    const GLubyte chessBoard[] = {
      0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00,
      0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
      0xFF, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF,
      0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0xFF,
      0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
      0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF};

    {
      gl::BindTexture bindTexture(GL_TEXTURE_2D, *instance.textureTest);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 8, 8, 0, GL_ALPHA,
                   GL_UNSIGNED_BYTE, chessBoard);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glGenerateMipmap(GL_TEXTURE_2D);
    }
    throwOnGlError();
  }

  void OpenGLDataInstance::Initialize(void)
  {
    OpenGLDataInstance::instance = std::make_unique<OpenGLDataInstance>();
    initializeDrawable();
    initializeTestResources();
  }

} // Soleil
