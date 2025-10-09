cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project (yo_pcap_in)

# This project will output an executable file
add_executable(${PROJECT_NAME} pcap_in.cpp)


# Include the configuration header in the build
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(${PROJECT_NAME} tins yo_utils)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

set(YO_CAN_IN yo_can_in)
add_executable(${YO_CAN_IN} yo_can_in.cpp)
target_include_directories(${YO_CAN_IN} PUBLIC ${PROJECT_BINARY_DIR})
target_link_libraries(${YO_CAN_IN} PRIVATE zmq yo_utils)