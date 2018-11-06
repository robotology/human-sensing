# Copyright: 2016 iCub Facility, Istituto Italiano di Tecnologia
# Author: Vadim Tikhanoff
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
#
cmake_minimum_required(VERSION 3.5)
project(faceLandmarks)

find_package(YARP 3.1 REQUIRED COMPONENTS OS sig dev math rosmsg)
find_package(ICUBcontrib REQUIRED)
find_package(dlib REQUIRED)
find_package(OpenCV REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})

#adding c++11 flags
set(CMAKE_CXX_FLAGS "-std=c++11")

include(ICUBcontribHelpers)
include(ICUBcontribOptions)

icubcontrib_set_default_prefix()

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
                    ${OpenCV_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${source} ${header} ${doc} ${idl} ${IDL_GEN_FILES})

target_link_libraries(${PROJECT_NAME} dlib::dlib ${YARP_LIBRARIES} ${OpenCV_LIBRARIES}  )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
yarp_install(FILES ${doc} DESTINATION ${ICUBCONTRIB_MODULES_INSTALL_DIR})

add_subdirectory(app)

option(DOWNLOAD_FACE_LANDMARKS_DAT "download face landmarks dataset" OFF)
set(BZIP2FILE shape_predictor_68_face_landmarks.dat)

if (DOWNLOAD_FACE_LANDMARKS_DAT)
  if (NOT EXISTS ${CMAKE_SOURCE_DIR}/${BZIP2FILE}.bz2 AND NOT EXISTS ${CMAKE_BINARY_DIR}/${BZIP2FILE})
    file(DOWNLOAD http://dlib.net/files/${BZIP2FILE}.bz2 ${CMAKE_BINARY_DIR}/${BZIP2FILE}.bz2 STATUS status LOG log SHOW_PROGRESS)
    find_program(BZIP2_EXECUTABLE bzip2)
    if (NOT BZIP2_EXECUTABLE)
      message(WARNING "BZIP2 Not found on you system, please extract ${CMAKE_BINARY_DIR}/${BZIP2FILE}.bz2 manually")
      message(WARNING "Extract it in $ICUBcontrib_DIR/share/ICUBcontrib/contexts/faceLandmarks")
    else()
      execute_process(COMMAND ${BZIP2_EXECUTABLE} -d ${CMAKE_BINARY_DIR}/${BZIP2FILE}.bz2)
    endif()
  endif()
endif()

if (EXISTS ${CMAKE_BINARY_DIR}/${BZIP2FILE})
  yarp_install( FILES ${CMAKE_BINARY_DIR}/${BZIP2FILE} DESTINATION ${ICUBCONTRIB_CONTEXTS_INSTALL_DIR}/${PROJECT_NAME} )
else()
  message(WARNING "no ${BZIP2FILE} file has been found please turn ON the DOWNLOAD_FACE_LANDMARKS_DAT flag or download the file manually before running cmake")
endif()
