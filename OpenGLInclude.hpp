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

#ifndef SOLEIL__OPENGLINCLUDE_HPP_
#define SOLEIL__OPENGLINCLUDE_HPP_

#if defined(SOLEIL_FORCE_ES) || defined(__ANDROID__)

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>

#else

#include <GL/glew.h>

#include <GL/gl.h>
#include <GL/glext.h>

#include <glm/glm.hpp>

#endif

#include "Logger.hpp"
#include "stringutils.hpp"

#define throwOnGlError() ::Soleil::_checkGLError(true, __FILE__, __LINE__)
#define warnOnGlError() ::Soleil::_checkGLError(false, __FILE__, __LINE__)

namespace Soleil {

  void LogInfo(const std::string& message);
  void LogWarning(const std::string& message);
  void LogError(const std::string& message);

  inline void _checkGLError(const bool throwOnError, const char* file, int line)
  {
    GLenum err     = GL_NO_ERROR;
    bool   errored = false;

    while ((err = glGetError()) != GL_NO_ERROR) {
      std::string error;

      switch (err) {
        case GL_INVALID_OPERATION: error = "INVALID_OPERATION"; break;
        case GL_INVALID_ENUM: error      = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE: error     = "INVALID_VALUE"; break;
        case GL_OUT_OF_MEMORY: error     = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
          error = "INVALID_FRAMEBUFFER_OPERATION";
          break;
        default:
          error = "Unknown"; // TODO chek all errors
      }
      // LogWarning(
      //   toString("GL Error, ", error, "(", err, "): ", file, ":", line,
      //   "\n"));
      std::cout << toString("GL Error, ", error, "(", err, "): ", file, ":",
                            line, "\n")
                << "\n";

      errored = true;
    }

    if (errored && throwOnError)
      throw std::runtime_error("GL Error occured (see logs).");
  }

  // Below are RAII Wrapper for OpenGL -----------------------------------------

  namespace gl {

    void GlGenFramebuffers(GLsizei i, GLuint* names);
    void GlGenRenderbuffers(GLsizei i, GLuint* names);
    void GlGenTextures(GLsizei i, GLuint* names);
    void GlGenBuffers(GLsizei i, GLuint* names);

    void GlDeleteFramebuffers(GLsizei i, const GLuint* names);
    void GlDeleteRenderbuffers(GLsizei i, const GLuint* names);
    void GlDeleteTextures(GLsizei i, const GLuint* names);
    void GlDeleteBuffers(GLsizei i, const GLuint* names);

    void GlBindFramebuffer(GLenum target, GLuint name);
    void GlBindRenderbuffer(GLenum target, GLuint name);
    void GlBindTexture(GLenum target, GLuint name);
    void GlBindBuffer(GLenum target, GLuint name);

    template <void GenFunction(GLsizei, GLuint*),
              void DeleteFunction(GLsizei, const GLuint*)>
    class Generator
    {
    public:
      Generator()
      {
        GenFunction(1, &name);
        throwOnGlError();
      }

      virtual ~Generator()
      {
        SOLEIL__LOGGER_DEBUG(toString("~GL Resource destructed"));

        DeleteFunction(1, &name);
      }

      GLuint operator*(void) const { return name; }

      Generator(const Generator&) = delete;
      Generator& operator=(const Generator&) = delete;

      Generator(const Generator&& other)
        : name(other.name)
      {
      }

    private:
      GLuint name;
    };

    template <void BindFunction(GLenum, GLuint), GLuint defValue>
    class BindGuard
    {
    public:
      BindGuard(GLenum target, GLuint name)
        : target(target)
      {
        BindFunction(target, name);
      }

      virtual ~BindGuard() { BindFunction(target, defValue); }

    private:
      GLenum target;
    };

    typedef Generator<GlGenTextures, GlDeleteTextures>           Texture;
    typedef Generator<GlGenFramebuffers, GlDeleteFramebuffers>   rameBuffer;
    typedef Generator<GlGenRenderbuffers, GlDeleteRenderbuffers> RenderBuffer;
    typedef Generator<GlGenBuffers, GlDeleteBuffers>             Buffer;
    typedef BindGuard<GlBindFramebuffer, 0>  BindFrameBuffer;
    typedef BindGuard<GlBindRenderbuffer, 0> BindRenderBuffer;
    typedef BindGuard<GlBindTexture, 0>      BindTexture;
    typedef BindGuard<GlBindBuffer, 0>       BindBuffer;

  } // GL

} // Soleil

#endif /* SOLEIL__OPENGLINCLUDE_HPP_ */
