cmake_minimum_required(VERSION 3.11)

project(ImGui)

add_library(${PROJECT_NAME} STATIC
    include/imgui.h
    include/imgui_internal.h
    include/imstb_rectpack.h
    include/imstb_textedit.h
    include/imstb_truetype.h

    src/imgui.cpp
    src/imgui_demo.cpp
    src/imgui_draw.cpp
    src/imgui_widgets.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
#Dependency for implicit vector conversions
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm-header-only)
