cmake_minimum_required(VERSION 3.5)

set (RUINE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../../../../..)

add_library(app-glue STATIC
  ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_library(ruine-activity SHARED
  main.cpp
  ${RUINE_SOURCES}/AndroidEngine.cpp
  ${RUINE_SOURCES}/AndroidGLESContext.cpp
  ${RUINE_SOURCES}/AndroidAssetService.cpp

  ${RUINE_SOURCES}/Ruine.cpp
  ${RUINE_SOURCES}/Logger.cpp
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
