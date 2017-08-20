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

#ifndef SOLEIL__EDITOR_HPP_
#define SOLEIL__EDITOR_HPP_

#include "BoundingBox.hpp"
#include "Draw.hpp"
#include "OpenGLInclude.hpp"
#include "Program.hpp"
#include "World.hpp"

namespace Soleil {

  namespace gui {

    // A structure to maintain all the OpenGL resources
    struct EditorResources
    {
      Program gridProgram;
      GLuint  gridMVP;

      EditorResources();
    };

    // Print console log onto the screen
    class Console
    {
    public:
      Console();
      virtual ~Console();

    public:
      void push(const std::string& text);
      void render(void);

    private:
      std::vector<std::string> lines;
      size_t                   start;
      size_t                   end;
    };

    // Graphical elements that can be placed by the editor
    struct ShapeElement
    {
      size_t      shapeIndex;
      std::string name;
    };

    // Different type of object the user can interact with. Represent the global
    // scheme. Static is all the decor that cannot move
    enum class ObjectType
    {
      Static,
      Coin,
      Ghost
    };

    // An enum to represent all event the editor may trigger
    enum Events
    {
      event_map_loaded
    };

    // Any node of any type that can be selectable
    struct Selectable
    {
      BoundingBox boundingBox;
      std::size_t id;
      std::size_t type;
    };

    // --- Method for pushing elements on the world
    void pushCoin(DrawElement& draw, std::vector<DrawElement>& objects,
                  std::vector<Trigger>& triggers);

    void pushGhost(DrawElement& draw, std::vector<DrawElement>& objects,
                   std::vector<Trigger>&        triggers,
                   const std::vector<ShapePtr>& shapes);
    void newMap(std::vector<DrawElement>& elements,
                std::vector<DrawElement>& objects,
                std::vector<DrawElement>& ghosts,
                std::vector<Trigger>&     triggers);
    void loadMap(World& world, const std::string &fileName);

    // Method to load legacy levels
    void parseMaze(World& world, std::vector<DrawElement>& elements,
                   std::istringstream& s, Frame& frame);

    // --- Pick Select Delete Move
    void pick(const glm::vec3& orig, const glm::vec3& dir,
              std::vector<Selectable>& selectables);
    void worldBuilderDelete(std::size_t                  ptr,
                            const std::vector<ShapePtr>& shapes,
                            std::vector<DrawElement>&    elements,
                            std::vector<DrawElement>&    objects,
                            std::vector<DrawElement>&    ghosts);
    void updateTranslation(glm::vec3& position, glm::vec3& center);

    // Transform nodes of the current world into an array of selectables
    void worldAsSelectable(std::vector<Selectable>&        selectables,
                           const World& world);
    void clearSelection();
    
  } // gui
} // Soleil

#endif /* SOLEIL__EDITOR_HPP_ */
