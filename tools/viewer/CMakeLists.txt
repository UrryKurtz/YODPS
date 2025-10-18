cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project (yo_viewer)

link_directories(${YODPS_PATH}/third_party/dbc_parser_cpp/lib)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/bin)

set (CMAKE_PREFIX_PATH  ${YODPS_PATH}/third_party/rbfx/share/CMake)
find_package(Urho3D REQUIRED)

file(GLOB CONVERT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/convert/*.cpp")

# This project will output an executable file
add_executable(${PROJECT_NAME} 
main.cpp
YOViewer.cpp
YOGui.cpp
YOFlyController.cpp
YOPluginBus.cpp
plugins/YOPolylinePlugin.cpp
plugins/YORootLogic.cpp
plugins/YONodeLogic.cpp
plugins/YOVideoPlugin.cpp
plugins/YOCameraPlugin.cpp
plugins/YOGPSPlugin.cpp
plugins/YOGPSUtils.cpp
plugins/YOPlotterPlugin.cpp
plugins/YOOBD2Plugin.cpp
plugins/YOCANPlugin.cpp
plugins/YOTestPlugin.cpp
plugins/YODataViewerPlugin.cpp
plugins/YORecorderPlugin.cpp
${CONVERT_SOURCES}
)

# Include the configuration header in the build
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR} plugins
"${PROJECT_BINARY_DIR}" 
 ${YODPS_PATH}/third_party/rbfx/include
 ${YODPS_PATH}/third_party/rbfx/include/Urho3D
 ${YODPS_PATH}/third_party/rbfx/include/Urho3D/ThirdParty
 ${YODPS_PATH}/third_party/rbfx/include/Urho3D/ThirdParty/tracy
 ${YODPS_PATH}/third_party/dbc_parser_cpp/include
)
  
target_link_libraries(${PROJECT_NAME} yo_utils Urho3D ImGui ImGuizmo turbojpeg crypto ssl libdbc.a) 
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
