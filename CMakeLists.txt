# CMakeList.txt : CMake project for RemoteViewer
# currently only works for Windows 

# TODO:
#   Add support for macOS
#   Add support for linux


cmake_minimum_required (VERSION 3.8)

project(RemoteViewer VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(FALLBACK_WX_DIR ${CMAKE_SOURCE_DIR}/wxWidgets)

add_compile_definitions(_UNICODE)

# Download and dependencies
# execute_process(
#   COMMAND bash -c "chmod +x setup/download_dependencies"
#   COMMAND bash -c "setup/download_dependencies ."
#   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

if (UNIX)

  # Build dependencies
  execute_process(COMMAND bash -c "mkdir buildgtk"
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/wxWidgets)

  execute_process(COMMAND bash -c "../configure --with-gtk;make;sudo make install;sudo ldconfig"
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/wxWidgets/buildgtk)
 

  find_package(wxWidgets REQUIRED)
              include(${wxWidgets_USE_FILE})
              link_libraries(${wxWidgets_LIBRARIES})
endif()

if(WIN32)

# Add source to this project's executable.
add_executable (RemoteViewer WIN32 

# Source files
src/Application.cpp 
src/Logger/Logger.cpp
src/QuickShot/Capture.cpp #src/QuickShot/Scaler.cpp 
src/Networking/Netagent.cpp src/Networking/Client.cpp src/Networking/Server.cpp src/Networking/Packet.cpp 
src/Windows/RemoteViewerWindows.cpp src/Windows/ClientStreamWindow.cpp src/Windows/ServerInitWindow.cpp)

#find_package(wxWidgets REQUIRED)
#include(${wxWidgets_USE_FILE})
#target_link_libraries(RemoteViewer PUBLIC ${wxWidgets_LIBRARIES})

include_directories(${FALLBACK_WX_DIR}/include ${FALLBACK_WX_DIR}/include/msvc)
target_link_directories(RemoteViewer PUBLIC ${FALLBACK_WX_DIR}/lib/vc_x64_lib)

else()

add_executable (RemoteViewer
  
  # Source files
  src/Application.cpp 
  src/Logger/Logger.cpp
  src/QuickShot/Capture.cpp #src/QuickShot/Scaler.cpp 
  src/Networking/NetAgent.cpp src/Networking/Client.cpp src/Networking/Server.cpp src/Networking/Packet.cpp 
  src/Windows/RemoteViewerWindows.cpp src/Windows/ClientStreamWindow.cpp src/Windows/ServerInitWindow.cpp)

endif()

target_include_directories(RemoteViewer PUBLIC ${CMAKE_SOURCE_DIR}/boost)

if (UNIX)

find_package(X11 REQUIRED)
target_link_libraries(RemoteViewer PUBLIC ${X11_LIBRARIES})
target_include_directories(RemoteViewer PUBLIC ${X11_INCLUDE_DIR})

elseif(APPLE)

find_library(APPLICATIONSERVICES_LIBRARY ApplicationServices REQUIRED)
target_link_libraries(RemoteViewer PUBLIC ${APPLICATIONSERVICES_LIBRARY})

endif()

target_include_directories(RemoteViewer PUBLIC 
  ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/Networking  
  ${CMAKE_SOURCE_DIR}/include/QuickShot ${CMAKE_SOURCE_DIR}/include/Logger ${CMAKE_SOURCE_DIR}/include/Windows)

configure_file(${CMAKE_SOURCE_DIR}/setup/logo.png ${CMAKE_BINARY_DIR}/assets/logo.png COPYONLY)


if(UNIX)

execute_process(COMMAND bash -c "make"
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

elseif(APPLE)                

execute_process(COMMAND bash -c "make"
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

endif()                