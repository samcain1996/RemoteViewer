Remote Viewer
=============

Remote Viewer is an application that will allow
one computer to control the keyboard and mouse input
of another while simultaneously receiving a video feed
of the desktop. 

Application is cross platform and will support Windows, Linux and macOS.

This application relies on:
  1. CMake (https://cmake.org/download/) for building the project
  2. Boost (https://www.boost.org/) for networking
  3. wxWidgets (https://www.wxwidgets.org/) for window rendering

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
    
## How To Use
To test the application you need one of the following:
  1. 2 computers on the same network.
  2. 2 computers remotely of the computer receiving the video stream must have forwarded its ports
  3. 1 computer locally with 2 instances of the application open
 
1. Obtain the computers' ip addresses 
2. On the computer that you want to view, press the button labelled 'Server'
   On the other computer, press the button labelled 'Client'
3. On the 'Server' enter a port you would like to listen to (default should be ok)
   on the 'Client' enter the ip address of the 'Server', the port to connect to (top), and the port to use (default should be ok)'
4. On the 'Server' FIRST, press the button labelled 'Listen'. Then, on the 'Client' 
