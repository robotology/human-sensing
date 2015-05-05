add_executable(SimpleCLMYarp SimpleCLMYarp.cpp)

# Local libraries
include_directories(${CLM_SOURCE_DIR}/include)

include_directories(../../lib/local/CLM/include)

include_directories(${YARP_INCLUDE_DIRS})

			
target_link_libraries(SimpleCLMYarp CLM)

target_link_libraries(SimpleCLMYarp dlib)



if(WIN32)
	target_link_libraries(SimpleCLMYarp ${OpenCVLibraries})
endif(WIN32)
if(UNIX)
    target_link_libraries(SimpleCLMYarp ${OpenCV_LIBS} ${Boost_LIBRARIES} ${TBB_LIBRARIES} ${YARP_LIBRARIES})
endif(UNIX)

install (TARGETS SimpleCLMYarp DESTINATION ${CMAKE_BINARY_DIR}/bin)
