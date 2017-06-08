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

    instance.drawableMVPMatrix    = drawable.getUniform("MVPMatrix");
    instance.drawableMVMatrix     = drawable.getUniform("MVMatrix");
    instance.drawableNormalMatrix = drawable.getUniform("NormalMatrix");
    instance.drawableMaterialAmbiant =
      drawable.getUniform("material.ambiantColor");
    instance.drawableMaterialShininess =
      drawable.getUniform("material.shininess");
    instance.drawableAmbiantLight = drawable.getUniform("AmbiantLight");
    instance.drawableEyeDirection = drawable.getUniform("EyeDirection");
    // instance.drawableConstantAttenuation =
    //   drawable.getUniform("ConstantAttenuation");
    instance.drawablePointLightPosition =
      drawable.getUniform("pointLight.position");
    instance.drawablePointLightColor = drawable.getUniform("pointLight.color");
    instance.drawablePointLightLinearAttenuation =
      drawable.getUniform("pointLight.linearAttenuation");
    instance.drawablePointLightQuadraticAttenuation =
      drawable.getUniform("pointLight.quadraticAttenuation");
  }

  void OpenGLDataInstance::Initialize(void)
  {
    OpenGLDataInstance::instance = std::make_unique<OpenGLDataInstance>();
    initializeDrawable();
  }

} // Soleil
