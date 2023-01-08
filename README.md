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
1. Download dependencies `chmod +x setup/download_dependencies`
  1a. `setup/download_dependencies .`

2. Build wxWidgets (windowing system)
  2a. `cd wxWidgets`
  Windows:
    1. `cd build/msw` Navigate to correct folder
    2. `explorer .`
    3. Open wx_vc17.sln
    4. At the top goto Build->Build, wait for build
    5. `cd ../../../` (in the terminal again)
  macOS:
    1. `chmod +x build_dependencies_mac`
    2. `./build_dependencies_mac ../`
    3. `cd ../`
  Linux:
    1. `mkdir buildgtk`
    2. `cd buildgtk`
    3. `../configure --with-gtk`
    4. `make`
    5. `sudo make install`
    6. `sudo ldconfig`
    7. `cp wx-config ../../`
    8. `cd ../../`

3. Build RemoteViewer, navigate back to the root directory of RemoteViewer
  3a. `mkdir build`
  3b. `cd build`
  3c. `cmake ../`
  3d. (if Linux or macOS ONLY) `make`
      (if Windows ONLY) Run `explorer .` and open RemoteViewer.sln
      3e. Under the 'Solution Explorer' on the right hand side, 
        right-click RemoteViewer and select 'Set as Startup Project'
      3f. Run
