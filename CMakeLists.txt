cmake_minimum_required (VERSION 3.5)

project(ruine)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_library(GLEW_LIB GLEW)
find_package(X11 REQUIRED)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g -rdynamic -W -Wall -Wextra")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


include_directories (SYSTEM
  ${SDL2_INCLUDE_DIR}  
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
  Object.cpp
  Node.cpp
  Shader.cpp
  Shape.cpp
  Drawable.cpp
  )

add_executable(ruine
  main.cpp
  )


target_link_libraries(ruine
  ruinelib
  ${SDL2_LIBRARY}
  # ${SDL2IMAGE_LIBRARIES}
  # ${SDL2Mixer_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIB}
  )

add_custom_command(TARGET ruine PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/media" "$<TARGET_FILE_DIR:ruine>/media"
  )


enable_testing()
add_subdirectory(tests)

add_test(SceneGraphTest tests/sceneGraphTest)


add_custom_command(TARGET ruine POST_BUILD
  COMMAND ctest -V -C $<CONFIGURATION> --output-on-failure)

