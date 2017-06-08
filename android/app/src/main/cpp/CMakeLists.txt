cmake_minimum_required(VERSION 3.5)

set (RUINE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)

add_library(app-glue STATIC
  ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories (SYSTEM
  ${RUINE_SOURCES}/glm/
  )

add_library(ruine-activity SHARED
  main.cpp
  ${RUINE_SOURCES}/AndroidEngine.cpp
  ${RUINE_SOURCES}/AndroidGLESContext.cpp
  ${RUINE_SOURCES}/AndroidAssetService.cpp
  ${RUINE_SOURCES}/AndroidSoundService.cpp

  ${RUINE_SOURCES}/Ruine.cpp  
  ${RUINE_SOURCES}/Logger.cpp
  ${RUINE_SOURCES}/TypesToOStream.cpp
  ${RUINE_SOURCES}/OpenGLInclude.cpp
  ${RUINE_SOURCES}/AssetService.cpp
  ${RUINE_SOURCES}/SoundService.cpp
  ${RUINE_SOURCES}/Object.cpp
  ${RUINE_SOURCES}/Node.cpp
  ${RUINE_SOURCES}/Shader.cpp
  ${RUINE_SOURCES}/Shape.cpp
  ${RUINE_SOURCES}/Drawable.cpp
  ${RUINE_SOURCES}/Program.cpp
  ${RUINE_SOURCES}/WavefrontLoader.cpp
  ${RUINE_SOURCES}/MTLLoader.cpp
  ${RUINE_SOURCES}/Group.cpp
  ${RUINE_SOURCES}/OpenGLDataInstance.cpp
  )

target_include_directories(ruine-activity PRIVATE
  ${ANDROID_NDK}/sources/android/native_app_glue
  ${RUINE_SOURCES}
  )


target_link_libraries(ruine-activity
  android
  app-glue
  EGL
  GLESv2
  OpenSLES
  log
  )
