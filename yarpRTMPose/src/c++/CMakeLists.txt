cmake_minimum_required(VERSION 3.5)
project(yarpRTMPose LANGUAGES CXX VERSION 0.2)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(YARP 3.2.2 REQUIRED COMPONENTS os sig dev math cv)

add_executable(${PROJECT_NAME} include/yarpRTMPose.h src/yarpRTMPose.cpp main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
message("YARP_LIBRARIES = " ${YARP_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${YARP_LIBRARIES})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
