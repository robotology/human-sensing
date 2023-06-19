cmake_minimum_required(VERSION 3.5)
project(yarpRTMPose LANGUAGES CXX VERSION 0.2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(YARP 3.2.2 REQUIRED COMPONENTS os sig dev math cv)
find_package(ICUBcontrib REQUIRED)
find_package(MMDeploy REQUIRED)
find_package(OpenCV REQUIRED)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/c++/include)
set(THIRD_PARTY ${CMAKE_CURRENT_SOURCE_DIR}/src/c++/thirdparty)

add_subdirectory(src/c++)
add_subdirectory(tests)

file(GLOB conf ${CMAKE_CURRENT_SOURCE_DIR}/app/conf/yarpRTMPose.ini)
file(GLOB grabber_conf ${CMAKE_CURRENT_SOURCE_DIR}/app/conf/grabber.ini)
file(GLOB data ${CMAKE_CURRENT_SOURCE_DIR}/data/coco_wholebody.json)

yarp_install(FILES ${conf} DESTINATION ${YARP_CONTEXTS_INSTALL_DIR}/yarpRTMPose)
yarp_install(FILES ${grabber_conf} DESTINATION ${YARP_CONTEXTS_INSTALL_DIR}/yarpRTMPose)
yarp_install(FILES ${data} DESTINATION ${YARP_CONTEXTS_INSTALL_DIR}/yarpRTMPose)