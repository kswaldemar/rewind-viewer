cmake_minimum_required(VERSION 3.11)
project(loguru LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(VERSION v2.1.0)

include(FetchContent)
FetchContent_Declare(loguru_header
    URL https://raw.githubusercontent.com/emilk/loguru/${VERSION}/loguru.hpp
    DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_Declare(loguru_source
    URL https://raw.githubusercontent.com/emilk/loguru/${VERSION}/loguru.cpp
    DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(loguru_header loguru_source)

add_library(${PROJECT_NAME} STATIC ${loguru_source_SOURCE_DIR}/loguru.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${loguru_header_SOURCE_DIR})

if (UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE dl pthread)
endif()

