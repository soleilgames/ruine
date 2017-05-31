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

#include "OpenGLInclude.hpp"

namespace Soleil {
  namespace gl {

    void GlGenFramebuffers(GLsizei i, GLuint* names)
    {
      glGenFramebuffers(i, names);
    }

    void GlDeleteFramebuffers(GLsizei i, const GLuint* names)
    {
      glDeleteFramebuffers(i, names);
    }

    void GlGenRenderbuffers(GLsizei i, GLuint* names)
    {
      glGenRenderbuffers(i, names);
    }

    void GlDeleteRenderbuffers(GLsizei i, const GLuint* names)
    {
      glDeleteRenderbuffers(i, names);
    }

    void GlGenTextures(GLsizei i, GLuint* names) { glGenTextures(i, names); }

    void GlDeleteTextures(GLsizei i, const GLuint* names)
    {
      glDeleteTextures(i, names);
    }

    void GlBindFramebuffer(GLenum target, GLuint name)
    {
      glBindFramebuffer(target, name);
    }

    void GlBindRenderbuffer(GLenum target, GLuint name)
    {
      glBindRenderbuffer(target, name);
    }

    void GlBindTexture(GLenum target, GLuint name)
    {
      glBindTexture(target, name);
    }

    void GlBindBuffer(GLenum target, GLuint name)
    {
      glBindBuffer(target, name);
    }

    void GlGenBuffers(GLsizei i, GLuint* names) { glGenBuffers(i, names); }

    void GlDeleteBuffers(GLsizei i, const GLuint* names)
    {
      glDeleteBuffers(i, names);
    }

  } // gl
} // Soleil
