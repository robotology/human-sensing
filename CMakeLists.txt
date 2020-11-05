
cmake_minimum_required(VERSION 2.6)

project(human-sensing)

find_package(ICUBcontrib REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})
include(ICUBcontribOptions)
include(ICUBcontribHelpers)


option(BUILD_FACELANDMARKS "enable the build of Face Landmarks" OFF)
if(BUILD_FACELANDMARKS)
    add_subdirectory(faceLandmarks)
endif()

option(BUILD_GOOGLE_VISION_AI "enable the build of Google Vision AI" OFF)
if(BUILD_GOOGLE_VISION_AI)
  add_subdirectory(googleVisionAI)
endif()

icubcontrib_add_uninstall_target()

