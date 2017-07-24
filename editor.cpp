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

#include <World.hpp>
#include <fstream>
#include <functional>

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
#include "Pristine.hpp"
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

// TODO: Should be in gui:: package
struct EditorResources
{
  Program gridProgram;
  GLuint  gridMVP;

  EditorResources()
  {
    gridProgram.attachShader(Shader(GL_VERTEX_SHADER, "grid.vert"));
    gridProgram.attachShader(Shader(GL_FRAGMENT_SHADER, "grid.frag"));
    glBindAttribLocation(gridProgram.program, 0, "position");
    gridProgram.compile();

    gridMVP = gridProgram.getUniform("MVP");
  }
};

static std::string                      lineDebug;
static std::unique_ptr<EditorResources> editorResources;
static World                            world;

namespace Soleil {

  class EventService
  {
  public:
    typedef std::function<void(void)> Callback;
    typedef int                       Event;

  public:
    EventService() {}
    virtual ~EventService() {}

  public:
    static void Push(Event event, Callback c) { callbacks[event].push_back(c); }

    static void Trigger(Event event)
    {
      for (const auto& c : callbacks[event]) {
        c();
      }
    }

  private:
    static std::map<Event, std::vector<Callback>> callbacks;
  };
  std::map<EventService::Event, std::vector<EventService::Callback>>
    EventService::callbacks;

  enum Events
  {
    event_map_loaded
  };

  struct DrawElement
  {
    size_t    shapeIndex;
    glm::mat4 transformation;
    int       id;

    static int getNextId(void)
    {
      static int next = -1;
      next++;
      return next;
    }
  };

  static void pushCoin(DrawElement& draw, std::vector<DrawElement>& objects,
                       std::vector<Trigger>& triggers)
  {
    objects.push_back(draw);
    triggers.push_back(
      {BoundingBox(draw.transformation, 0.25f), // TODO: use gval
       TriggerState::NeverTriggered, TriggerType::Coin});
  }

  static void pushGhost(DrawElement& draw, std::vector<DrawElement>& objects,
                        std::vector<Trigger>&        triggers,
                        const std::vector<ShapePtr>& shapes)
  {
    BoundingBox box = shapes[ShapeType::Ghost]->makeBoundingBox();
    box.transform(draw.transformation);
    objects.push_back(draw);
    triggers.push_back({box, // TODO: use gval
                        TriggerState::NeverTriggered, TriggerType::Coin});
  }

  static void newMap(std::vector<DrawElement>& elements,
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

  static void loadMap(std::vector<DrawElement>&    elements,
                      std::vector<DrawElement>&    objects,
                      std::vector<DrawElement>&    ghosts,
                      std::vector<Trigger>&        triggers,
                      const std::vector<ShapePtr>& shapes,
                      std::istringstream&          s)
  {
    std::string line;

    elements.clear();
    objects.clear();
    ghosts.clear();
    triggers.clear();
    enum Step
    {
      Statics,
      Coins,
      Ghosts,
    };
    int step = Step::Statics;
    while (std::getline(s, line)) {
      if (line[0] == '#') continue; // Skip comments
      if (line.size() < 1) {
        step++;
        continue;
      }

      std::istringstream drawStr(line);
      DrawElement        draw;

      drawStr >> draw.shapeIndex;

      glm::mat4& t = draw.transformation;
      for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
          drawStr >> t[x][y];
        }
      }
      switch (step) {
        case Step::Statics:
          elements.push_back(draw);
          // TODO: World bounds and bounding boxes
          break;
        case Step::Coins: pushCoin(draw, objects, triggers); break;
        case Step::Ghosts: pushGhost(draw, ghosts, triggers, shapes); break;
      }
    }

    EventService::Trigger(Events::event_map_loaded);
  }

  struct Selectable
  {
    BoundingBox boundingBox;
    int         id;
  };

  namespace gui {

    class Console
    {
    public:
      Console()
        : lines(255)
        , start(0)
        , end(0)
      {
      }

      virtual ~Console() {}

      void push(const std::string& text)
      {
        if (end >= lines.capacity()) end = 0;

        lines[end] = text;
        end++;
        if (end <= start) {
          start++;
          if (start >= lines.capacity()) start = 0;
        }
      }

      void render(void)
      {
        int size = lines.capacity();

        size_t current = start;
        while (size > 0) {
          if (current >= lines.capacity()) current = 0;
          if (current == end) return;

          ImGui::Text("> %s", lines[current].c_str());
          current++;
          size--;
        }
      }

    private:
      std::vector<std::string> lines;
      size_t                   start;
      size_t                   end;
    };

    struct ShapeElement
    {
      size_t      shapeIndex;
      std::string name;
    };

    enum class ObjectType
    {
      Static,
      Coin,
      Ghost
    };

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
            lineDebug += toString("\n>clipped=", clipped);
          }
          this->transformation = glm::translate(glm::mat4(), clipped);

          lineDebug += toString("\n>worldPosition=", orig);
          lineDebug += toString("\n>dir=", dir);

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

    static void parseMaze(World& world, std::vector<DrawElement>& elements,
                          std::istringstream& s, Frame& frame)
    {
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

            elements.push_back({0, transformation, DrawElement::getNextId()});
            world.statics.push_back(
              DrawCommand(*world.wallShape, transformation));

            BoundingBox box = world.wallShape->makeBoundingBox();
            box.transform(transformation);
            world.hardSurfaces.push_back(box);

            world.bounds.x = glm::max(world.bounds.x, box.getMax().x);
            world.bounds.z = glm::max(world.bounds.z, box.getMax().z);
          } else if (c == 'G') {
            const glm::mat4 transformation = glm::translate(scale, position);

            elements.push_back({3, transformation, DrawElement::getNextId()});
            world.statics.push_back(
              DrawCommand(*world.gateShape, transformation));

            BoundingBox box = world.wallShape->makeBoundingBox();
            box.transform(transformation);
            world.hardSurfaces.push_back(box);

            world.bounds.x = glm::max(world.bounds.x, box.getMax().x);
            world.bounds.z = glm::max(world.bounds.z, box.getMax().z);
          } else {
            if (c == 'l') {
              PointLight p;

              p.color    = glm::vec3(0.5f, 0.0f, 0.0f);
              p.position = glm::vec3(scale * glm::vec4(position, 1.0f));
              frame.pointLights.push_back(p);

              glm::mat4 transformation =
                glm::translate(glm::mat4(),
                               glm::vec3(position.x, -1.0f, position.z)) *
                glm::scale(glm::mat4(), glm::vec3(0.5f));
              world.statics.push_back(
                DrawCommand(*world.torchShape, transformation));
            } else if (c == 'g') {
              glm::mat4 transformation =
                glm::translate(glm::mat4(),
                               glm::vec3(position.x, -0.60f, position.z)) *
                glm::scale(glm::mat4(), glm::vec3(.3f));
              lateComer.push_back(
                DrawCommand(*world.ghostShape, transformation));
            } else if (c == 'p') {
              const glm::vec3 coinPosition =
                glm::vec3(position.x, -1.0f, position.z);
              const glm::mat4 transformation =
                glm::translate(scale, coinPosition);
              auto pos =
                std::find(std::begin(world.coinPickedUp),
                          std::end(world.coinPickedUp), transformation);
              if (pos == std::end(world.coinPickedUp)) {
                // TODO: Do put in a specific vector;
                world.statics.push_back(
                  DrawCommand(*world.purseShape, transformation));

                BoundingBox bbox(transformation, 0.3f);
                world.coinTriggers.push_back({bbox, transformation});
              }
            } else if (c == 'k') {
              const glm::vec3 keyPosition =
                glm::vec3(position.x, -1.0f, position.z);
              const glm::mat4 transformation =
                glm::scale(glm::translate(scale, keyPosition), glm::vec3(2.0f));

              world.statics.push_back(
                DrawCommand(*world.keyShape, transformation));
              world.theKey = transformation;
            } else if (c == 'b' || c == 'B') {
              const glm::vec3 barrelPosition =
                glm::vec3(position.x, -1.0f, position.z);
              const glm::mat4 transformation =
                glm::translate(scale, barrelPosition);

              BoundingBox box = world.barrelShape->makeBoundingBox();
              box.transform(transformation);
              world.hardSurfaces.push_back(box);
#if 0
	    if (c == 'b')
            world.statics.push_back(
              DrawCommand(*world.barrel2Shape, transformation));
	    else
#endif
              world.statics.push_back(
                DrawCommand(*world.barrelShape, transformation));
            } else if (c == 'A') {
              glm::mat4 transformation =
                glm::rotate(
                  glm::translate(glm::mat4(),
                                 glm::vec3(position.x, -0.60f, position.z)),
                  glm::half_pi<float>(), glm::vec3(0, 1, 0)) *
                glm::scale(glm::mat4(), glm::vec3(.3f));

              world.statics.push_back(
                DrawCommand(*world.ghostShape, transformation));
            }

            glm::mat4 groundTransformation =
              glm::translate(scale, glm::vec3(2.0f * x, -1.0f, 2.0f * z));

            glm::mat4 ceilTransformation =
              glm::translate(scale, glm::vec3(2.0f * x, 1.0f, 2.0f * z)) *
              glm::rotate(glm::mat4(), glm::pi<float>(),
                          glm::vec3(0.0f, 0.0f, 1.0f));

            elements.push_back(
              {2, groundTransformation, DrawElement::getNextId()});
            world.statics.push_back(
              DrawCommand(*world.floorShape, groundTransformation));
            world.statics.push_back(
              DrawCommand(*world.floorShape, ceilTransformation));
          }
          x += 1.0f;
        }
        z += 1.0f;
        x = 0.0f;
      }

      // TODO: In a future release move the ghost in a specific vector instead
      // of
      // using the static one. +1 is for the player ghost.
      world.statics.reserve(world.statics.size() + lateComer.size() + 1);

      for (size_t i = 0; i < lateComer.size(); ++i) {
        const auto& o = lateComer[i];

        world.statics.push_back(o);

        PointLight p;

        p.color     = gval::ghostColor;
        p.position  = glm::vec3(world.statics.back().transformation[3]);
        p.linear    = 0.09f;
        p.quadratic = 0.032f;
        frame.pointLights.push_back(p);

        world.sentinels.push_back(
          GhostData(&(world.statics.back().transformation),
                    frame.pointLights.size() - 1, glm::vec3(0, 0, 1)));
      }
    }

  } // gui

} // Soleil

static float                    TranslationSpeed = -1.0f;
static gui::CursorSelection     cursorSelection;
static gui::Console             console;
static std::vector<BoundingBox> debugBox;
static int                      selection = -1;

// TODO: regroup functions
static void
pick(const glm::vec3& orig, const glm::vec3& dir,
     std::vector<Selectable>& selectables)
{
  debugBox.clear();
  BoundingBox bbox;
  float       distance = std::numeric_limits<float>::max();

  for (const auto s : selectables) {
    float     t;
    glm::vec3 point;
    if (s.boundingBox.rayIntersect(orig, dir, t)) {
      console.push(
        toString("Intersection with ptr=", s.id, " at distance:", t));

      if (t < distance) {
        distance  = t;
        selection = s.id;
        bbox      = s.boundingBox;
      }
    }
  }

  bbox.expandBy(bbox.getMin() - 0.1f);
  bbox.expandBy(bbox.getMax() + 0.1f);
  debugBox.push_back(bbox);
}

static void
worldAsSelectable(std::vector<Selectable>&        selectables,
                  const std::vector<ShapePtr>&    shapes,
                  const std::vector<DrawElement>& elements,
                  const std::vector<DrawElement>& objects,
                  const std::vector<DrawElement>& ghosts)
{
  std::map<int, BoundingBox> prepared;
  selectables.clear();

  const auto items = {elements, objects, ghosts};

  for (const auto& item : items) {
    for (auto it = item.begin(); it != item.end(); ++it) {
      auto& element = *it;
      if (prepared.find(element.shapeIndex) == prepared.end()) {
        prepared[element.shapeIndex] =
          shapes[element.shapeIndex]->makeBoundingBox();
      }

      BoundingBox b = prepared[element.shapeIndex];
      b.transform(element.transformation);

      console.push(toString("> ", (void*)&element));
      selectables.push_back({b, element.id});
    }
  }
}

static void
worldBuilderDelete(int ptr, const std::vector<ShapePtr>& shapes,
                   std::vector<DrawElement>& elements,
                   std::vector<DrawElement>& objects,
                   std::vector<DrawElement>& ghosts)
{
  const auto items = {elements, objects, ghosts};

  for (const auto& item : items) {
    for (auto it = item.begin(); it != item.end(); ++it) {
      auto& element = *it;

      console.push(toString("+ ", (void*)&element));
      if (element.id == ptr) {
        elements.erase(it);
        return;
      }
    }
  }
  console.push(toString("No item found!! ", ptr));
}

static void
updateTranslation(glm::vec3& position, glm::vec3& center)
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

  static bool doRenderStatics  = true;
  static bool doRenderTriggers = false;

  ImVec4 clear_color = ImColor(114, 144, 154);
  int    width, height;
  glfwGetFramebufferSize(window, &width, &height);

  AssetService::Instance = std::make_shared<DesktopAssetService>("media/");
  SoundService::Instance = std::make_unique<DesktopSoundService>();
  OpenGLDataInstance::Initialize(); // TODO: Required by MTL - Should not be?
  editorResources                         = std::make_unique<EditorResources>();
  OpenGLDataInstance::Instance().viewport = glm::vec2(width, height);

  glm::mat4 projection = glm::perspective(
    glm::radians(50.0f), (float)width / (float)height, 0.1f, 50.0f);
  glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 8.0f, -8.0f), glm::vec3(0.0f),
                               glm::vec3(0, 1, 0));

  gui::Grid grid(20, 20);
  gui::Grid grid2(5, 5);
  gui::Grid grid3(5, 5);
  grid2.transformation =
    glm::rotate(glm::mat4(), -glm::half_pi<float>(), glm::vec3(1, 0, 0));
  grid3.transformation =
    glm::rotate(glm::mat4(), glm::half_pi<float>(), glm::vec3(0, 0, 1));

  std::vector<ShapePtr> shapes = {
    Soleil::WavefrontLoader::fromContent(
      AssetService::LoadAsString("wallcube.obj")),
    Soleil::WavefrontLoader::fromContent(
      AssetService::LoadAsString("barrel.obj")),
    Soleil::WavefrontLoader::fromContent(
      AssetService::LoadAsString("floor.obj")),
    Soleil::WavefrontLoader::fromContent(
      AssetService::LoadAsString("gate.obj")),
    Soleil::WavefrontLoader::fromContent(AssetService::LoadAsString("key.obj")),
    Soleil::WavefrontLoader::fromContent(
      AssetService::LoadAsString("coin.obj")),
    Soleil::WavefrontLoader::fromContent(
      AssetService::LoadAsString("ghost.obj"))};
  // All the shape that can be used in game. In a future release it could be
  // configured per level?

  std::vector<DrawElement> elements;
  // TODO: Should be named statics. Will have a fixed size

  std::vector<DrawElement> objects;
  // Coins, keys, ... size will varry

  std::vector<Trigger> triggers;

  std::vector<DrawElement> ghosts;

  std::vector<gui::ShapeElement> guiStaticsShapes = {
    {ShapeType::WallCube, "Wall cube"},
    {ShapeType::Barrel, "Barrel"},
    {ShapeType::Floor, "Floor"},
    {ShapeType::GateHeavy, "Gate heavy"},
  };

  std::vector<gui::ShapeElement> guiObjectsShapes = {
    {ShapeType::Coin, "Coin"}, {ShapeType::Ghost, "Ghost"},
  };

  Frame                          frame;
  static std::vector<Selectable> selectables;

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

  {
    Frame dummyFrame;

    EventService::Push(Events::event_map_loaded, [&]() {
      worldAsSelectable(selectables, shapes, elements, objects, ghosts);
      console.push(toString("selectable size: ", selectables.size()));
    });
    // TODO: ! Warning ! Also add any new objects

    InitializeWorldModels(world);
#if 0
    std::istringstream is(AssetService::LoadAsString("corpsdegarde.level.new"));
#else
    std::istringstream is(AssetService::LoadAsString("empty.new"));
#endif
    loadMap(elements, objects, ghosts, triggers, shapes, is);
  }

  while (!glfwWindowShouldClose(window)) {
    Soleil::Timer time((int)(glfwGetTime() * 1000));
    glfwPollEvents();

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

      cursorSelection.draw(shapes, frame);
      grid.draw(frame);
      grid2.draw(frame);
      grid3.draw(frame);
      // RenderFlatShape(statics, frame);
      // RenderFlatShape(world.statics, frame);
      if (doRenderStatics) {
        for (const auto& e : elements) {
          RenderFlatShape(e.transformation, *shapes[e.shapeIndex], frame);
        }
        for (const auto& e : objects) {
          RenderFlatShape(e.transformation, *shapes[e.shapeIndex], frame);
        }
        for (const auto& e : ghosts) {
          RenderFlatShape(e.transformation, *shapes[e.shapeIndex], frame);
        }
      }
      if (doRenderTriggers) {
        for (const auto& t : triggers) {
          DrawBoundingBox(t.aoe, frame);
        }
      }

      for (const auto b : debugBox) {
        DrawBoundingBox(b, frame);
      }
    }

    ImGui_ImplGlfwGL3_NewFrame();

    // Update Eye z;
    const ImGuiIO& io = ImGui::GetIO();
    eye.y -= io.MouseWheel;

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

    if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered()) {
      // Check if we have to add an entity
      if (cursorSelection.isActive()) {
        DrawElement draw{cursorSelection.getShape(),
                         cursorSelection.getTransformation(),
                         DrawElement::getNextId()};
        switch (cursorSelection.getShape()) {
          case ShapeType::WallCube:
          case ShapeType::Barrel:
          case ShapeType::Floor:
          case ShapeType::GateHeavy: elements.push_back(draw); break;
          case ShapeType::Coin: pushCoin(draw, objects, triggers); break;
          case ShapeType::Ghost:
            pushGhost(draw, ghosts, triggers, shapes);
            break;
        }
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
    static char fileName[255] = "corpsdegarde.level";
    ImGui::InputText("input text", fileName, 255);
    if (ImGui::Button("Save")) {
      std::ofstream outfile("media/" + std::string(fileName) + ".new");

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

      const auto view = {elements, objects, ghosts};

      for (const auto& current : view) {
        for (const auto& e : current) {
          outfile << e.shapeIndex << " ";

          const glm::mat4& t = e.transformation;
          for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
              outfile << t[x][y] << " ";
            }
          }
          outfile << "\n";
        }

        outfile << "\n";
      }

      /*
Here I've multiple choices: What I save into the map ?
- indices of Shape?
- Directly vertices as world?
       */
    }

    if (ImGui::Button("Load Old")) {
      Frame dummyFrame;

      InitializeWorldModels(world);
      std::istringstream is(AssetService::LoadAsString(fileName));
      gui::parseMaze(world, elements, is, dummyFrame);
    }

    if (ImGui::Button("Load New")) {
      Frame dummyFrame;

      InitializeWorldModels(world);
      std::istringstream is(AssetService::LoadAsString(fileName));
      loadMap(elements, objects, ghosts, triggers, shapes, is);
    }

    if (ImGui::Button("New")) {
      Frame dummyFrame;

      InitializeWorldModels(world);
      newMap(elements, objects, ghosts, triggers);
    }

    ImGui::End();

    if (selection >= 0) {
      ImGui::Begin("Selection");
      if (ImGui::Button("Delete")) {
        worldBuilderDelete(selection, shapes, elements, objects, ghosts);
      }
      ImGui::End();
    }

    // 1. Show a simple window
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in
    // a window automatically called "Debug"
    {
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::Text(">%s", lineDebug.c_str());
      console.render();
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
