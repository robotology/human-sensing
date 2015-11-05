add_executable(CLMYarp CLMYarp.cpp)

# Local libraries
include_directories(${CLM_SOURCE_DIR}/include)

include_directories(../../lib/local/CLM/include)

include_directories(${YARP_INCLUDE_DIRS})

			
target_link_libraries(CLMYarp CLM)

target_link_libraries(CLMYarp dlib)



if(WIN32)
	target_link_libraries(CLMYarp ${OpenCVLibraries})
endif(WIN32)
if(UNIX)
    target_link_libraries(CLMYarp ${OpenCV_LIBS} ${Boost_LIBRARIES} ${TBB_LIBRARIES} ${YARP_LIBRARIES})
endif(UNIX)

install (TARGETS CLMYarp DESTINATION bin)
