# CMakeList.txt : CMake project for RemoteViewer
# currently only works for Windows 

# TODO:
#   Add support for macOS
#   Add support for linux


cmake_minimum_required (VERSION 3.8)

project(RemoteViewer)

# Executable location
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(BOOST_DIR C:\\boost_1_79_0)
set(WX_DIR C:\\Users\\scain\\source\\repos\\dependencies\\wxWidgets-3.1.7)

add_compile_definitions(_UNICODE)

# Add source to this project's executable.
add_executable (RemoteViewer WIN32 
  # Source files
  src/Application.cpp src/QuickShot/Capture.cpp src/Networking/Client.cpp
  src/Networking/NetAgent.cpp src/Networking/Packet.cpp src/Networking/Server.cpp src/Windows/RemoteViewerWindows.cpp
  src/Windows/ClientStreamWindow.cpp src/Windows/ServerInitWindow.cpp

  # Header files
  include/Application.h include/Types.h include/QuickShot/Capture.h include/Networking/Client.h 
  include/Messages.h include/Networking/NetAgent.h include/Networking/Packet.h include/Networking/Server.h include/Windows/RemoteViewerWindows.h)

include_directories(include include/Networking include/QuickShot include/Windows ${WX_DIR}/include ${WX_DIR}/include/msvc ${BOOST_DIR})
target_link_directories(RemoteViewer PUBLIC ${WX_DIR}/lib/vc_x64_lib)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET RemoteViewer PROPERTY CXX_STANDARD 20)
endif()
