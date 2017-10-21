cmake_minimum_required(VERSION 2.8)

project(ImGui)

add_library(${PROJECT_NAME} STATIC
    imgui.h
    imgui_internal.h
    stb_rect_pack.h
    stb_textedit.h
    stb_truetype.h

    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
