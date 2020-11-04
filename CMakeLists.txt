cmake_minimum_required(VERSION 3.0)
project(rewind-viewer)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#3rdparites
add_subdirectory(3rdparty/glad)
add_subdirectory(3rdparty/stb_image)
add_subdirectory(3rdparty/csimplesocket)
add_subdirectory(3rdparty/json)
add_subdirectory(3rdparty/loguru)

if(UNIX)
    find_library(Glfw_LIBRARY glfw)
else()
    find_library(Glfw_LIBRARY glfw3)
endif()
if (Glfw_LIBRARY)
    message(STATUS "Found glfw in system path: ${Glfw_LIBRARY}")
    add_library(GLFW SHARED IMPORTED)
    set_target_properties(GLFW PROPERTIES IMPORTED_LOCATION ${Glfw_LIBRARY})
else()
    message(STATUS "Build glfw from sources: ${Glfw_LIBRARY}")
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    add_subdirectory(3rdparty/glfw)
endif()

set(GLM_TEST_ENABLE OFF)
add_subdirectory(3rdparty/glm)
add_subdirectory(3rdparty/imgui) # Depends from glm


#Project
add_subdirectory(src)