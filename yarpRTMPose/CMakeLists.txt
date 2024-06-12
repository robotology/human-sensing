cmake_minimum_required(VERSION 3.5)
project(yarpRTMPose LANGUAGES CXX VERSION 0.2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ENABLE_tests OFF CACHE BOOL "Enable tests")

find_package(YARP 3.2.2 REQUIRED COMPONENTS os sig dev math cv)
find_package(MMDeploy REQUIRED)
find_package(OpenCV REQUIRED)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/c++/include)
set(THIRD_PARTY ${CMAKE_CURRENT_SOURCE_DIR}/src/c++/thirdparty)

# Add MMDeploy example dirs to include utils/visualize.h
set(MMDeploy_EXAMPLE_DIR ${MMDeploy_DIR}/../../../example/cpp/cpp)

add_subdirectory(src/c++)

if(ENABLE_tests)
    add_subdirectory(tests)
endif()

file(GLOB conf ${CMAKE_CURRENT_SOURCE_DIR}/app/conf/yarpRTMPose.ini)
file(GLOB grabber_conf ${CMAKE_CURRENT_SOURCE_DIR}/app/conf/grabber.ini)
file(GLOB coco_wholebody ${CMAKE_CURRENT_SOURCE_DIR}/data/coco_wholebody.json)
file(GLOB coco_to_op ${CMAKE_CURRENT_SOURCE_DIR}/data/coco_to_op.json)
file(GLOB op_not_in_coco ${CMAKE_CURRENT_SOURCE_DIR}/data/op_not_in_coco.json)

yarp_install(FILES ${conf} DESTINATION ${YARP_CONTEXTS_INSTALL_DIR}/yarpRTMPose)
yarp_install(FILES ${grabber_conf} DESTINATION ${YARP_CONTEXTS_INSTALL_DIR}/yarpRTMPose)
yarp_install(FILES ${coco_wholebody} DESTINATION ${YARP_CONTEXTS_INSTALL_DIR}/yarpRTMPose)
yarp_install(FILES ${coco_to_op} DESTINATION ${YARP_CONTEXTS_INSTALL_DIR}/yarpRTMPose)
yarp_install(FILES ${op_not_in_coco} DESTINATION ${YARP_CONTEXTS_INSTALL_DIR}/yarpRTMPose)