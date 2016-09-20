
cmake_minimum_required(VERSION 2.6)

project(human-sensing)

find_package(ICUBcontrib REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})
include(ICUBcontribOptions)
include(ICUBcontribHelpers)

add_subdirectory(faceLandmarks)

icubcontrib_add_uninstall_target()

