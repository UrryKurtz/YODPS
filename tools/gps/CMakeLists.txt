cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project (yo_gps_utils
	VERSION 0.1
	DESCRIPTION "A YODPS GPS Utils Project")

# Create a simple configuration header
configure_file(config.h.in config.h)

set(YO_GPS yo_gps)
add_executable(${YO_GPS} yo_gps.cpp)
target_include_directories(${YO_GPS} PUBLIC ${PROJECT_BINARY_DIR})
target_link_libraries(${YO_GPS} PRIVATE zmq yo_utils gps)

set(YO_MAPS yo_maps)
add_executable(${YO_MAPS} yo_maps.cpp )
target_include_directories(${YO_MAPS} PUBLIC ${PROJECT_BINARY_DIR})
target_link_libraries(${YO_MAPS} PRIVATE zmq yo_utils)
