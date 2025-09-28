cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project (yo_test
	VERSION 0.1
	DESCRIPTION "A YODPS Test Project")

# Create a simple configuration header
configure_file(config.h.in config.h)

set(YO_RECEIVER yo_receiver)
add_executable(${YO_RECEIVER} yo_receiver.cpp)
target_include_directories(${YO_RECEIVER} PUBLIC ${PROJECT_BINARY_DIR})
target_link_libraries(${YO_RECEIVER} PRIVATE zmq yo_utils)

set(YO_SENDER yo_sender)
add_executable(${YO_SENDER} yo_sender.cpp)
target_include_directories(${YO_SENDER} PUBLIC ${PROJECT_BINARY_DIR})
target_link_libraries(${YO_SENDER} PRIVATE zmq yo_utils)

set(YO_MODELS yo_models)
add_executable(${YO_MODELS} yo_models.cpp)
target_include_directories(${YO_MODELS} PUBLIC ${PROJECT_BINARY_DIR})
target_link_libraries(${YO_MODELS} PRIVATE zmq yo_utils)