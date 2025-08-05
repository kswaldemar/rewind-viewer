cmake_minimum_required(VERSION 3.11)

project(csimplesocket)

set(Sources
    src/ActiveSocket.cpp
    src/HTTPActiveSocket.cpp
    src/PassiveSocket.cpp
    src/SimpleSocket.cpp)

add_library(${PROJECT_NAME} STATIC ${Sources})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE include/csimplesocket)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
    set(os_definition _LINUX)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    set(os_definition _WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WINSOCK_DEPRECATED_NO_WARNINGS)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Darwin)
    set(os_definition _DARWIN)
endif()
target_compile_definitions(${PROJECT_NAME} PUBLIC "${os_definition}")
