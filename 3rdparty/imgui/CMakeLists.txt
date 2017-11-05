cmake_minimum_required(VERSION 2.8)

project(ImGui)

add_library(${PROJECT_NAME} STATIC
    include/imgui.h
    include/imgui_internal.h
    include/stb_rect_pack.h
    include/stb_textedit.h
    include/stb_truetype.h

    src/imgui.cpp
    src/imgui_demo.cpp
    src/imgui_draw.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
#Dependency for implicit vector conversions
target_include_directories(${PROJECT_NAME} PRIVATE glm)
