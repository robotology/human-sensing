#=============================================================================
# Copyright 2017  iCub Facility, Istituto Italiano di Tecnologia
#   Authors: Daniele E. Domenichelli <daniele.domenichelli@iit.it>
#            Vadim Tikhanoff <vadim.tikhanoff@iit.it>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of YCM, substitute the full
#  License text for the above reference.)

find_library(openpose_LIBRARY
             NAMES openpose
             PATH_SUFFIXES lib
                           build/lib
			   build/src/openpose
             PATHS /usr/
                   /usr/local/
                   ${openpose_ROOT_DIR}
                   ENV openpose_ROOT)

find_library(openpose_core_LIBRARY
             NAMES openpose_core
             PATH_SUFFIXES lib
                           build/lib
						   build/src/openpose/core
             PATHS /usr/
                   /usr/local/
                   ${openpose_ROOT_DIR}
                   ENV openpose_ROOT)


find_library(openpose_pose_LIBRARY
             NAMES openpose_pose
             PATH_SUFFIXES lib
                           build/lib
						   build/src/openpose/pose
             PATHS /usr/
                   /usr/local/
                   ${openpose_ROOT_DIR}
                   ENV openpose_ROOT)

find_library(openpose_face_LIBRARY
             NAMES openpose_face
             PATH_SUFFIXES lib
                           build/lib
						   build/src/openpose/face
             PATHS /usr/
                   /usr/local/
                   ${openpose_ROOT_DIR}
                   ENV openpose_ROOT)

find_library(openpose_hand_LIBRARY
             NAMES openpose_hand
             PATH_SUFFIXES lib
                           build/lib
						   build/src/openpose/hand
             PATHS /usr/
                   /usr/local/
                   ${openpose_ROOT_DIR}
                   ENV openpose_ROOT)

find_library(openpose_hand_LIBRARY
             NAMES openpose_hand
             PATH_SUFFIXES lib
                           build/lib
						   build/src/openpose/hand
             PATHS /usr/
                   /usr/local/
                   ${openpose_ROOT_DIR}
                   ENV openpose_ROOT)

find_library(openpose_producer_LIBRARY
             NAMES openpose_producer
             PATH_SUFFIXES lib
                           build/lib
						   build/src/openpose/producer
             PATHS /usr/
                   /usr/local/
                   ${openpose_ROOT_DIR}
                   ENV openpose_ROOT)

find_library(openpose_thread_LIBRARY
             NAMES openpose_thread
             PATH_SUFFIXES lib
                           build/lib
						   build/src/openpose/thread
             PATHS /usr/
                   /usr/local/
                   ${openpose_ROOT_DIR}
                   ENV openpose_ROOT)

find_library(openpose_utilities_LIBRARY
             NAMES openpose_utilities
             PATH_SUFFIXES lib
                           build/lib
						   build/src/openpose/utilities
             PATHS /usr/
                   /usr/local/
                   ${openpose_ROOT_DIR}
                   ENV openpose_ROOT)

find_library(openpose_wrapper_LIBRARY
             NAMES openpose_wrapper
             PATH_SUFFIXES lib
                           build/lib
						   build/src/openpose/wrapper
             PATHS /usr/
                   /usr/local/
                   ${openpose_ROOT_DIR}
                   ENV openpose_ROOT)




find_path(openpose_INCLUDE_DIR
          NAMES openpose/headers.hpp
          PATH_SUFFIXES include
          PATHS /usr/
                /usr/local/
                ${openpose_ROOT_DIR}
                ENV openpose_ROOT)

set(openpose_LIBRARIES ${openpose_LIBRARY} 
			   ${openpose_core_LIBRARY}
			   ${openpose_pose_LIBRARY}
			   ${openpose_face_LIBRARY}
			   ${openpose_hand_LIBRARY}
			   ${openpose_producer_LIBRARY}
			   ${openpose_thread_LIBRARY}
			   ${openpose_utilities_LIBRARY}
			   ${openpose_wrapper_LIBRARY})

set(openpose_INCLUDE_DIRS ${openpose_INCLUDE_DIR})

message("openpose_LIBRARIES = ${openpose_LIBRARIES}")
message("openpose_INCLUDE_DIRS = ${openpose_INCLUDE_DIRS}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(openpose
                                  REQUIRED_VARS openpose_LIBRARIES
                                                openpose_INCLUDE_DIRS)

# Set package properties if FeatureSummary was included
if(COMMAND set_package_properties)
    set_package_properties(openpose PROPERTIES DESCRIPTION "openpose"
                                               URL "http://www.openpose.org/")
endif()
