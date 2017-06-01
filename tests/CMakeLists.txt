cmake_minimum_required(VERSION 3.5)

project(ruine)

add_executable(sceneGraphTest

  SceneGraphTest.cpp
  )

target_link_libraries(sceneGraphTest

  ruinelib)


add_executable(checkElementGain

  CheckElementGain.cpp
  )
