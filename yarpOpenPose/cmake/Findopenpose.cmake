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

find_library(openpose_openpose_LIBRARY
             NAMES openpose
             PATH_SUFFIXES lib
                           build/lib
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

set(openpose_LIBRARIES ${openpose_openpose_LIBRARY})
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
