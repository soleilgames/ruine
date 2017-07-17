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

#include "ControllerService.hpp"
#include "DesktopAssetService.hpp"
#include "DesktopSoundService.hpp"
#include "OpenGLInclude.hpp"
#include "Ruine.hpp"
#include "stringutils.hpp"

using namespace Soleil;

class DesktopControllerService
{
public:
  DesktopControllerService() {}
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
  while (!glfwWindowShouldClose(window)) {
    double time = glfwGetTime();

    r.render(std::chrono::milliseconds((int)(time * 1000)));

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
  controllerService.player.push.position =
    (glm::vec2(xpos / (float)width, -ypos / (float)height) +
     glm::vec2(-0.5f, 0.5f)) *
    2.0f;

  if (controllerService.player.push.active == PushState::Down)
    controllerService.player.push.active = PushState::Active;
}

void
mouse_buttonCallback(GLFWwindow* /*window*/, int button, int action,
                     int /*mods*/)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    controllerService.player.push.active =
      ((action == GLFW_RELEASE) ? PushState::Release : PushState::Down) |
      PushState::Fresh;
  }
}

int
main(int /*argc*/
     ,
     char* /*argv*/
     [])
{
  GLFWwindow* window;
  int         width  = 1920;
  int         height = 1080;

  glfwSetErrorCallback(errorCallback);
  if (!glfwInit()) return -1;

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

  // TODO: Use correct method to retrieve viewport size
  Soleil::Ruine r(AssetService::Instance.get(), SoundService::Instance.get(),
                  width, height);
  render(window, r);

  glfwTerminate();

  return 0;
}
