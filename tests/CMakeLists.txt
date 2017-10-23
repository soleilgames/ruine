cmake_minimum_required(VERSION 3.5)

project(ruine)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -W -Wall -Wextra")

add_executable(sceneGraphTest SceneGraphTest.cpp)
target_link_libraries(sceneGraphTest ruinelib)

add_executable(wavefrontTest WavefrontTest.cpp)
target_link_libraries(wavefrontTest ruinelib
  ${GLFW}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIB}

  #used in benchmark
  benchmark pthread
  )



add_executable(checkElementGain CheckElementGain.cpp)
