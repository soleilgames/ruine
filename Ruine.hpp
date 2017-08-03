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

#ifndef SOLEIL__RUINE_HPP_
#define SOLEIL__RUINE_HPP_

#include "AssetService.hpp"
#include "Draw.hpp"
#include "OpenGLInclude.hpp"
#include "SoundService.hpp"
#include "World.hpp"
#include "types.hpp"

#include "OpenGLDataInstance.hpp"
#include "Text.hpp"

#include <glm/vec3.hpp>

#include <queue>

#ifndef NDEBUG
#define SOLEIL__CONSOLE_DRAW() console.draw()
#define SOLEIL__CONSOLE_TEXT(str) console.text = str
#define SOLEIL__CONSOLE_APPEND(str) console.text += str
#else
#define SOLEIL__CONSOLE_DRAW (void)
#define SOLEIL__CONSOLE_TEXT(str) (void)str
#define SOLEIL__CONSOLE_APPEND(str) (void)str
#endif

namespace Soleil {

  struct Menu
  {
    gl::Texture door;
    TextCommand title;
    TextCommand newGame;

    glm::mat4 doorTransformation;
    glm::mat4 titleTransformation;
    glm::mat4 newGameTransformation;


    Menu() {}
  };

  class FadeTimer
  {
  public:
    FadeTimer(const Timer& start, const Timer& length)
      : start(start.count())
      , length(length.count())
    {
    }

    FadeTimer()
      : start(0)
      , length(1)
    {
    }

    float ratio(const Timer& time) const noexcept
    {
      return glm::clamp((time.count() - start) / length, 0.0f, 1.0f);
    }

  private:
    float start;
    float length;
  };

#ifndef NDEBUG
  struct Console
  {
    glm::mat4    transformation;
    std::wstring text;

    Console()
      : dirty(true)
    {
    }

    void draw()
    {
      if (text.size() > 0) {
        if (dirty) {
          Text::FillBuffer(text, display,
                           OpenGLDataInstance::Instance().textAtlas, 0.5f);
        }
        DrawText(display, transformation, Color(0.8f));
      }
    }

  private:
    TextCommand display;
    bool        dirty;
  };
#endif

  class Ruine
  {
  public:
    Ruine(AssetService* assetService, SoundService* soundService,
          int viewportWidth, int viewportHeight);
    virtual ~Ruine();

  public:
    void render(Timer time);

  private:
    void initializeGame(const Timer& time);
    void updateTriggers(World& world, Frame& frame);
    void renderMenu(const Timer& time);
    void renderGame(const Timer& time);
    void renderDialogue(const Timer& time);
    void renderCredits(const Timer& time);

  private:
    AssetService* assetService;
    SoundService* soundService;
    int           viewportWidth;
    int           viewportHeight;
    Camera        camera;
    glm::mat4     view;

    Timer timeToReset;
    Frame frame;

    World       world;
    PopUp       caption;
    TextCommand goldLabel;
    glm::mat4   goldLabelTransformation;
    int         goldScore;

    Timer nextGhostSound;

    std::queue<std::wstring> dialogueQueue;
    
  private:
    enum State
    {
      StateMenu         = 0x01,
      StateFadingIn     = 0x02,
      StateFadingOut    = 0x04,
      StateGame         = 0x08,
      StateInitializing = 0x10,
      StateDialogue     = 0x20,
      StateCredits      = 0x40
    };
    int       state;
    FadeTimer fading;

    TextCommand dialogueLabel;
    glm::mat4   dialogueTransformation;
    glm::mat4   dialogueBackgroundTransformation;
    int         sentence;
    bool        dirty;

    TextCommand credits;
    glm::mat4   creditsTransformation;

#ifndef NDEBUG
    Console console;
#endif
  };

} // Soleil

#endif /* SOLEIL__RUINE_HPP_ */
