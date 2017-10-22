//
// Created by valdemar on 14.10.17.
//

#include "UIController.h"

#include "imgui_impl/imgui_impl_glfw_gl3.h"
#include "imgui_impl/style.h"

struct UIController::wnd_t {
    bool show_style_editor = false;
    bool show_fps_overlay = true;
};

UIController::UIController(GLFWwindow *window, Camera *camera)
    : camera_(camera)
{
    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, true);
    wnd_ = std::make_unique<wnd_t>();

    setup_custom_style(false);
}

UIController::~UIController() {
    // Cleanup imgui
    ImGui_ImplGlfwGL3_Shutdown();
}

void UIController::next_frame() {
    // Start new frame
    ImGui_ImplGlfwGL3_NewFrame();

    // Clear the screen to black
    glClearColor(clear_color_.x, clear_color_.y, clear_color_.z, clear_color_.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    update_windows();

    check_input();
}

void UIController::frame_end() {
    ImGui::Render();
}

bool UIController::close_requested() {
    return request_exit_;
}

void UIController::update_windows() {
    //debug_window();
    //
    //if (wnd_->show_style_editor) {
    //    ImGui::Begin("Style Editor", &wnd_->show_style_editor);
    //    ImGui::ShowStyleEditor();
    //    ImGui::End();
    //}

    if (wnd_->show_fps_overlay) {
        fps_overlay_window();
    }
}

void UIController::debug_window() {
    static float f = 0.0f;
    ImGui::Text("Hello, world!");
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    ImGui::ColorEdit3("clear color", reinterpret_cast<float *>(&clear_color_));
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);

    if (ImGui::Button("Set default style")) {
        ImGui::GetStyle() = ImGuiStyle();
    }

    if (ImGui::Button("Set light style")) {
        setup_custom_style(false);
    }

    if (ImGui::Button("Show style editor")) {
        wnd_->show_style_editor = true;
    }
}

void UIController::fps_overlay_window() {
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    const auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                       | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    if (!ImGui::Begin("Example: Fixed Overlay", &wnd_->show_fps_overlay, ImVec2(0, 0), 0.3f, flags)) {
        ImGui::End();
        return;
    }
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    //ImGui::Separator();
    //ImGui::Text("Mouse Position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
    ImGui::End();
}

void UIController::check_input() {
    const auto &io = ImGui::GetIO();
    if (io.KeysDown[GLFW_KEY_W]) {
        camera_->forward();
    }
    if (io.KeysDown[GLFW_KEY_S]) {
        camera_->backward();
    }
    if (io.KeysDown[GLFW_KEY_A]) {
        camera_->left();
    }
    if (io.KeysDown[GLFW_KEY_D]) {
        camera_->right();
    }
    //if (io.KeysDown[GLFW_KEY_E]) {
    //    camera_->up();
    //}
    //if (io.KeysDown[GLFW_KEY_Q]) {
    //    camera_->down();
    //}

    request_exit_ = io.KeysDown[GLFW_KEY_ESCAPE];
}
