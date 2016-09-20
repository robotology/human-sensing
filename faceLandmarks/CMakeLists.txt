# Copyright: 2016 iCub Facility, Istituto Italiano di Tecnologia
# Author: Vadim Tikhanoff
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
# 

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9)
project(faceLandmarks)

find_package(YARP)
find_package(ICUB)
find_package(ICUBcontrib)
#find_package(X11)

#if(NOT X11_FOUND)
#    message(FATAL_ERROR "Failed to find X11 which is required to build dlib")
#endif(NOT X11_FOUND)

list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ICUB_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})

find_package(OpenCV)

#this needs changing and automatically set DLIB_DIR
set(DLIB_DIR "/usr/local/src/robot/dlib-19.1/" CACHE PATH "Path to DLIB root directory")

include(${DLIB_DIR}/dlib/cmake)

include(YarpInstallationHelpers)
include(ICUBcontribHelpers)
include(ICUBcontribOptions)
icubcontrib_set_default_prefix()

set(DLIB_SRC ${DLIB_DIR}/dlib/all/source.cpp)
set(DLIB_LIB ${DLIB_DIR}/dlib/build/libdlib.a)

#SET(EXTRA_LIBS ${X11_LIBRARIES} ${DLIB_LIB})
#SET(EXTRA_LIBS ${DLIB_LIB})

include(YarpIDL)
yarp_add_idl(IDL_GEN_FILES ${PROJECT_NAME}.thrift)

file(GLOB source src/*.cpp)
file(GLOB header include/*.h)
set(idl ${PROJECT_NAME}.thrift)
set(doc ${PROJECT_NAME}.xml)

source_group("Source Files" FILES ${source})
source_group("Header Files" FILES ${header})
source_group("Source Files" FILES ${DLIB_SRC})
source_group("IDL Files" FILES ${idl})
source_group("DOC Files" FILES ${doc})

include_directories(${PROJECT_SOURCE_DIR}/include
                    ${OpenCV_INCLUDE_DIRS}
                    ${YARP_INCLUDE_DIRS}
                    ${ICUB_INCLUDE_DIRS}
                    )

add_executable(${PROJECT_NAME} ${source} ${header} ${doc} ${DLIB_SRC} ${idl} ${IDL_GEN_FILES})

target_link_libraries(${PROJECT_NAME} dlib ${YARP_LIBRARIES} ${OpenCV_LIBRARIES}  )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
yarp_install(FILES ${doc} DESTINATION ${ICUBCONTRIB_MODULES_INSTALL_DIR})

add_subdirectory(app)
