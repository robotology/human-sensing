# Copyright: 2017 iCub Facility, Istituto Italiano di Tecnologia
# Author: Vadim Tikhanoff
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
#
cmake_minimum_required(VERSION 3.0)
project(yarpOpenPose)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
message("CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

include(YarpInstallationHelpers)
include(ICUBcontribHelpers)
include(ICUBcontribOptions)
icubcontrib_set_default_prefix()

find_package(openpose REQUIRED)
include_directories(${openpose_INCLUDE_DIRS})

find_package(YARP REQUIRED)
include_directories(${YARP_INCLUDE_DIRS})

find_package(OpenCV 3 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})

find_package(Caffe REQUIRED)
include_directories(${Caffe_INCLUDE_DIRS})

add_definitions(-DUSE_CAFFE)

message("YARP_LIBRARIES = ${YARP_LIBRARIES}")
foreach(target ${YARP_LIBRARIES})
    get_property(x TARGET ${target} PROPERTY INTERFACE_LINK_LIBRARIES)
    message("    ${target} -> ${x}")
endforeach()
message("openpose_LIBRARIES = ${openpose_LIBRARIES}")
message("Caffe_LIBRARIES = ${Caffe_LIBRARIES}")

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} ${YARP_LIBRARIES} ${openpose_LIBRARIES} ${Caffe_LIBRARIES} ${OpenCV_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
yarp_install(FILES ${doc} DESTINATION ${ICUBCONTRIB_MODULES_INSTALL_DIR})

add_subdirectory(app)
