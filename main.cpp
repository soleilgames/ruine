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

#include <SDL.h>
#include <chrono>
#include <memory>
#include <stdexcept>

#include "DesktopAssetService.hpp"
#include "DesktopSoundService.hpp"
#include "OpenGLInclude.hpp"
#include "Ruine.hpp"
#include "stringutils.hpp"

using namespace Soleil;

static void
render(SDL_Window* window, Ruine& r)
{
  for (int i = 0; i < 500; i++) {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
      switch (e.type) {
        case SDL_QUIT: return;
        case SDL_KEYUP:
          switch (e.key.keysym.sym) {
            case SDLK_ESCAPE: return;
          }
      }
    }

    auto time = SDL_GetTicks();

    r.render(std::chrono::milliseconds(time));
    SDL_GL_SwapWindow(window);

    if (60 > (SDL_GetTicks() - time)) {
      SDL_Delay(60 - (SDL_GetTicks() - time));
    }
  }
}

int
main(int /*argc*/
     ,
     char* /*argv*/
     [])
{
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window*   window;
  SDL_GLContext glContext;

  window = SDL_CreateWindow(
    "Ruine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1920, 1080,
    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
  if (window == nullptr) {
    throw std::runtime_error(
      toString("SDL Window initialization failed: ", SDL_GetError()));
  }

  glContext = SDL_GL_CreateContext(window);
  if (glContext == nullptr) {
    throw std::runtime_error(
      toString("SDL GLContext initialization failed: ", SDL_GetError()));
  }

  SDL_GL_SetSwapInterval(1);
  glewExperimental = GL_TRUE;
  GLenum err       = glewInit();
  if (err != GLEW_OK) {
    throw std::runtime_error(
      toString("Unable to initialize GLEW: ", glewGetErrorString(err)));
  }

  std::unique_ptr<AssetService> assetService =
    std::make_unique<DesktopAssetService>("media/");
  std::unique_ptr<SoundService> soundService =
    std::make_unique<DesktopSoundService>();
  Soleil::Ruine r(assetService.get(), soundService.get());
  render(window, r);

  SDL_DestroyWindow(window);
  return 0;
}
