cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project (yo_utils
	VERSION 0.1
	DESCRIPTION "A YODPS Utils Project")

# Create a simple configuration header
configure_file(config.h.in config.h)

# This project will output an executable file
add_library(${PROJECT_NAME} 
YOMessage.cpp
YOVariant.cpp
YONode.cpp
)

# Include the configuration header in the build
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/utils)
target_link_libraries(${PROJECT_NAME} PRIVATE zmq)