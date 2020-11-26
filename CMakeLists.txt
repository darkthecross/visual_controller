cmake_minimum_required(VERSION 3.5)

project(mobile_planner)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

#####################################################################################

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# Add protos.
ADD_SUBDIRECTORY(src/proto)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

add_executable(collector src/collector.cc)
target_link_libraries(collector training_example_proto ${OpenCV_LIBS} )

add_executable(inspector src/inspector.cc)
target_link_libraries(inspector training_example_proto ${OpenCV_LIBS} )
