Remote Viewer
=============

Remote Viewer is an application that will allow
one computer to control the keyboard and mouse input
of another while simultaneously receiving a video feed
of the desktop. 

Application is cross platform and will support Windows, Linux and macOS.

This application relies on:
  1. Boost 1.79.0 (https://www.boost.org/) for networking
  2. wxWidgets 3.2.0 (https://www.wxwidgets.org/) for window rendering
  3. CMake (https://cmake.org/download/) for building the project - optional but recommended 

# Set-up / Installation
## Run the following commands in the console from the root folder
1. Download dependencies `chmod +x download_dependencies`
2. Build wxWidgets (windowing system)
  a. `cd wxWidgets-3.2.0`
  Windows:
    1. `cd build/msw` Navigate to correct folder
    2. Open wx_vc17.sln
    3. At the top goto Build->Build
  macOS:
    1. `chmod +x build_dependencies_mac`
    2. `./build_dependencies_mac`
  Linux:
    1. `mkdir buildgtk`
    2. `cd buildgtk`
    3. `../configure --with-gtk`
    4. `make`
    5. `sudo make install`
    6. `sudo ldconfig`
    7. `cp wx-config ../../`
3. Build RemoteViewer, navigate back to the root directory of RemoteViewer
  a. `mkdir build`
  b. `cmake ..`
  Windows:
    1. Open RemoteViewer.sln and build
  macOS and Linux:
    1. `make`
