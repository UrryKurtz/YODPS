cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project (YODPS 
	VERSION 0.2
	DESCRIPTION "A YODPS  Project")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(YODPS_PATH ${CMAKE_SOURCE_DIR})

# Create a simple configuration header
configure_file(config.h.in config.h)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)

include_directories(
third_party
third_party/cpp-httplib-0.26.0 
)

add_subdirectory(tools)
add_subdirectory(utils)
