cmake_minimum_required(VERSION 3.5)

project(ruine)


#####################################################################
# Coverage build						    #
# ==============						    #
# mkdir buildcov						    #
# cd buildcov							    #
# cmake ../ # make sure to use GCC				    #
# make								    #
# ./coverage/coveragebin # Or play tests			    #
# lcov -c -d ./coverage/CMakeFiles/coveragebin.dir/__/ -o test.lcov #
# genhtml test.lcov						    #
# firefox index.html						    #
#####################################################################


set(CMAKE_CXX_FLAGS "-std=c++14 -W -Wall -Wextra -g -O0 -Wno-system-headers -fprofile-arcs -ftest-coverage  -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined")
set(CMAKE_CFLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined")
set(CMAKE_EXE_LINKER_FLAGS="-fprofile-arcs -ftest-coverage")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined")

add_executable(coveragebin
  ../DesktopAssetService.cpp
  ../DesktopSoundService.cpp

  ../Ruine.cpp  
  ../Logger.cpp
  ../TypesToOStream.cpp
  ../OpenGLInclude.cpp
  ../AssetService.cpp
  ../SoundService.cpp
  ../Object.cpp
  ../Node.cpp
  ../Shader.cpp
  ../Shape.cpp
  ../Program.cpp
  ../WavefrontLoader.cpp
  ../MTLLoader.cpp
  ../Group.cpp
  ../OpenGLDataInstance.cpp
  ../Draw.cpp
  ../World.cpp
  ../Text.cpp
  ../Recorder.cpp

  
  ../main.cpp
  )

target_link_libraries(coveragebin
  ${GLFW}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIB}
  )
