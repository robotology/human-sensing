cmake_minimum_required(VERSION 3.5)
project(yarpRTMPoseTest LANGUAGES CXX VERSION 0.2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(YARP 3.2.2 REQUIRED COMPONENTS os sig dev math cv)
find_package(OpenCV REQUIRED)
find_package(MMDeploy REQUIRED)
find_package(Catch2 3 REQUIRED)

add_executable(${PROJECT_NAME} process_image.cpp ../../../src/c++/include/yarpRTMPose.h ../../../src/c++/src/yarpRTMPose.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ../../../src/c++/include)
target_include_directories(${PROJECT_NAME} PRIVATE /mmdeploy/build/install/example/cpp/cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE mmdeploy ${OpenCV_LIBS} ${YARP_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2WithMain)