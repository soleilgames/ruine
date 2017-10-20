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

#include "mcut.hpp"

#include "WavefrontLoader.hpp"

#include "ControllerService.hpp"
#include "DesktopAssetService.hpp"
#include "DesktopSoundService.hpp"
#include "OpenGLDataInstance.hpp"
#include "OpenGLInclude.hpp"

#define SOLEIL__DO_BENCHMARK 1

#if SOLEIL__DO_BENCHMARK
// Benchmark of the Wavefront loader refactoring
#include <benchmark/benchmark.h>
#endif

using namespace Soleil;

void
fullLoad()
{
  const std::string content =
    "# Blender v2.76 (sub 0) OBJ File: 'entities.blend'"
    "# www.blender.org"
    "mtllib wallcube.mtl"
    "o Cube.001"
    "v 1.000000 -0.000418 1.000000"
    "v -1.000000 -0.000418 1.000000"
    "v -1.000000 -0.000418 -1.000000"
    "v -1.000000 1.999582 -1.000000"
    "v -1.000000 1.999582 1.000000"
    "v 0.999999 1.999582 1.000001"
    "v 1.000000 1.999582 -0.999999"
    "v 1.000000 -0.000418 -1.000000"
    "vt 1.001884 0.998109"
    "vt 0.001891 1.001884"
    "vt -0.001884 0.001891"
    "vt 0.998109 -0.001884"
    "vn 0.000000 -1.000000 -0.000000"
    "vn -0.000000 1.000000 0.000000"
    "vn 1.000000 0.000000 0.000000"
    "vn -0.000000 -0.000000 1.000000"
    "vn -1.000000 0.000000 0.000000"
    "vn 0.000000 0.000000 -1.000000"
    "usemtl Material.001"
    "s 1"
    "f 1/1/1 2/2/1 3/3/1"
    "f 4/1/2 5/2/2 6/3/2"
    "f 7/1/3 6/2/3 1/3/3"
    "f 6/1/4 5/2/4 2/3/4"
    "f 2/4/5 5/1/5 4/2/5"
    "f 8/3/6 3/4/6 4/1/6"
    "f 8/4/1 1/1/1 3/3/1"
    "f 7/4/2 4/1/2 6/3/2"
    "f 8/4/3 7/1/3 1/3/3"
    "f 1/4/4 6/1/4 2/3/4"
    "f 3/3/5 2/4/5 4/2/5"
    "f 7/2/6 8/3/6 4/1/6";

  std::shared_ptr<Shape> shape = WavefrontLoader::fromContent(content);
  // TODO: Cannot do that test yet, need to create an OpenGL Context
}

static void
errorCallback(int error, const char* description)
{
  std::cerr << "GLFW failed with error N." << error << ": " << description;
}

static void
BM_WavefrontLoading(benchmark::State& state)
{
  while (state.KeepRunning()) {
    std::shared_ptr<Shape> shape =
      WavefrontLoader::fromContent(AssetService::LoadAsString("barrel.obj"));
  }
}
// Register the function as a benchmark
BENCHMARK(BM_WavefrontLoading);

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

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glewExperimental = GL_TRUE;
  GLenum err       = glewInit();
  if (err != GLEW_OK) {
    throw std::runtime_error(
      toString("Unable to initialize GLEW: ", glewGetErrorString(err)));
  }

  AssetService::Instance = std::make_shared<DesktopAssetService>("../media/");
  SoundService::Instance = std::make_unique<DesktopSoundService>();
  OpenGLDataInstance::instance = std::make_unique<OpenGLDataInstance>();

  mcut::TestSuite basics("TDD");
  basics.add(fullLoad);
  basics.run();

#if SOLEIL__DO_BENCHMARK
  ::benchmark::Initialize(&argc, argv);
  ::benchmark::RunSpecifiedBenchmarks();
#endif

  glfwTerminate();

  return 0;
}
