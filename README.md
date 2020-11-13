# Rewind viewer

[![MIT License](https://img.shields.io/github/license/kswaldemar/rewind-viewer.svg?style=flat-square)](./LICENSE)
[![C++ standard](https://img.shields.io/badge/C++-14-blue.svg?style=flat-square)](https://isocpp.org/)
[![OpenGL](https://img.shields.io/badge/OpenGL-3.3-green.svg?style=flat-square)](https://www.khronos.org/opengl/)
[![RAIC](https://img.shields.io/badge/Russian%20AI%20Cup-2017-yellow.svg?style=flat-square)](http://russianaicup.ru/)
[![Build status](https://travis-ci.org/kswaldemar/rewind-viewer.svg?branch=master)](https://travis-ci.org/kswaldemar/rewind-viewer)
[![Linux](https://github.com/kswaldemar/rewind-viewer/workflows/Linux/badge.svg)](https://github.com/kswaldemar/rewind-viewer/actions?query=workflow%3ALinux)
[![Windows](https://github.com/kswaldemar/rewind-viewer/workflows/Windows/badge.svg)](https://github.com/kswaldemar/rewind-viewer/actions?query=workflow%3AWindows)
[![GitHub Releases](https://img.shields.io/github/release/kswaldemar/rewind-viewer.svg?style=flat-square)](https://github.com/kswaldemar/rewind-viewer/releases)

Fast Russain AI Cup championship match viewer with rewinding support written in OpenGL

![](https://user-images.githubusercontent.com/10009947/33450137-e71f641e-d61b-11e7-9802-e14575e63b75.png)

## Overview
The viewer has several advantages in comparison of local-runner with drawing plugin:
 - All figures are drawn using your video adapter, so no more problems with slow drawing
 - Rewinding - ability to navigate between game tick
 - In Pause navigation - zoom and navigate in any game state
 - Handy mouse navigation

Drawbacks:
 - Viewer running as standalone application, it knows nothing about local runner or your strategy, so you need manually 
send all data (like buildings, units etc.) and you can draw only data visible by your strategy
 - In theory, high memory usage, because it needs to store all drawing primitives for rewinding support


## Binaries

Source code with changelog for significant releases can be found in [github releases page](https://github.com/kswaldemar/rewind-viewer/releases).

Prebuilt windows binaries for other version, such as bugfixes [can be found here](https://github.com/kswaldemar/rewind-viewer/issues/23). It is required to have x86 C++ Redistributable for Visual Studio 2015 installed to run prebuild windows binaries.


## Build

Clone repository with submodules:
```
git clone --recursive https://github.com/kswaldemar/rewind-viewer.git
```

Unix, MacOS:
```
mkdir build && cd build
cmake --CMAKE_BUILD_TYPE=Release ..
cmake --build .
```
Windows:
```
mkdir build && cd build
cmake ..
cmake --build . --config Release
```
*Note*: Compiler with c++14 support needed. That means Visual Studio 2015 or higher on Windows. 

:warning: **Note**: Viewer should be launched from the same folder, where `resources` is located. 
So you need to manually copy `resources` to build folder, or copy executable to project root directory.

## Strategy integration

Strategy should send commands in json format via socket. You may use one from `clients` folder or implement your own.

:warning: Json protocol starting from release 2.0 doesn't compatible with older clients (from 1.3 and below). 
Check that your client is updated. 

Sample usage: 
1. Start the viewer.
2. Start localrunner, preferably in render_to_screen=false mode.
3. Start your strategy.
4. To be able to drew things in the viewer you will need to create a client, send data to the client in your strategy, and **end the frame** with client command. 
5. There is no need to close the viewer after the strategy is done, just start from step 2. Old drawn data will be cleaned after new connection.

### Create client four your language

You can use [Python3 client](https://github.com/kswaldemar/rewind-viewer/blob/master/clients/python3/RewindClient.py) as a reference.

Documentation for json protocol [can be found here](./clients/README.md).
## License
Project sources distributed under [MIT license](https://github.com/kswaldemar/rewind-viewer/blob/master/LICENSE), third parties distributed under their own licences

## Credits
Project created with help of many great libraries:
 - [glad](https://github.com/Dav1dde/glad) for loading OpenGL functions
 - [glm](https://glm.g-truc.net/0.9.8/index.html) math library for OpenGL
 - [glfw](http://www.glfw.org/) for creating window and crossplatform context handling
 - [ImGui](https://github.com/ocornut/imgui) for UI widgets inside application
 - [nlohmann json](https://github.com/nlohmann/json) for json operating
 - [csimplesocket](https://github.com/DFHack/clsocket) for network interaction
 - [stb_image](https://github.com/nothings/stb) for images processing
 - [loguru](https://github.com/emilk/loguru) for logging support

Resources: 
 - [fontawesome](http://fontawesome.io/) icon font, embedded inside text in many UI elements
 - Application icon by [Laura Reen](https://www.iconfinder.com/laurareen)  
 
