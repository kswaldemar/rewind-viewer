cmake_minimum_required(VERSION 3.0)

project(Glad)

find_package(OpenGL REQUIRED)

add_library(${PROJECT_NAME} STATIC src/glad.c)
target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENGL_LIBRARIES})
if (UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE dl)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
