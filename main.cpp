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

#include <chrono>
#include <memory>
#include <stdexcept>
#include <unistd.h>

#include "ControllerService.hpp"
#include "DesktopAssetService.hpp"
#include "DesktopSoundService.hpp"
#include "OpenGLInclude.hpp"
#include "Recorder.hpp"
#include "Ruine.hpp"
#include "stringutils.hpp"

using namespace Soleil;

class DesktopControllerService
{
public:
  DesktopControllerService() { player.push.triggerTime = 0.0f; }
  virtual ~DesktopControllerService() {}

public:
  Controller player;
  glm::vec2  padPosition;
};

DesktopControllerService controllerService;

Controller&
ControllerService::GetPlayerController() noexcept
{
  return controllerService.player;
}

glm::vec2&
ControllerService::GetPadPosition(void) noexcept
{
  return controllerService.padPosition;
}

Push&
ControllerService::GetPush(void) noexcept
{
  return controllerService.player.push;
}

static void
render(GLFWwindow* window, Ruine& r)
{
  const Recorder::Record* record           = Recorder::records.data();
  std::size_t             currentRecordRow = 0;

  while (!glfwWindowShouldClose(window)) {
    std::chrono::milliseconds time((int)(glfwGetTime() * 1000));

    if (Recorder::state == Recorder::DoReplay) {
      time = std::chrono::milliseconds(record->time);

      Push& push    = ControllerService::GetPush();
      push.active   = record->pushState;
      push.start    = glm::vec2(record->startx, record->starty);
      push.position = glm::vec2(record->positionx, record->positiony);
    }

    r.render(time);

    if (Recorder::state == Recorder::DoReplay) {
      // SOLEIL__LOGGER_DEBUG(
      //   "CUR:", Recorder::currentRecord.frameHash, "==REC:",
      //   record->frameHash,
      //   " - ", Recorder::currentRecord.frameNumber, "==",
      //   record->frameNumber,
      //   " - ", Recorder::currentRecord.debug, "->", record->debug);
      assert(Recorder::currentRecord.frameHash == record->frameHash);
      if (Recorder::currentRecord.frameHash != record->frameHash)
        throw std::runtime_error(
          toString("CUR:", Recorder::currentRecord.frameHash,
                   "==REC:", record->frameHash));

      ++record;
      ++currentRecordRow;
      if (currentRecordRow >= Recorder::records.size())
        glfwSetWindowShouldClose(window, 1);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

static void
errorCallback(int error, const char* description)
{
  std::cerr << "GLFW failed with error N." << error << ": " << description;
}

static void
keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action,
            int /*mods*/)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
    return;
  }

  switch (action) {
    case GLFW_PRESS:
      switch (key) {
        case GLFW_KEY_W: controllerService.player.dpad.z = 1.0f; break;
        case GLFW_KEY_S: controllerService.player.dpad.z = -1.0f; break;
        case GLFW_KEY_A: controllerService.player.dpad.x = 1.0f; break;
        case GLFW_KEY_D: controllerService.player.dpad.x = -1.0f; break;
      }
      break;
    case GLFW_RELEASE:
      switch (key) {
        case GLFW_KEY_W: controllerService.player.dpad.z = 0.0f; break;
        case GLFW_KEY_S: controllerService.player.dpad.z = 0.0f; break;
        case GLFW_KEY_A: controllerService.player.dpad.x = 0.0f; break;
        case GLFW_KEY_D: controllerService.player.dpad.x = 0.0f; break;
        case GLFW_KEY_1:
          controllerService.player.option1 = !controllerService.player.option1;
          break;
        case GLFW_KEY_2:
          controllerService.player.option2 = !controllerService.player.option2;
          break;
        case GLFW_KEY_3:
          controllerService.player.option3 = !controllerService.player.option3;
          break;
        case GLFW_KEY_4:
          controllerService.player.option4 = !controllerService.player.option4;
          break;
      }
      break;
  }
}

static void
cursor_positionCallback(GLFWwindow* window, double xpos, double ypos)
{
  int width, height;
  glfwGetFramebufferSize(window, &width, &height); // TODO: Can use a static
                                                   // viewport instead of
                                                   // calling this each frames

  Push& push    = controllerService.player.push;
  push.position = (glm::vec2(xpos / (float)width, -ypos / (float)height) +
                   glm::vec2(-0.5f, 0.5f)) *
                  2.0f;

  if (push.active == PushState::Down) {
    push.active = PushState::Active;
    push.start  = controllerService.player.push.position;
  }
}

void
mouse_buttonCallback(GLFWwindow* /*window*/, int button, int action,
                     int /*mods*/)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    Push& push = controllerService.player.push;

    push.active =
      ((action == GLFW_RELEASE) ? PushState::Release : PushState::Down) |
      PushState::Fresh;

    if (action == GLFW_RELEASE) {
      const double now  = glfwGetTime() * 1000.0f;
      const double diff = now - push.triggerTime;
      push.triggerTime  = now;
      if (diff > 100.0f && diff < 600.0f) {
        push.active      = PushState::DoubleClick | PushState::Fresh;
        push.triggerTime = 0; // Avoid triple click
      }
    }
  }
}

int
main(int argc, char* argv[])
{
  GLFWwindow* window;
  int         width  = 1920;
  int         height = 1080;

  glfwSetErrorCallback(errorCallback);
  if (!glfwInit()) return -1;

  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  window =
    glfwCreateWindow(width, height, "Ruine", glfwGetPrimaryMonitor(), nullptr);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  glfwSetKeyCallback(window, keyCallback);
  glfwSetCursorPosCallback(window, cursor_positionCallback);
  glfwSetMouseButtonCallback(window, mouse_buttonCallback);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glewExperimental = GL_TRUE;
  GLenum err       = glewInit();
  if (err != GLEW_OK) {
    throw std::runtime_error(
      toString("Unable to initialize GLEW: ", glewGetErrorString(err)));
  }

  AssetService::Instance = std::make_shared<DesktopAssetService>("media/");
  SoundService::Instance = std::make_unique<DesktopSoundService>();

  Recorder::state         = Recorder::DoNothing;
  Recorder::currentRecord = {0, 0, 0, 0, 0, 0, 0}; // TODO: constructor
  int         opt;
  std::string recordFileName = "last_record";
  while ((opt = getopt(argc, argv, "r:p:P:")) != -1) {
    switch (opt) {
      case 'r':
        Recorder::state = Recorder::DoRecord;
        recordFileName  = optarg;
        break;
      case 'P':
      case 'p':
        Recorder::state = Recorder::DoReplay;
        recordFileName  = optarg;
        Recorder::loadRecords(recordFileName);
        if (opt == 'p') glfwSwapInterval(0);
        break;
      default:
        std::cout << "usage: " << argv[0]
                  << " [-r record_file | -p play_file]\n";
        return 1;
        break;
    }
  }

  // TODO: Use correct method to retrieve viewport size
  Soleil::Ruine r(AssetService::Instance.get(), SoundService::Instance.get(),
                  width, height);
  render(window, r);

  glfwTerminate();

  if (Recorder::state == Recorder::DoRecord)
    SOLEIL__RECORD_DUMP(
      recordFileName); // TODO: Undefined macro on compilation?

  return 0;
}
