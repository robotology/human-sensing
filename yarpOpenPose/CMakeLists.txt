# Copyright: 2017 iCub Facility, Istituto Italiano di Tecnologia
# Author: Vadim Tikhanoff
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
#
cmake_minimum_required(VERSION 3.5)
project(yarpOpenPose)

find_package(YARP 3.1 REQUIRED COMPONENTS OS sig dev math)
find_package(ICUB REQUIRED)
find_package(ICUBcontrib REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})
message("CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

include(ICUBcontribOptions)
include(ICUBcontribHelpers)

icubcontrib_set_default_prefix()

find_package(openpose REQUIRED)
find_package(Caffe REQUIRED)
find_package(OpenCV 3 REQUIRED)
find_package(CUDA)

if( CUDA_FOUND )
    message(WARNING "Found CUDA, will set GPU version")
else()
    message(WARNING "Did not find CUDA, will set CPU version")
endif()

include_directories(${openpose_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${Caffe_INCLUDE_DIRS})
if (CUDA_FOUND)
    include_directories(${CUDA_INCLUDE_DIRS})
endif()

add_definitions(-DUSE_CAFFE)

message("YARP_LIBRARIES = ${YARP_LIBRARIES}")
message("Caffe_INCLUDE_DIRS = ${Caffe_INCLUDE_DIRS}")
foreach(target ${YARP_LIBRARIES})
    get_property(x TARGET ${target} PROPERTY INTERFACE_LINK_LIBRARIES)
    message("    ${target} -> ${x}")
endforeach()
message("openpose_LIBRARIES = ${openpose_LIBRARIES}")
message("Caffe_LIBRARIES = ${Caffe_LIBRARIES}")

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} ${YARP_LIBRARIES} ${openpose_LIBRARIES} ${Caffe_LIBRARIES} ${OpenCV_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
#yarp_install(FILES ${doc} DESTINATION ${ICUBCONTRIB_MODULES_INSTALL_DIR})

add_subdirectory(app)
