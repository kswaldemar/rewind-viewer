cmake_minimum_required(VERSION 3.0)

project(rewindviewer LANGUAGES C CXX)

add_compile_options(-Wall -Wextra -Weffc++)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

message("${CMAKE_MODULE_PATH}")

include(RequireLibrary)
RequireLibrary(Glfw glfw)

set(Sources
    main.cpp
    imgui_impl/imgui_impl_glfw_gl3.cpp
    cgutils/Shader.cpp
    cgutils/Camera.cpp

    viewer/UIController.cpp
)

set(Headers
    cgutils/utils.h
    common/logger.h
    imgui_impl/style.h
    viewer/UIController.h
)

add_executable(${PROJECT_NAME} ${Sources} ${Headers})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
    Glad
    ImGui
    stb_image
    ${Glfw_LIBRARY})

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


