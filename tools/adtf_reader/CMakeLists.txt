cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project (yo_reader
	VERSION 0.1
	DESCRIPTION "A YODPS ADTF file reader")

link_directories(/home/kurtz/opt/adtf_file/lib)
link_directories(/home/kurtz/opt/dev_essetial/lib)

# Create a simple configuration header
configure_file(config.h.in config.h)

# This project will output an executable file
add_executable(${PROJECT_NAME} yo_reader.cpp)

# Include the configuration header in the build
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR} /home/kurtz/opt/adtf_file/include /home/kurtz/opt/dev_essetial/include)
target_link_libraries(${PROJECT_NAME} PRIVATE zmq yo_utils 
adtf_file
ifhd_file 
utils5extension
ddl
a_util_filesystem
a_util_concurrency
a_util_datetime 
a_util_logging
a_util_memory 
a_util_regex
a_util_result
a_util_strings 
a_util_system
a_util_variant
a_util_xml )
