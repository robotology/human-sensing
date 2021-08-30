
cmake_minimum_required(VERSION 3.5)

project(human-sensing)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED 11)

option(BUILD_GOOGLE_VISION_AI "enable the build of Google Vision AI" OFF)
option(BUILD_FACELANDMARKS "enable the build of Face Landmarks" OFF)
option(BUILD_YARPOPENPOSE "enable the build of yarp Open Pose" OFF)

find_package(ICUBcontrib REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})
include(ICUBcontribOptions)
include(ICUBcontribHelpers)

icubcontrib_set_default_prefix()

if(BUILD_FACELANDMARKS)
    add_subdirectory(faceLandmarks)
endif()

if(BUILD_GOOGLE_VISION_AI)
  add_subdirectory(googleVisionAI)
endif()

if(BUILD_YARP_OPEN_POSE)
  add_subdirectory(yarpOpenPose)
endif()

icubcontrib_add_uninstall_target()
