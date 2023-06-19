cmake_minimum_required(VERSION 3.5)
project(yarpRTMPoseTest LANGUAGES CXX VERSION 0.2)

find_package(Catch2 3 REQUIRED)

add_executable(${PROJECT_NAME} process_image.cpp ${INCLUDE_DIR}/yarpRTMPose.h ${CMAKE_SOURCE_DIR}/src/c++/src/yarpRTMPose.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR} ${THIRD_PARTY})
target_include_directories(${PROJECT_NAME} PRIVATE /usr/local/example/cpp/cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE mmdeploy ${OpenCV_LIBS} ${YARP_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2WithMain)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
