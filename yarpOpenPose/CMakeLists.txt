# Copyright: 2019 iCub Facility, Istituto Italiano di Tecnologia
# Author: Vadim Tikhanoff
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
#
cmake_minimum_required(VERSION 3.5)
project(yarpOpenPose LANGUAGES CXX C VERSION 1.0.0)

find_package(ICUBcontrib REQUIRED)
find_package(YARP 3.2.2 REQUIRED COMPONENTS os sig dev math cv)

include(GNUInstallDirs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})
message("CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

include(ICUBcontribOptions)
include(ICUBcontribHelpers)

if(NOT CMAKE_CONFIGURATION_TYPES)
  if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
  endif()
endif()

icubcontrib_set_default_prefix()

find_package(openpose REQUIRED)
find_package(OpenCV REQUIRED)
include_directories(${openpose_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})

message("YARP_LIBRARIES = ${YARP_LIBRARIES}")
foreach(target ${YARP_LIBRARIES})
    get_property(x TARGET ${target} PROPERTY INTERFACE_LINK_LIBRARIES)
    message("    ${target} -> ${x}")
endforeach()
message("openpose_LIBRARIES = ${openpose_LIBRARIES}")

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} ${openpose_LIBRARIES} ${OpenCV_LIBRARIES} ${YARP_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
#yarp_install(FILES ${doc} DESTINATION ${ICUBCONTRIB_MODULES_INSTALL_DIR})

add_subdirectory(app)
