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

#include "editor.hpp"

#include <chrono>
#include <cstring>
#include <memory>
#include <set>
#include <stdexcept>

#include <unistd.h> // getopt

#include "stringutils.hpp"
#include "types.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/matrix.hpp>

#include <World.hpp>
#include <fstream>
#include <functional>

#define IMGUI_API
#include "imgui_impl_glfw_gl3.h"
#include <imgui.h>
#include <stdio.h>

#include "AssetService.hpp"
#include "ControllerService.hpp"
#include "DesktopAssetService.hpp"
#include "DesktopSoundService.hpp"
#include "Draw.hpp"
#include "EventService.hpp"
#include "OpenGLDataInstance.hpp"
#include "Pristine.hpp"
#include "SoundService.hpp"
#include "WavefrontLoader.hpp"

using namespace Soleil;

static std::string                           lineDebug;
static std::unique_ptr<gui::EditorResources> editorResources;
static World                                 world;
static std::vector<const char*>              doorNames;
static int                                   doorSelected;
constexpr int                                FILENAMESIZE = 255;
static char fileName[FILENAMESIZE] = "corpsdegarde.level";

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

void
saveMat4(std::ofstream& os, const glm::mat4& mat)
{
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      os << mat[x][y] << " ";
    }
  }
}

void
saveVec3(std::ofstream& os, const glm::vec3& v)
{
  for (int x = 0; x < 3; ++x) {
    os << v[x] << " ";
  }
}

void
saveBoundingBox(std::ofstream& os, const BoundingBox& b)
{
  saveVec3(os, b.getMin());
  os << " ";
  saveVec3(os, b.getMax());
  os << " ";
}

static void
errorCallback(int error, const char* description)
{
  std::cerr << "GLFW failed with error N." << error << ": " << description;
}

gui::EditorResources::EditorResources()
{
  gridProgram.attachShader(Shader(GL_VERTEX_SHADER, "grid.vert"));
  gridProgram.attachShader(Shader(GL_FRAGMENT_SHADER, "grid.frag"));
  glBindAttribLocation(gridProgram.program, 0, "position");
  gridProgram.compile();

  gridMVP = gridProgram.getUniform("MVP");
}

namespace Soleil {

  std::map<EventService::Event, std::vector<EventService::Callback>>
    EventService::callbacks;

  namespace gui {

    void pushCoin(DrawElement& draw, std::vector<DrawElement>& objects,
                  std::vector<Trigger>& triggers)
    {
      objects.push_back(draw);
      triggers.push_back(
        {BoundingBox(draw.transformation, 0.25f), // TODO: use gval
         TriggerState::NeverTriggered, TriggerType::Coin, draw.id});
    }

    void pushGhost(DrawElement& draw, std::vector<DrawElement>& objects,
                   std::vector<Trigger>&        triggers,
                   const std::vector<ShapePtr>& shapes)
    {
      BoundingBox box = shapes[ShapeType::Ghost]->makeBoundingBox();
      box.transform(draw.transformation);
      objects.push_back(draw);
      assert(false && "Restore ghost bounds in editor");
      // triggers.push_back({box, // TODO: use gval
      //                     TriggerState::NeverTriggered, TriggerType::Ghost,
      //                     draw.id});
    }

    void setKey(DrawElement& draw, World& world)
    {
      BoundingBox box = world.shapes[ShapeType::Key]->makeBoundingBox();

      box.transform(draw.transformation);
      world.items.push_back(draw);
      world.triggers.push_back(
        {box, TriggerState::NeverTriggered, TriggerType::Key, draw.id});
    }

    static void refreshDoorList()
    {
      doorNames.clear();
      for (auto& d : world.doors) {

        // TODO: The gui editor requires to have a specific size to works
        // correctly but we need to resize if here because it invalidate the
        // 'c_str()' pointer
        d.id.reserve(64);
        d.name.reserve(64);

        doorNames.push_back(d.id.c_str());
      }
    }

    void pushDoor(const glm::mat4& transformation, World& world)
    {
      BoundingBox aoe(transformation, 0.1f);

      Door d;
      d.id          = RandomString(8);
      d.name        = L"";
      d.output      = "";
      d.uid         = std::hash<std::string>{}(d.id);
      d.triggerZone = aoe;
      world.doors.push_back(d);

      world.triggers.push_back(
        Trigger{aoe, TriggerState::NeverTriggered, TriggerType::Door, d.uid});

      refreshDoorList();
    }

    void newMap(std::vector<DrawElement>& elements,
                std::vector<DrawElement>& objects,
                std::vector<DrawElement>& ghosts,
                std::vector<Trigger>&     triggers)
    {
      std::string line;

      elements.clear();
      objects.clear();
      ghosts.clear();
      triggers.clear();

      EventService::Trigger(Events::event_map_loaded);
    }

    void loadMap(World& world, const std::string& fileName)
    {
      Frame f;

      clearSelection();
      world.resetLevel();
      world.doors.clear();

      InitializeWorldModels(world);
      InitializeWorldDoors(world, "doors.ini");
      std::istringstream is(AssetService::LoadAsString(fileName));

      ::Soleil::loadMap(world, f, is);
      for (const Door& door : world.doors) {
        if (fileName == door.level)
          world.triggers.push_back({door.triggerZone,
                                    TriggerState::NeverTriggered,
                                    TriggerType::Door, door.uid});
      }

      EventService::Trigger(Events::event_map_loaded);
    }

    Console::Console()
      : lines(255)
      , start(0)
      , end(0)
    {
    }

    Console::~Console() {}

    void Console::push(const std::string& text)
    {
      if (end >= lines.capacity()) end = 0;

      lines[end] = text;
      end++;
      if (end <= start) {
        start++;
        if (start >= lines.capacity()) start = 0;
      }
    }

    void Console::render(void)
    {
      int size = lines.capacity();

      size_t current = end;
      while (size > 0) {
        if (current >= lines.capacity()) current = 0;
        if (current == start) return;

        ImGui::Text("> %s", lines[current].c_str());
        current--;
        size--;
      }
    }

    class CursorSelection
    {
    public:
      CursorSelection()
        : isSet(false)
        , shape(-1)
      {
      }

    public:
      void getRay(glm::vec3& orig, glm::vec3& dir, const Frame& frame)
      {
        ImGuiIO& io = ImGui::GetIO();

        glm::vec3 mouse(io.MousePos.x,
                        OpenGLDataInstance::Instance().viewport.y - 1.0f -
                          io.MousePos.y,
                        0.0f);
        orig = glm::unProject(
          mouse, frame.View, frame.Projection,
          glm::vec4(0, 0, OpenGLDataInstance::Instance().viewport.x,
                    OpenGLDataInstance::Instance().viewport.y));
        // TODO: Another way to get (and manage the viewport)

        mouse.z                    = 1.0f;
        glm::vec3 worldPositionFar = glm::unProject(
          mouse, frame.View, frame.Projection,
          glm::vec4(0, 0, OpenGLDataInstance::Instance().viewport.x,
                    OpenGLDataInstance::Instance().viewport.y));

        dir = glm::normalize(worldPositionFar - orig);
      }

      void draw(const std::vector<ShapePtr>& shapes, const Frame& frame)
      {
        if (isSet) {
          ImGuiIO& io = ImGui::GetIO();

          float dist;

          glm::vec3 orig;
          glm::vec3 dir;
          getRay(orig, dir, frame);

          glm::intersectRayPlane(orig, dir, glm::vec3(0.0f), glm::vec3(0, 1, 0),
                                 dist);
          glm::vec3 clipped = (orig) + dir * dist;
          if (io.KeyCtrl) {
            clipped.x = (int)clipped.x;
            clipped.z = (int)clipped.z;
          }
          this->transformation = glm::translate(glm::mat4(), clipped);

          if (shape == ShapeType::ST_BoundingBox)
            DrawBoundingBox(BoundingBox(transformation, 0.1f), frame);
          else
            RenderFlatShape(transformation, *shapes[shape], frame);
        }
      }
      void set(size_t shape)
      {
        this->shape = shape;
        this->isSet = true;
      }
      void clear(void)
      {
        isSet = false;
        shape = -1;
      }
      bool             isActive(void) const noexcept { return isSet; }
      const glm::mat4& getTransformation(void) const noexcept
      {
        return transformation;
      }
      size_t getShape(void) noexcept { return shape; }

    private:
      bool      isSet;
      size_t    shape; // TODO: Will be replaced by the enum
      glm::mat4 transformation;
    };

    struct Grid
    {
      gl::Buffer            buffer;
      glm::mat4             transformation;
      std::vector<GLushort> indices;

      Grid(const int width, const int height, const float unitSize = 1.0f)
      {
        std::vector<glm::vec3> points;

        const glm::vec3 square[] = {{0.0f, 0.0f, 0.0f},
                                    {1.0f, 0.0f, 0.0f},
                                    {1.0f, 0.0f, 1.0f},
                                    {0.0f, 0.0f, 1.0f}};

        int ids = 0;
        int y   = 0.0f;
        while (y < height) {

          int x = 0.0f;
          while (x < width) {
            glm::vec3 pos = glm::vec3(x, 0, y) * unitSize;
            points.push_back(unitSize * square[0] + pos);
            points.push_back(unitSize * square[1] + pos);
            points.push_back(unitSize * square[2] + pos);
            points.push_back(unitSize * square[3] + pos);

            indices.push_back(0 + ids);
            indices.push_back(1 + ids);
            indices.push_back(1 + ids);
            indices.push_back(2 + ids);
            indices.push_back(2 + ids);
            indices.push_back(3 + ids);
            indices.push_back(3 + ids);
            indices.push_back(0 + ids);

            ids += 4;
            x++;
          }
          y++;
        }

        glBindBuffer(GL_ARRAY_BUFFER, *buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(points[0]) * points.size(),
                     points.data(), GL_STATIC_DRAW);
        throwOnGlError();
      }

      void draw(const Frame& frame)
      {
        const GLsizei stride = sizeof(glm::vec3);
        glUseProgram(editorResources->gridProgram.program);
        glBindBuffer(GL_ARRAY_BUFFER, *buffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                              (const GLvoid*)0);
        glEnableVertexAttribArray(0);

        glUniformMatrix4fv(
          editorResources->gridMVP, 1, GL_FALSE,
          glm::value_ptr(frame.ViewProjection * transformation));

        glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_SHORT,
                       indices.data());
        throwOnGlError();
      }
    };

    void parseMaze(World& world, std::vector<DrawElement>& elements,
                   std::istringstream& s)
    {
      clearSelection();
      world.resetLevel();
      world.doors.clear();

      InitializeWorldModels(world);
      InitializeWorldDoors(world, "doors.ini");

      std::string            line;
      float                  x     = 0.0f;
      float                  z     = 0.0f;
      const static glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(1.0f));

      RenderInstances lateComer;

      while (std::getline(s, line) && line.size() > 0) {
        for (auto const& c : line) {
          const glm::vec3 position(2.0f * x, 0.0f, 2.0f * z);

          if (c == 'x') {
            const glm::mat4 transformation = glm::translate(scale, position);

            elements.push_back({ShapeType::WallCube, transformation});

          } else if (c == 'G') {
            const glm::mat4 transformation = glm::translate(scale, position);

            elements.push_back({ShapeType::GateHeavy, transformation});

          } else {
            if (c == 'g') {
              glm::mat4 transformation =
                glm::translate(glm::mat4(),
                               glm::vec3(position.x, 0.00f, position.z)) *
                glm::scale(glm::mat4(), glm::vec3(1.0f));

              DrawElement d{ShapeType::Ghost, transformation};
              pushGhost(d, world.ghosts, world.triggers, world.shapes);
            } else if (c == 'p') {
              const glm::vec3 coinPosition =
                glm::vec3(position.x, 0.0f, position.z);
              const glm::mat4 transformation =
                glm::translate(glm::mat4(), coinPosition);

              DrawElement d{ShapeType::Coin, transformation};
              pushCoin(d, world.items, world.triggers);

            } else if (c == 'k') {
              const glm::vec3 keyPosition =
                glm::vec3(position.x, -1.0f, position.z);
              const glm::mat4 transformation =
                glm::scale(glm::translate(scale, keyPosition), glm::vec3(2.0f));

              world.theKey = transformation;
            } else if (c == 'b' || c == 'B') {
              const glm::vec3 barrelPosition =
                glm::vec3(position.x, -1.0f, position.z);
              const glm::mat4 transformation =
                glm::translate(scale, barrelPosition);

            } else if (c == 'A') {
              glm::mat4 transformation =
                glm::rotate(
                  glm::translate(glm::mat4(),
                                 glm::vec3(position.x, -0.60f, position.z)),
                  glm::half_pi<float>(), glm::vec3(0, 1, 0)) *
                glm::scale(glm::mat4(), glm::vec3(.3f));
            }

            glm::mat4 groundTransformation =
              glm::translate(scale, glm::vec3(2.0f * x, -0.0f, 2.0f * z));

            glm::mat4 ceilTransformation =
              glm::translate(scale, glm::vec3(2.0f * x, 0.0f, 2.0f * z)) *
              glm::rotate(glm::mat4(), glm::pi<float>(),
                          glm::vec3(0.0f, 0.0f, 1.0f));

            elements.push_back({ShapeType::Floor, groundTransformation});
          }
          x += 1.0f;
        }
        z += 1.0f;
        x = 0.0f;
      }
      EventService::Trigger(Events::event_map_loaded);
    }

    static float                    TranslationSpeed = -1.0f;
    static gui::CursorSelection     cursorSelection;
    static gui::Console             console;
    static std::vector<BoundingBox> debugBox;
    static std::size_t              selection     = 0;
    static std::size_t              selectionType = 0;
    static bool                     selected      = false;
    static std::string              loadLevelOnStartup;
    static std::string              loadOldLevelOnStartup;

    void pick(const glm::vec3& orig, const glm::vec3& dir,
              std::vector<Selectable>& selectables)
    {
      clearSelection();

      BoundingBox bbox;
      float       distance = std::numeric_limits<float>::max();

      selected = false;
      for (const auto s : selectables) {
        float     t;
        glm::vec3 point;
        if (s.boundingBox.rayIntersect(orig, dir, t)) {
          console.push(
            toString("Intersection with ptr=", s.id, " at distance:", t));

          if (t < distance) {
            distance      = t;
            selection     = s.id;
            selectionType = s.type;
            bbox          = s.boundingBox;
            selected      = true;
          }
        }
      }

      if (selected == false) return;

      bbox.expandBy(bbox.getMin() - 0.1f);
      bbox.expandBy(bbox.getMax() + 0.1f);
      debugBox.push_back(bbox);
    }

    void worldAsSelectable(std::vector<Selectable>& selectables,
                           const World&             world)
    {
      std::map<int, BoundingBox> prepared;
      selectables.clear();

      const auto items = {world.elements, world.items, world.ghosts};

      for (const auto& item : items) {
        for (auto it = item.begin(); it != item.end(); ++it) {
          auto& element = *it;
          if (prepared.find(element.shapeIndex) == prepared.end()) {
            prepared[element.shapeIndex] =
              world.shapes[element.shapeIndex]->makeBoundingBox();
          }

          BoundingBox b = prepared[element.shapeIndex];
          b.transform(element.transformation);

          console.push(toString("> ", (void*)&element));
          selectables.push_back({b, element.id, element.shapeIndex});
        }
      }

      for (const auto& t : world.triggers) {
        if (t.type == TriggerType::Door) {
          selectables.push_back({t.aoe, t.link, ShapeType::ST_BoundingBox});
        }
      }
    }

    static bool RemoveFromVector(std::vector<DrawElement>& elements,
                                 std::size_t               uid)
    {
      for (auto it = elements.begin(); it != elements.end(); ++it) {
        auto& el = *it;

        if (el.id == uid) {
          elements.erase(it);
          return true;
        }
      }
      return false;
    }

    static bool RemoveFromTriggers(std::vector<Trigger>& triggers, World& world,
                                   std::size_t uid)
    {
      for (auto it = triggers.begin(); it != triggers.end(); ++it) {
        auto& el = *it;

        if (el.link == uid) {
          // If it's a door we also need to remove it's name in the door list.
          // Otherwise we also need to remove the World object,
          bool doContinueRemoval = (it->type != TriggerType::Door);

          triggers.erase(it);

          if (doContinueRemoval) return false;

          // Also remove the door from the list
          for (auto itd = world.doors.begin(); itd != world.doors.end();
               ++itd) {
            if (itd->uid == uid) {
              world.doors.erase(itd);
              return true;
            }
          }

          assert(false && "No door associated with trigger");
        }
      }
      return false;
    }

    void worldBuilderDelete(std::size_t ptr, World& world)
    {
      if (RemoveFromTriggers(world.triggers, world, ptr)) return;
      if (RemoveFromVector(world.elements, ptr)) return;
      if (RemoveFromVector(world.items, ptr)) return;
      if (RemoveFromVector(world.ghosts, ptr)) return;

      console.push(toString("No item found!! ", ptr));
    }

    void worldBuilderYawItem(const std::size_t ptr, const float yaw,
                             World& world)
    {
      auto search = [&ptr, yaw](std::vector<DrawElement>& elements) {
        for (auto it = elements.begin(); it != elements.end(); ++it) {
          auto& el = *it;

          if (el.id == ptr) {
            el.transformation =
              glm::rotate(el.transformation, yaw, glm::vec3(0, 1, 0));
            return true;
          }
        }
        return false;
      };

      if (search(world.elements)) return;
      if (search(world.items)) return;
      if (search(world.ghosts)) return;

      console.push(toString("No item found!! ", ptr));
    }

    void updateTranslation(glm::vec3& position, glm::vec3& center)
    {
      ImGuiIO& io = ImGui::GetIO();

      static bool dirty = true;
      if (io.MouseDown[1]) {
        static ImVec2 Prev = io.MousePos;
        if (dirty) {
          Prev  = io.MousePos;
          dirty = false;
        }

        glm::vec3 translation =
          glm::vec3(Prev.x - io.MousePos.x, 0.0f, Prev.y - io.MousePos.y) *
          io.DeltaTime * TranslationSpeed;

        position += translation;
        center += translation;
        Prev = io.MousePos;
      } else {
        dirty = true;
      }
    }

    void clearSelection()
    {
      debugBox.clear();
      selection     = 0;
      selectionType = 0;
      selected      = false;
      doorSelected  = -1;
    }

    static void render(GLFWwindow* window)
    {

      // Any options the gui may temporary customize
      bool   doRenderStatics  = true;
      bool   doRenderTriggers = true;
      ImVec4 clear_color      = ImColor(114, 144, 154);
      int    width, height;
      glfwGetFramebufferSize(window, &width, &height);

      // GL resources initialization
      AssetService::Instance = std::make_shared<DesktopAssetService>("media/");
      SoundService::Instance = std::make_unique<DesktopSoundService>();
      OpenGLDataInstance::Initialize();
      editorResources = std::make_unique<EditorResources>();
      OpenGLDataInstance::Instance().viewport = glm::vec2(width, height);

      glm::mat4 projection = glm::perspective(
        glm::radians(50.0f), (float)width / (float)height, 0.1f, 50.0f);
      glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 8.0f, -8.0f),
                                   glm::vec3(0.0f), glm::vec3(0, 1, 0));

      gui::Grid grid(20, 20);
      gui::Grid grid2(5, 5);
      gui::Grid grid3(5, 5);
      grid2.transformation =
        glm::rotate(glm::mat4(), -glm::half_pi<float>(), glm::vec3(1, 0, 0));
      grid3.transformation =
        glm::rotate(glm::mat4(), glm::half_pi<float>(), glm::vec3(0, 0, 1));

      auto ghost = Soleil::WavefrontLoader::fromContent(
        AssetService::LoadAsString("ghost.obj"));
      world.shapes = {Soleil::WavefrontLoader::fromContent(
                        AssetService::LoadAsString("wallcube.obj")),
                      Soleil::WavefrontLoader::fromContent(
                        AssetService::LoadAsString("barrel.obj")),
                      Soleil::WavefrontLoader::fromContent(
                        AssetService::LoadAsString("floor.obj")),
                      Soleil::WavefrontLoader::fromContent(
                        AssetService::LoadAsString("gate.obj")),
                      Soleil::WavefrontLoader::fromContent(
                        AssetService::LoadAsString("key.obj")),
                      Soleil::WavefrontLoader::fromContent(
                        AssetService::LoadAsString("coin.obj")),
                      ghost,
                      ghost};
      // All the shape that can be used in game. In a future release it could be
      // configured per level?

      std::vector<gui::ShapeElement> guiStaticsShapes = {
        {ShapeType::WallCube, "Wall cube"},
        {ShapeType::Barrel, "Barrel"},
        {ShapeType::Floor, "Floor"},
        {ShapeType::GateHeavy, "Gate heavy"},
        {ShapeType::GhostFriend, "Friend"},
      };

      std::vector<gui::ShapeElement> guiObjectsShapes = {
        {ShapeType::Coin, "Coin"},
        {ShapeType::Ghost, "Ghost"},
        {ShapeType::Key, "The Key"},
      };

      Frame                          frame;
      static std::vector<Selectable> selectables;

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
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, *fbTexture, 0);

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

      {
        Frame dummyFrame;

        EventService::Push(Events::event_map_loaded, [&]() {
          worldAsSelectable(selectables, world);
          console.push(toString("selectable size: ", selectables.size()));
          refreshDoorList();
        });

        InitializeWorldModels(world);
        // Load a level if requested
        if (loadLevelOnStartup.empty() == false) {
          assert(loadLevelOnStartup.size() < FILENAMESIZE &&
                 "File name too long");
          strncpy(fileName, loadLevelOnStartup.data(), FILENAMESIZE);

          loadMap(world, loadLevelOnStartup);
        } else if (loadOldLevelOnStartup.empty() == false) {
          assert(loadOldLevelOnStartup.size() < FILENAMESIZE &&
                 "File name too long");
          strncpy(fileName, loadOldLevelOnStartup.data(), FILENAMESIZE);

          std::istringstream is(
            AssetService::LoadAsString(loadOldLevelOnStartup));
          parseMaze(world, world.elements, is);
        }
      }

      while (!glfwWindowShouldClose(window)) {
        Soleil::Timer time((int)(glfwGetTime() * 1000));
        glfwPollEvents();
        const ImGuiIO& io = ImGui::GetIO();

#if 0    
    view = glm::rotate(view, 0.01f, glm::vec3(0, 1, 0));
#endif
        static glm::vec3 eye    = glm::vec3(0.0f, 8.0f, -.0f);
        static glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
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
          glClearColor(clear_color.x, clear_color.y, clear_color.z,
                       clear_color.w);
          glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          glEnable(GL_DEPTH_TEST);
          glDepthFunc(GL_LESS);

#if 1
          // TODO: FIXME: Actually It does not work without due to the
          // FrameBuffer
          // depth that does not work.
          glEnable(GL_CULL_FACE);
          glCullFace(GL_BACK);
#endif
          // DrawImage(*OpenGLDataInstance::Instance().textures[0],
          // glm::mat4());
          glEnable(GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

          cursorSelection.draw(world.shapes, frame);
          grid.draw(frame);
          grid2.draw(frame);
          grid3.draw(frame);
          // RenderFlatShape(statics, frame);
          // RenderFlatShape(world.statics, frame);
          if (doRenderStatics) {
            for (const auto& e : world.elements) {
              RenderFlatShape(e.transformation, *world.shapes[e.shapeIndex],
                              frame);
            }
            for (const auto& e : world.items) {
              RenderFlatShape(e.transformation, *world.shapes[e.shapeIndex],
                              frame);
            }
            for (const auto& e : world.ghosts) {
              RenderFlatShape(e.transformation, *world.shapes[e.shapeIndex],
                              frame);
            }
          }
          if (doRenderTriggers) {
            for (const auto& t : world.triggers) {
              DrawBoundingBox(t.aoe, frame);
            }
          }

          for (const auto b : debugBox) {
            DrawBoundingBox(b, frame, RGBA(1.0f, 0.5f, 0.2f, 0.5f));
          }
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

        // Update Eye z;
        if (ImGui::IsWindowHovered()) {
          const ImGuiIO& io = ImGui::GetIO();
          eye.y -= io.MouseWheel;
        }

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

        if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered()) {
          // Check if we have to add an entity
          if (cursorSelection.isActive()) {
            DrawElement draw{cursorSelection.getShape(),
                             cursorSelection.getTransformation()};
            switch (cursorSelection.getShape()) {
              case ShapeType::WallCube:
              case ShapeType::Barrel:
              case ShapeType::Floor:
              case ShapeType::GhostFriend:
              case ShapeType::GateHeavy: world.elements.push_back(draw); break;
              case ShapeType::Coin:
                pushCoin(draw, world.items, world.triggers);
                break;
              case ShapeType::Ghost:
                pushGhost(draw, world.ghosts, world.triggers, world.shapes);
                break;
              case ShapeType::ST_BoundingBox:
                pushDoor(draw.transformation, world);
                break;
              case ShapeType::Key: setKey(draw, world); break;
            }
            // TODO: Triggers
            worldAsSelectable(selectables, world);
          } else {
            glm::vec3 orig;
            glm::vec3 dir;
            cursorSelection.getRay(orig, dir, frame);

            pick(orig, dir, selectables);
          }
        }
        ImGui::End();

#endif

        ImGui::Begin("Statics");
        ImGui::BeginChild("shape list", ImVec2(150, 150), true);
        for (size_t i = 0; i < guiStaticsShapes.size(); i++) {
          if (ImGui::Selectable(guiStaticsShapes[i].name.c_str(),
                                cursorSelection.isActive() &&
                                  cursorSelection.getShape() ==
                                    guiStaticsShapes[i].shapeIndex))
            cursorSelection.set(guiStaticsShapes[i].shapeIndex);
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("Objects list", ImVec2(150, 0), true);
        for (size_t i = 0; i < guiObjectsShapes.size(); i++) {
          if (ImGui::Selectable(guiObjectsShapes[i].name.c_str(),
                                cursorSelection.isActive() &&
                                  cursorSelection.getShape() ==
                                    guiObjectsShapes[i].shapeIndex))
            cursorSelection.set(guiObjectsShapes[i].shapeIndex);
        }
        if (ImGui::Selectable("Door trigger",
                              cursorSelection.isActive() &&
                                cursorSelection.getShape() ==
                                  ShapeType::ST_BoundingBox))
          cursorSelection.set(ShapeType::ST_BoundingBox);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("buttons");
        if (ImGui::Button("Clear selection")) {
          cursorSelection.clear();
        }
        ImGui::EndChild();
        ImGui::End();

        ImGui::Begin("Game");
        ImGui::Checkbox("Render statics", &doRenderStatics);
        ImGui::Checkbox("Render triggers", &doRenderTriggers);
        ImGui::End();

        ImGui::Begin("Export");
        ImGui::InputText("input text", fileName, FILENAMESIZE);
        if (ImGui::Button("Save")) {
          std::ofstream outfile("media/" + std::string(fileName));

          // I. MAP
          // # Statics:
          // 0 0.000 0.000 0.000 ...
          // ...
          //
          // # objects:
          // 0 0.000 0.000 0.000 ...
          // ...

          // II. Save
          // key true
          // dialogue 3

          const auto view = {world.elements, world.items, world.ghosts};

          for (const auto& current : view) {
            for (const auto& e : current) {
              outfile << e.shapeIndex << " ";

              saveMat4(outfile, e.transformation);
              outfile << "\n";
            }

            outfile << "\n";
          }

          // Save the doors:
          std::ofstream doorsfile("media/doors.ini");
          for (const auto& door : world.doors) {
            // TODO: to check
            doorsfile << door.id.c_str();
            doorsfile << " ";
            doorsfile << ((door.level.empty()) ? std::string(fileName)
                                               : door.level.c_str());
            doorsfile << " ";
            saveBoundingBox(doorsfile, door.triggerZone);
            doorsfile << door.output.c_str() << " ";
            doorsfile << (char*)door.name.data();
            doorsfile << "\n";
          }
        }

        if (ImGui::Button("Load Old")) {
          clearSelection();

          clearSelection();
          world.resetLevel();
          world.doors.clear();

          InitializeWorldModels(world);
          InitializeWorldDoors(world, "doors.ini");
          std::istringstream is(AssetService::LoadAsString(fileName));
          gui::parseMaze(world, world.elements, is);
        }

        if (ImGui::Button("Load New")) {
          loadMap(world, fileName);
        }

        if (ImGui::Button("New")) {
          Frame dummyFrame;

          InitializeWorldModels(world);
          newMap(world.elements, world.items, world.ghosts, world.triggers);
        }

        ImGui::End();

        if (selected) {
          ImGui::Begin("Selection");

          switch (selectionType) {
            case ShapeType::ST_BoundingBox:
              Door* d = GetDoorByUID(world.doors, selection);

              ImGui::InputText("ID", &(d->id[0]), 64);

              if (doorSelected < 0) {
                for (const auto& listed : doorNames) {
                  doorSelected++;

                  if (listed == d->output) break;
                }
              }
              int c = doorSelected;
              ImGui::ListBox("Target", &c, doorNames.data(), doorNames.size());
              if (c != doorSelected) {
                doorSelected = c;
                d->output    = doorNames[doorSelected];
              }

              d->name.resize(255, '\0');
              ImGui::InputText("Name", (char*)&(d->name[0]), 32);

              float x            = 0.0f;
              float y            = 0.0f;
              bool  modification = false;
              modification += ImGui::DragFloat("Shift x", &x, 0.001f);
              modification += ImGui::DragFloat("Shift Y", &y, 0.001f);
              if (modification) {
                d->triggerZone.transform(
                  glm::translate(glm::mat4(), glm::vec3(x, 0.0f, y)));

                // also update the trigger
                for (auto& t : world.triggers) {
                  if (t.link == d->uid) {
                    t.aoe = d->triggerZone;
                    break;
                  }
                }
              }

              // ImGui::Button("Save");
              break;
          }

          float yaw = 0.0f;
          ImGui::DragFloat("Shift x", &yaw, 0.001f);
	  if (yaw > 0.0f || yaw < 0.0f)
	    {
	      worldBuilderYawItem(selection, yaw, world);
	    }

          if (ImGui::Button("Delete") ||
              (io.KeyCtrl && io.KeysDown[(int)'X'])) {
            worldBuilderDelete(selection, world);
            worldAsSelectable(selectables, world);
            clearSelection();
          }

          ImGui::End();
        }

        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears
        // in
        // a window automatically called "Debug"
        {
          ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                      1000.0f / ImGui::GetIO().Framerate,
                      ImGui::GetIO().Framerate);
          ImGui::Text(">%s", lineDebug.c_str());
          console.render();
        }

        bool popupLoad = io.KeyCtrl && io.KeysDown[(int)'O'];

        if (io.KeysDown[io.KeyMap[ImGuiKey_::ImGuiKey_Escape]]) {
          if (selection > 0)
            clearSelection();
          else if (cursorSelection.isActive())
            cursorSelection.clear();
        }

        static std::vector<std::string> levels;
        if (popupLoad) {
          ImGui::OpenPopup("PopupLoad");

          std::set<std::string> set;
          for (const auto& d : world.doors) {
            set.emplace(d.level);
          }
          levels.clear();
          levels.reserve(set.size());
          std::copy(std::begin(set), std::end(set), std::back_inserter(levels));
        }

        if (ImGui::BeginPopup("PopupLoad")) {

          ImGui::Text("Load New Level");
          ImGui::BeginChild("Levels", ImVec2(150, 150), true);
          for (size_t i = 0; i < levels.size(); i++) {
            if (ImGui::Selectable(levels[i].c_str())) {
              std::strncpy(fileName, levels[i].c_str(), FILENAMESIZE);
              loadMap(world, levels[i]);
              ImGui::CloseCurrentPopup();
            }
          }
          ImGui::EndChild();

          ImGui::EndPopup();
        }

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        glfwSwapBuffers(window);
      }

      // Cleanup
      ImGui_ImplGlfwGL3_Shutdown();
      glfwTerminate();
    }

  } // gui

} // Soleil

int
main(int argc, char* argv[])
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

  if (argc >= 2) {

    int  c;
    auto remove_media = [](std::string s) {
      if (s.find("media/") == 0) {
        return s.substr(strlen("media/"));
      }
      return s;
    };
    while ((c = getopt(argc, argv, "n:o:")) != -1) {
      switch (c) {
        case 'n': {
          Soleil::gui::loadLevelOnStartup = remove_media(optarg);
        } break;
        case 'o': {
          Soleil::gui::loadOldLevelOnStartup = remove_media(optarg);
        } break;
      }
    }
  }
  Soleil::gui::render(window);

  glfwTerminate();

  return 0;
}
