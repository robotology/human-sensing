cmake_minimum_required(VERSION 3.5)
project(yarpRTMPoseTest LANGUAGES CXX VERSION 0.2)

Include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0 # or a later release
)
message("Fetching catch2...")
FetchContent_MakeAvailable(Catch2)

add_executable(${PROJECT_NAME} process_image.cpp ${INCLUDE_DIR}/yarpRTMPose.h ${CMAKE_SOURCE_DIR}/src/c++/src/yarpRTMPose.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR} ${THIRD_PARTY})
target_include_directories(${PROJECT_NAME} PRIVATE ${MMDeploy_EXAMPLE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE mmdeploy ${OpenCV_LIBS} ${YARP_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2WithMain)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
