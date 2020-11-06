cmake_minimum_required(VERSION 3.0)
project(rewindviewer LANGUAGES C CXX)

add_compile_options(-Wall)
if (MSVC)
    add_compile_options(/W0)
else()
    add_compile_options(-Wextra)
endif()

set(CMAKE_CXX_STANDARD 14)

set(Sources
    main.cpp
    imgui_impl/imgui_impl_glfw.cpp
    imgui_impl/imgui_impl_opengl3.cpp
    cgutils/Shader.cpp
    cgutils/Camera.cpp
    cgutils/utils.cpp
    cgutils/ResourceManager.cpp
    viewer/UIController.cpp
    viewer/Scene.cpp
    viewer/Renderer.cpp
    viewer/Config.cpp
    viewer/Popup.cpp

    viewer/RenderContext.cpp
    viewer/ShaderCollection.cpp
    viewer/Frame.cpp
    viewer/FrameEditor.cpp

    net/NetListener.cpp
    net/ProtoHandler.cpp
    net/json_handler/JsonHandler.cpp
    net/PrimitiveType.cpp
)

add_executable(${PROJECT_NAME} ${Sources})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}
    Glad glfw glm
    ImGui stb_image csimplesocket nljson loguru)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE OPENGL_DEBUG)
endif()

