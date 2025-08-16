cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project (yo_viewer)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/bin)

set (CMAKE_PREFIX_PATH  ${YODPS_PATH}/third_party/rbfx/share/CMake)
find_package(Urho3D REQUIRED)

message (${Urho3D_LIBRARIES} === )

# This project will output an executable file
add_executable(${PROJECT_NAME} 
main.cpp
#YOViewer.cpp
)

# Include the configuration header in the build
target_include_directories(${PROJECT_NAME} PUBLIC 
"${PROJECT_BINARY_DIR}" 
 ${YODPS_PATH}/third_party/rbfx/include
 ${YODPS_PATH}/third_party/rbfx/include/Urho3D
 ${YODPS_PATH}/third_party/rbfx/include/Urho3D/ThirdParty
 ${YODPS_PATH}/third_party/rbfx/include/Urho3D/ThirdParty/tracy
)
  
target_link_libraries(${PROJECT_NAME}  yo_utils Urho3D) 
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
