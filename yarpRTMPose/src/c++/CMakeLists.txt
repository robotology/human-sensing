cmake_minimum_required(VERSION 3.5)
project(yarpRTMPose LANGUAGES CXX VERSION 0.2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} include/yarpRTMPose.h src/yarpRTMPose.cpp main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/thirdparty)
target_include_directories(${PROJECT_NAME} PRIVATE ${MMDeploy_EXAMPLE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE mmdeploy ${OpenCV_LIBS} ${YARP_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)