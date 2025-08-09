cmake_minimum_required(VERSION 3.11)

project(rapidjson)

set(Revision 24b5e7a8b27f42fa16b96fc70aade9106cf7102f)

set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "" FORCE)

include(FetchContent)
# Using rapidjson version from 2025-02-26, same as used in vcpkg
# https://github.com/microsoft/vcpkg/blob/d30fdf55cfca16e12bc3ad99cbc615997014b61b/ports/rapidjson/portfile.cmake
# Because last release of rapidjson is from 2016
FetchContent_Declare(rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG 24b5e7a8b27f42fa16b96fc70aade9106cf7102f # accessed on 2025-02-26
)
FetchContent_MakeAvailable(rapidjson)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${rapidjson_SOURCE_DIR}/include)