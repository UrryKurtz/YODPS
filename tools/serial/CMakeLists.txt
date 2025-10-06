cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project (yo_serial_utils
	VERSION 0.1
	DESCRIPTION "A YODPS Serial Utils Project")

# Create a simple configuration header
configure_file(config.h.in config.h)

set(YO_SERIAL yo_serial)
add_executable(${YO_SERIAL} yo_serial.cpp)
target_include_directories(${YO_SERIAL} PUBLIC ${PROJECT_BINARY_DIR})
target_link_libraries(${YO_SERIAL} PRIVATE zmq yo_utils)

