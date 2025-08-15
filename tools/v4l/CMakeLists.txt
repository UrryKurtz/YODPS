cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project (yo_v4l_in)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/bin)

# This project will output an executable file
add_executable(${PROJECT_NAME} v4l_in.cpp)


# Include the configuration header in the build
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(${PROJECT_NAME} yo_utils)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})