cmake_minimum_required(VERSION 3.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#3rdparites
add_subdirectory(3rdparty/glad)
add_subdirectory(3rdparty/imgui)
add_subdirectory(3rdparty/stb_image)
add_subdirectory(3rdparty/csimplesocket)
add_subdirectory(3rdparty/json)

#Project
add_subdirectory(src)
