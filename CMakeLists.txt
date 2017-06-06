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
  AssetService.cpp
  SoundService.cpp
  Object.cpp
  Node.cpp
  Shader.cpp
  Shape.cpp
  Drawable.cpp
  Program.cpp
  WavefrontLoader.cpp
  MTLLoader.cpp
  Group.cpp
  OpenGLDataInstance.cpp
  )


enable_testing()
add_subdirectory(tests)

add_test(SceneGraphTest tests/sceneGraphTest)
add_test(WavefrontTest tests/wavefrontTest)



add_executable(ruine
  main.cpp
  )


set(RUINELIBS
  ruinelib
  ${SDL2_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIB}
  )

target_link_libraries(ruine
  ${RUINELIBS}
  )

add_custom_command(TARGET ruine PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/media" "$<TARGET_FILE_DIR:ruine>/media"
  )


add_custom_command(TARGET ruine POST_BUILD
  COMMAND ctest -V -C $<CONFIGURATION> --output-on-failure)

