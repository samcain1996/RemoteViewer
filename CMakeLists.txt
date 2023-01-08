# CMakeList.txt : CMake project for RemoteViewer
# currently only works for Windows 

# TODO:
#   Add support for macOS
#   Add support for linux


cmake_minimum_required (VERSION 3.8)

project(RemoteViewer VERSION 0.0.1)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(BOOST_DIR ${CMAKE_SOURCE_DIR}\\boost)
set(WX_DIR ${CMAKE_SOURCE_DIR}\\wxWidgets)

file(WRITE ${CMAKE_BINARY_DIR}/asset_dir.txt ${CMAKE_SOURCE_DIR}/assets)

add_compile_definitions(_UNICODE)

# Add source to this project's executable.
add_executable (RemoteViewer WIN32 
  # Source files
  src/Application.cpp src/QuickShot/Capture.cpp src/Networking/Client.cpp src/Logger/Logger.cpp
  src/Networking/NetAgent.cpp src/Networking/Packet.cpp src/Networking/Server.cpp src/Windows/RemoteViewerWindows.cpp
  src/Windows/ClientStreamWindow.cpp src/Windows/ServerInitWindow.cpp)

target_include_directories(RemoteViewer PUBLIC build/ ${WX_DIR}/include ${WX_DIR}/include/msvc ${BOOST_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/Networking
  ${CMAKE_SOURCE_DIR}/include/Windows ${CMAKE_SOURCE_DIR}/include/QuickShot ${CMAKE_SOURCE_DIR}/include/Logger ${CMAKE_SOURCE_DIR}/include/Eigen)
target_link_directories(RemoteViewer PUBLIC ${WX_DIR}/lib/vc_x64_lib)
set_target_properties(RemoteViewer PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")