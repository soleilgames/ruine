cmake_minimum_required (VERSION 3.5)

project(ruine)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")
find_package(OpenGL REQUIRED)
find_library(GLEW_LIB GLEW)
find_package(X11 REQUIRED)
find_library(GLFW "glfw" REQUIRED)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -rdynamic -W -Wall -Wextra")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



include_directories (SYSTEM
  ${OPENGL_INCLUDE_DIR}
  glm/
  )

include_directories(
  ./
  )



add_library(ruinelib
  DesktopAssetService.cpp
  DesktopSoundService.cpp
  
  Ruine.cpp  
  Logger.cpp
  TypesToOStream.cpp
  OpenGLInclude.cpp
  AssetService.cpp
  SoundService.cpp
  Object.cpp
  Node.cpp
  Shader.cpp
  Shape.cpp
  Program.cpp
  WavefrontLoader.cpp
  MTLLoader.cpp
  Group.cpp
  OpenGLDataInstance.cpp
  Draw.cpp
  World.cpp
  Text.cpp
  )


enable_testing()
add_subdirectory(tests)

add_test(SceneGraphTest tests/sceneGraphTest)
add_test(WavefrontTest tests/wavefrontTest)

set(RUINELIBS
  ruinelib
  ${GLFW}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIB}
  )

add_executable(ruine
  main.cpp
  )



target_link_libraries(ruine
  ${RUINELIBS}
  )

add_custom_command(TARGET ruine PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/media" "$<TARGET_FILE_DIR:ruine>/media"
  )


add_custom_command(TARGET ruine POST_BUILD
  COMMAND ctest -V -C $<CONFIGURATION> --output-on-failure)


add_executable(reditor
  editor.cpp
  imgui_impl_glfw_gl3.cpp
  imgui.cpp
  imgui_draw.cpp
  )

add_custom_command(TARGET reditor PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/media" "$<TARGET_FILE_DIR:ruine>/media"
  )


target_link_libraries(reditor
  ${RUINELIBS}
  )
