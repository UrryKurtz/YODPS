cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project (YODPS 
	VERSION 0.2
	DESCRIPTION "A YODPS  Project")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create a simple configuration header
configure_file(config.h.in config.h)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)

include_directories(third_party)

add_subdirectory(tools/broker)
add_subdirectory(tools/adtf_reader)
add_subdirectory(tools/test)
add_subdirectory(utils)
