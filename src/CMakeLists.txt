cmake_minimum_required(VERSION 3.0)

project(rewindviewer LANGUAGES C CXX)

add_subdirectory(graphics)
add_subdirectory(viewer)

find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Utils Viewer ${OpenGL_LIBRARIES})

