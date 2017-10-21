cmake_minimum_required(VERSION 3.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#3rdparites
add_subdirectory(3rdparty/glad)
add_subdirectory(3rdparty/imgui)
add_subdirectory(3rdparty/stb_image)

#Project
add_subdirectory(src)
