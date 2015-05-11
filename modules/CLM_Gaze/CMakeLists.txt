# add_executable(SimpleCLMYarpGaze SimpleCLMYarpGaze.cpp)
add_executable(CLMGaze CLMGaze.cpp)

# Local libraries
include_directories(${CLM_SOURCE_DIR}/include)

include_directories(../../lib/local/CLM/include)

include_directories(${YARP_INCLUDE_DIRS})

include_directories(${ICUB_INCLUDE_DIRS})

			
target_link_libraries(CLMGaze CLM)

target_link_libraries(CLMGaze dlib)


if(WIN32)
    target_link_libraries(CLMGaze ${OpenCVLibraries})
endif(WIN32)
if(UNIX)
    target_link_libraries(CLMGaze ${OpenCV_LIBS} ${Boost_LIBRARIES} ${TBB_LIBRARIES} ${YARP_LIBRARIES} icubmod)
endif(UNIX)

install (TARGETS CLMGaze DESTINATION ${CMAKE_BINARY_DIR}/bin)
