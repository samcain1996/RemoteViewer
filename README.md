Remote Viewer
=============

Remote Viewer is an application that will allow
one computer to control the keyboard and mouse input
of another while simultaneously receiving a video feed
of the desktop. 

Application is cross platform and supports the big 3 operating systems (Windows, Linux, macOS).

This application relies on:
  1. Boost 1.79.0 (https://www.boost.org/)
  2. SDL2 and SDL_TTF (https://www.libsdl.org/)
  3. OpenTTF
  
Because it is kind of a pain to get all the dependencies working on each operating system (particularly OpenTTF), I am moving away from OpenTTF and manual window rendering in favor of wxWidgets.
