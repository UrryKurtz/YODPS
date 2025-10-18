cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project (yo_broker
	VERSION 0.3
	DESCRIPTION "A YODPS Broker Project")

# Create a simple configuration header
configure_file(config.h.in config.h)

# This project will output an executable file
add_executable(${PROJECT_NAME} yo_proxy.cpp)

# Include the configuration header in the build
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR} )


target_link_libraries(${PROJECT_NAME} PRIVATE zmq yo_utils)