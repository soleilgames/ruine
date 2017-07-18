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

#include "OpenGLInclude.hpp"
#include "stringutils.hpp"
#include "types.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/matrix.hpp>

#define IMGUI_API
//#include "imgui_impl_glfw.h"
#include "imgui_impl_glfw_gl3.h"
#include <imgui.h>
#include <stdio.h>

#include "AssetService.hpp"
#include "ControllerService.hpp"
#include "DesktopAssetService.hpp"
#include "DesktopSoundService.hpp"
#include "Draw.hpp"
#include "OpenGLDataInstance.hpp"
#include "SoundService.hpp"
#include "WavefrontLoader.hpp"

using namespace Soleil;

// TODO: Required by Draw command 'for debug' - Should not be.
Push&
ControllerService::GetPush(void) noexcept
{
  static Push p;
  return p;
}

Controller&
Soleil::ControllerService::GetPlayerController() noexcept
{
  static Controller c;

  return c;
}
// -------------------------

static void
errorCallback(int error, const char* description)
{
  std::cerr << "GLFW failed with error N." << error << ": " << description;
}

static std::string lineDebug;

namespace Soleil {

  namespace gui {

    struct ShapeElement
    {
      std::shared_ptr<Shape> shape;
      std::string            name;
    };

    class CursorSelection
    {
    public:
      CursorSelection()
        : isSet(false)
        , object(0, glm::mat4(), nullptr)
      {
      }

    public:
      void draw(const Frame& frame)
      {
        if (isSet) {
          ImGuiIO& io = ImGui::GetIO();

          glm::vec2 viewport =
            glm::vec2(OpenGLDataInstance::Instance().viewport.x,
                      OpenGLDataInstance::Instance().viewport.y);
          glm::vec4 screenPosition(
            (-0.5f + (io.MousePos.x / viewport.x)) * 2.0f,
            (-0.5f + (io.MousePos.y / viewport.y)) * -2.0f, 1.0f, 1.0f);

          lineDebug = toString("ScreenPosition:", screenPosition,
                               ". io.MousePos.y=", io.MousePos.y);

#if 0
          glm::vec3 worldPos = glm::unProject(
            glm::vec3(screenPosition), frame.View, frame.Projection,
            glm::vec4(0, 0, OpenGLDataInstance::Instance().viewport.x,
                      OpenGLDataInstance::Instance().viewport.y));
          // TODO: Another way to get (and manage the viewport)
          SOLEIL__LOGGER_DEBUG(toString("WorldPos", worldPos));
          // worldPos.y = 0.0f;
          glm::mat4 transformation = glm::translate(glm::mat4(), worldPos);
#else
          glm::mat4 inverseVP = glm::inverse(frame.ViewProjection);

          glm::vec4 worldPosition = inverseVP * screenPosition;
          glm::vec3 dir           = glm::normalize(glm::vec3(worldPosition));
          float     dist;
          glm::intersectRayPlane(frame.cameraPosition, dir, glm::vec3(0.0f),
                                 glm::vec3(0, 1, 0), dist);
          lineDebug += toString("\n>dist=", dist);
          lineDebug += toString("\n>worldPosition=", worldPosition);

          glm::vec3 clipped = (frame.cameraPosition) + dir * dist;
          clipped.x         = (int)clipped.x;
          clipped.z         = (int)clipped.z;
          lineDebug += toString("\n>clipped=", clipped);

          glm::mat4 transformation = glm::translate(glm::mat4(), clipped);

#endif

          object.transformation = transformation;

          RenderInstances r = {object};

          RenderFlatShape(r, frame);
        }
      }
      void set(const Shape& shape)
      {
        ImGuiIO& io = ImGui::GetIO();

        glm::mat4 transformation = glm::translate(
          glm::mat4(), glm::vec3(io.MousePos.x, 0.0f, io.MousePos.y));
        object = DrawCommand(shape, transformation);
        isSet  = true;
      }
      void               clear(void) { isSet = false; }
      bool               isActive(void) const noexcept { return isSet; }
      const DrawCommand& getDrawCommand(void) const noexcept { return object; }

    private:
      bool        isSet;
      DrawCommand object;
    };

  } // gui

} // Soleil

static float                TranslationSpeed = -1.0f;
static gui::CursorSelection cursorSelection;

static void
updateTranslation(glm::vec3& position, glm::vec3& center)
{
  ImGuiIO& io = ImGui::GetIO();

  static bool dirty = true;
  if (io.MouseDown[2]) {
    static ImVec2 Prev = io.MousePos;
    if (dirty) {
      Prev  = io.MousePos;
      dirty = false;
    }

    glm::vec3 translation =
      glm::vec3(Prev.x - io.MousePos.x, 0.0f, Prev.y - io.MousePos.y) *
      io.DeltaTime * TranslationSpeed;
    SOLEIL__LOGGER_DEBUG(
      toString("-----------------Translation:", translation));

    position += translation;
    center += translation;
    Prev = io.MousePos;
  } else {
    dirty = true;
  }
}

static void
render(GLFWwindow* window)
{

  ImVec4 clear_color = ImColor(114, 144, 154);
  int    width, height;
  glfwGetFramebufferSize(window, &width, &height);

  AssetService::Instance = std::make_shared<DesktopAssetService>("media/");
  SoundService::Instance = std::make_unique<DesktopSoundService>();
  OpenGLDataInstance::Initialize(); // TODO: Required by MTL - Should not be?
  OpenGLDataInstance::Instance().viewport = glm::vec2(width, height);

  glm::mat4 projection = glm::perspective(
    glm::radians(50.0f), (float)width / (float)height, 0.1f, 50.0f);
  glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 8.0f, -8.0f), glm::vec3(0.0f),
                               glm::vec3(0, 1, 0));

  std::vector<gui::ShapeElement> shapes = {
    {Soleil::WavefrontLoader::fromContent(
       AssetService::LoadAsString("wallcube.obj")),
     "Wall cube"},
    {Soleil::WavefrontLoader::fromContent(
       AssetService::LoadAsString("barrel.obj")),
     "Barrel"},
    {Soleil::WavefrontLoader::fromContent(
       AssetService::LoadAsString("floor.obj")),
     "Floor"},
    {Soleil::WavefrontLoader::fromContent(
       AssetService::LoadAsString("gate.obj")),
     "Gate heavy"},
    {Soleil::WavefrontLoader::fromContent(
       AssetService::LoadAsString("key.obj")),
     "Key"},
    {Soleil::WavefrontLoader::fromContent(
       AssetService::LoadAsString("coin.obj")),
     "Coin"},
  };
  RenderInstances statics;

#if 0
  std::shared_ptr<Shape> shape = Soleil::WavefrontLoader::fromContent(
    AssetService::LoadAsString("wallcube.obj"));  
  statics.emplace_back(*shape, glm::mat4());

  std::shared_ptr<Shape> grid = Soleil::WavefrontLoader::fromContent(
    AssetService::LoadAsString("grid.obj"));
  statics.emplace_back(*grid, glm::scale(glm::mat4(), glm::vec3(8.0f)));
#endif


  
  Frame frame;

#if 0
  frame.pointLights.push_back(
    {glm::vec3(0.0f, 1.0f, -5.0f), glm::vec3(1.0f), 0.014f, 0.0007f});
  frame.pointLights.push_back(
    {glm::vec3(0.0f, 8.0f, -8.0f), glm::vec3(1.0f), 0.014f, 0.0007f});
  frame.pointLights.push_back(
    {glm::vec3(0.0f, 8.0f, 8.0f), glm::vec3(1.0f), 0.014f, 0.0007f});
  frame.pointLights.push_back(
    {glm::vec3(-8.0f, 8.0f, 0.0f), glm::vec3(1.0f), 0.014f, 0.0007f});
  frame.pointLights.push_back(
    {glm::vec3(8.0f, 8.0f, 0.0f), glm::vec3(1.0f), 0.014f, 0.0007f});
#endif

  frame.cameraPosition = glm::vec3(2.0f, 8.0f, -8.0f);
  // frame.ambiant = glm::vec4(1.0f);

  gl::FrameBuffer  frameBuffer;
  gl::Texture      fbTexture;
  gl::RenderBuffer renderBuffer;

  {
    glEnable(GL_DEPTH_TEST);
    gl::BindFrameBuffer bindFb(GL_FRAMEBUFFER, *frameBuffer);
    {
      gl::BindTexture BindTex(GL_TEXTURE_2D, *fbTexture);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, NULL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           *fbTexture, 0);

    {
      gl::BindRenderBuffer bindRB(GL_RENDERBUFFER, *renderBuffer);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width,
                            height);
    }
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, *renderBuffer);

    throwOnGlError();
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      throw std::runtime_error("Framebuffer is not complete!");
  }

  while (!glfwWindowShouldClose(window)) {
    Soleil::Timer time((int)(glfwGetTime() * 1000));
    glfwPollEvents();

#if 0    
    view = glm::rotate(view, 0.01f, glm::vec3(0, 1, 0));
#endif
    static glm::vec3 eye    = glm::vec3(0.0f, 8.0f, 0.0f);
    static glm::vec3 center = glm::vec3(0);
    updateTranslation(eye, center);
    view = glm::lookAt(eye, center, glm::vec3(0, 0, 1));
    frame.updateViewProjectionMatrices(view, projection);
    frame.cameraPosition = eye;
    frame.delta          = time - frame.time;
    frame.time           = time;

    {
      gl::BindFrameBuffer bindFB(GL_FRAMEBUFFER, *frameBuffer);
      glEnable(GL_DEPTH_TEST);
      glViewport(0, 0, width, height);
      glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LESS);

#if 1
      // TODO: FIXME: Actually It does not work without due to the FrameBuffer
      // depth that does not work.
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
#endif
      // DrawImage(*OpenGLDataInstance::Instance().textures[0], glm::mat4());
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      cursorSelection.draw(frame);
      RenderFlatShape(statics, frame);
    }

    ImGui_ImplGlfwGL3_NewFrame();

#if 0
    ImGui::Begin("Another Window");
    // Get the current cursor position (where your window is)
    ImVec2 pos = ImGui::GetCursorScreenPos();

    GLuint tex = *OpenGLDataInstance::Instance().textures[0];

    float w = 1920;
    float h = 1080;
    // Ask ImGui to draw it as an image:
    // Under OpenGL the ImGUI image type is GLuint
    // So make sure to use "(void *)tex" but not "&tex"
    ImGui::GetWindowDrawList()->AddImage(
      (void*)tex, ImVec2(ImGui::GetItemRectMin().x + pos.x,
                         ImGui::GetItemRectMin().y + pos.y),
      ImVec2(pos.x + w / 2, pos.y + h / 2), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
#else
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("foobar", NULL, ImVec2(0, 0), 0.0f,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoScrollWithMouse |
                   ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Get the current cursor position (where your window is)
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // GLuint tex = *OpenGLDataInstance::Instance().textures[0];
    GLuint tex = *fbTexture;

    float w = 1920;
    float h = 1080;
    // Ask ImGui to draw it as an image:
    // Under OpenGL the ImGUI image type is GLuint
    // So make sure to use "(void *)tex" but not "&tex"
    ImGui::GetWindowDrawList()->AddImage(
      (void*)tex, ImVec2(ImGui::GetItemRectMin().x + pos.x,
                         ImGui::GetItemRectMin().y + pos.y),
      ImVec2(pos.x + w, pos.y + h), ImVec2(0, 1), ImVec2(1, 0));

    // Check if we have to add an entity
    if (cursorSelection.isActive() && ImGui::IsMouseClicked(0)) {
      statics.push_back(cursorSelection.getDrawCommand());
    }
    ImGui::End();

#endif

    ImGui::Begin("Objects");
    static size_t selected = 0;
    ImGui::BeginChild("shape list", ImVec2(150, 0), true);
    for (size_t i = 0; i < shapes.size(); i++) {
      if (ImGui::Selectable(shapes[i].name.c_str(), selected == i))
        selected = i;
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("buttons");
    if (ImGui::Button("Add")) {
      // statics.emplace_back(*shapes[selected].shape, glm::mat4());
      cursorSelection.set(*shapes[selected].shape);
    }
    ImGui::EndChild();
    ImGui::End();

    // 1. Show a simple window
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in
    // a window automatically called "Debug"
    {
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::Text(">%s", lineDebug.c_str());
    }

    // Rendering
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplGlfwGL3_Shutdown();
  glfwTerminate();
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

#if 1
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
  window = glfwCreateWindow(width, height, "Ruine Editor", nullptr, nullptr);
#else
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window = glfwCreateWindow(1280, 720, "ImGui OpenGL3 example", NULL, NULL);
#endif

  if (!window) {
    glfwTerminate();
    return -1;
  }

#if 0  
  glfwSetKeyCallback(window, keyCallback);
  glfwSetCursorPosCallback(window, cursor_positionCallback);
  glfwSetMouseButtonCallback(window, mouse_buttonCallback);
#endif
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glewExperimental = GL_TRUE;
  GLenum err       = glewInit();
  if (err != GLEW_OK) {
    throw std::runtime_error(
      toString("Unable to initialize GLEW: ", glewGetErrorString(err)));
  }

  ImGui_ImplGlfwGL3_Init(window, true);

  render(window);

  glfwTerminate();

  return 0;
}
