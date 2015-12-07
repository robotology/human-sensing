# add_executable(SimpleCLMYarpGaze SimpleCLMYarpGaze.cpp)
add_executable(CLMH CLMH.cpp)

# Local libraries
include_directories(${CLM_SOURCE_DIR}/include)

include_directories(../../lib/local/CLM/include)

include_directories(${YARP_INCLUDE_DIRS})

include_directories(${ICUB_INCLUDE_DIRS})

			
#target_link_libraries(SimpleCLMYarpGaze CLM)

#target_link_libraries(SimpleCLMYarpGaze dlib)

target_link_libraries(CLMH CLM)

target_link_libraries(CLMH dlib)


if(WIN32)
    # target_link_libraries(SimpleCLMYarpGaze ${OpenCVLibraries})
    target_link_libraries(CLMH ${OpenCVLibraries})
endif(WIN32)
if(UNIX)
    # target_link_libraries(SimpleCLMYarpGaze ${OpenCV_LIBS} ${Boost_LIBRARIES} ${TBB_LIBRARIES} ${YARP_LIBRARIES} icubmod)
    target_link_libraries(CLMH ${OpenCV_LIBS} ${Boost_LIBRARIES} ${TBB_LIBRARIES} ${YARP_LIBRARIES})
endif(UNIX)

#install (TARGETS SimpleCLMYarpGaze DESTINATION ${CMAKE_BINARY_DIR}/bin)
install (TARGETS CLMH DESTINATION bin)
