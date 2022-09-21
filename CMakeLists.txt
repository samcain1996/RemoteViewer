# CMakeList.txt : CMake project for RemoteViewer
# currently only works for Windows 

# TODO:
#   Add support for macOS
#   Add support for linux


cmake_minimum_required (VERSION 3.8)

project (RemoteViewer)

# Executable location
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(BOOST_DIR C:\\boost_1_79_0)
set(WX_DIR C:\\Users\\scain\\source\\repos\\wxWidgets-3.1.7)


if (APPLE)

find_library(APPLICATIONSERVICES_LIBRARY ApplicationServices REQUIRED)
link_libraries(${APPLICATIONSERVICES_LIBRARY})

elseif (UNIX)

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

endif()

if (WINDOWS)

add_compile_definitions(_UNICODE)

# Add source to this project's executable.
add_executable (RemoteViewer WIN32 
  # Source files
  src/Application.cpp src/Capture.cpp src/Networking/Client.cpp
  src/Networking/NetAgent.cpp src/Networking/Packet.cpp src/Networking/Server.cpp src/Window.cpp

  # Header files
  include/Application.h include/Types.h include/Capture.h include/Networking/Client.h 
  include/Messages.h include/Networking/NetAgent.h include/Networking/Packet.h include/Networking/Server.h include/Window.h)

include_directories(include include/Networking ${WX_DIR}/include ${WX_DIR}/include/msvc ${BOOST_DIR})
target_link_directories(RemoteViewer PUBLIC ${WX_DIR}/lib/vc_x64_lib)

else

# Add source to this project's executable.
add_executable (RemoteViewer 
  # Source files
  src/Application.cpp src/Capture.cpp src/Networking/Client.cpp
  src/Networking/NetAgent.cpp src/Networking/Packet.cpp src/Networking/Server.cpp src/Window.cpp

  # Header files
  include/Application.h include/Types.h include/Capture.h include/Networking/Client.h 
  include/Messages.h include/Networking/NetAgent.h include/Networking/Packet.h include/Networking/Server.h include/Window.h)

include_directories(include include/Networking ${WX_DIR}/include ${BOOST_DIR})
target_link_directories(RemoteViewer PUBLIC ${WX_DIR}/lib/vc_x64_lib)

endif()



if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET RemoteViewer PROPERTY CXX_STANDARD 20)
endif()
