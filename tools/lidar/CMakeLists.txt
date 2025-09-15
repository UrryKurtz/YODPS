cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project (yo_lidar_in)

# This project will output an executable file
add_executable(${PROJECT_NAME} lidar_in.cpp)


# Include the configuration header in the build
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(${PROJECT_NAME} tins yo_utils)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
