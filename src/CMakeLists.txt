cmake_minimum_required(VERSION 3.11)
project(rewindviewer LANGUAGES C CXX)

add_compile_options(-Wall)
if (MSVC)
    add_compile_options(/W0)
else()
    add_compile_options(-Wextra)
endif()

set(CMAKE_CXX_STANDARD 20)

file(GLOB SHADER_FILES "${CMAKE_SOURCE_DIR}/resources/shaders/*")
embed_shaders(${SHADER_FILES})

embed_binary_resource(generated/resources/fontawesome_ttf.h resources/fonts/fontawesome-webfont.ttf)
embed_binary_resource(generated/resources/icon_png.h resources/icon.png)

# Create custom target for all generated resources
add_custom_target(Embedded_resources
    DEPENDS
        ${CMAKE_BINARY_DIR}/generated/resources/shaders/circle.frag.h
        ${CMAKE_BINARY_DIR}/generated/resources/shaders/circle.geom.h
        ${CMAKE_BINARY_DIR}/generated/resources/shaders/circle.vert.h
        ${CMAKE_BINARY_DIR}/generated/resources/shaders/color_pos.frag.h
        ${CMAKE_BINARY_DIR}/generated/resources/shaders/color_pos.vert.h
        ${CMAKE_BINARY_DIR}/generated/resources/shaders/simple.vert.h
        ${CMAKE_BINARY_DIR}/generated/resources/shaders/uniform_color.frag.h
        ${CMAKE_BINARY_DIR}/generated/resources/fontawesome_ttf.h
        ${CMAKE_BINARY_DIR}/generated/resources/icon_png.h
)

set(Sources
    main.cpp

    imgui_impl/imgui_impl_glfw.cpp
    imgui_impl/imgui_impl_opengl3.cpp

    cgutils/Shader.cpp
    cgutils/Camera.cpp
    cgutils/utils.cpp
    cgutils/ResourceManager.cpp
    common/Spinlock.cpp

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
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/generated)
target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

add_dependencies(${PROJECT_NAME} Embedded_resources)

target_link_libraries(${PROJECT_NAME}
    Glad glfw glm
    ImGui stb_image csimplesocket nljson loguru)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE OPENGL_DEBUG)
endif()

