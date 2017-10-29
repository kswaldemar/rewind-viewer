cmake_minimum_required(VERSION 3.0)
project(loguru LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(version_1_5_1 75606d74bd1d374e8525a53db91f054b8b7d4cac)

include(utils)
LoadDependency(https://raw.githubusercontent.com/emilk/loguru/${version_1_5_1}/loguru.hpp include/loguru.hpp)

add_library(${PROJECT_NAME} STATIC src/loguru.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include)

if (UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE dl pthread)
endif()

