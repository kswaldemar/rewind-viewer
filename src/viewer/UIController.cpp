//
// Created by valdemar on 14.10.17.
//

#include "UIController.h"

#include <imgui_impl/imgui_widgets.h>
#include <imgui_impl/imgui_impl_glfw_gl3.h>
#include <imgui_impl/style.h>
#include <imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>


struct UIController::wnd_t {
    bool show_style_editor = false;
    bool show_fps_overlay = true;
    bool show_info = true;
    bool show_playback_control = true;
    bool show_ui_help = false;
    bool show_shortcuts_help = false;
};

UIController::UIController(Camera *camera)
    : camera_(camera) {
    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(glfwGetCurrentContext(), true);
    wnd_ = std::make_unique<wnd_t>();

    setup_custom_style(false);

    auto &io = ImGui::GetIO();
    io.IniFilename = "rewindviewer.ini";
}

UIController::~UIController() {
    // Cleanup imgui
    ImGui_ImplGlfwGL3_Shutdown();
}

void UIController::next_frame(Scene *scene) {
    // Start new frame
    ImGui_ImplGlfwGL3_NewFrame();

    //Update windows status
    main_menu_bar();

    if (ImGui::BeginMainMenuBar()) {
        ImGui::EndMainMenuBar();
    }

    if (wnd_->show_fps_overlay) {
        fps_overlay_widget();
    }
    if (wnd_->show_info) {
        info_widget(scene);
    }
    if (wnd_->show_playback_control) {
        playback_control_widget(scene);
    }
    if (wnd_->show_style_editor) {
        ImGui::Begin("Style editor", &wnd_->show_style_editor);
        ImGui::ShowStyleEditor();
        ImGui::End();
    }
    if (wnd_->show_ui_help) {
        ImGui::Begin("UI Help", &wnd_->show_ui_help, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::ShowUserGuide();
        ImGui::End();
    }
    if (wnd_->show_shortcuts_help) {
        ImGui::Begin("Viewer Help", &wnd_->show_shortcuts_help, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::BulletText("Mouse drag on map to move camera");
        ImGui::BulletText("Mouse wheel to zoom");
        ImGui::BulletText("Space to play/stop frame playback");
        ImGui::BulletText("Left, right arrow to manually change frames");
        ImGui::BulletText("Esc to close application");
        ImGui::End();
    }

    //Checking hotkeys
    check_hotkeys();

    //Background color
    glClearColor(clear_color_.r, clear_color_.g, clear_color_.b, 1.0f);
}

void UIController::frame_end() {
    ImGui::Render();
}

bool UIController::close_requested() {
    return request_exit_;
}

void UIController::check_hotkeys() {
    const auto &io = ImGui::GetIO();
    if (!io.WantTextInput) {
        if (io.KeysDown[GLFW_KEY_SPACE]) {
            if (!space_pressed_) {
                space_pressed_ = true;
                autoplay_scene_ = !autoplay_scene_;
            }
        } else {
            space_pressed_ = false;
        }
    }

    request_exit_ = io.KeysDown[GLFW_KEY_ESCAPE];
}

void UIController::main_menu_bar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("View", true)) {
            ImGui::Checkbox("Style editor", &wnd_->show_style_editor);
            ImGui::Separator();
            ImGui::Checkbox("FPS overlay", &wnd_->show_fps_overlay);
            ImGui::Checkbox("Utility window", &wnd_->show_info);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help", true)) {
            ImGui::MenuItem("UI help", nullptr, &wnd_->show_ui_help);
            ImGui::MenuItem("Viewer help", nullptr, &wnd_->show_shortcuts_help);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void UIController::fps_overlay_widget() {
    ImGui::SetNextWindowPos(ImVec2(10, 20));
    const auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                       | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("FPS Overlay", &wnd_->show_fps_overlay, ImVec2(0, 0), 0.3f, flags)) {
        ImGui::BeginGroup();
        ImGui::TextColored({1.0, 1.0, 0.0, 1.0}, "FPS %.1f", ImGui::GetIO().Framerate);
        ImGui::SameLine();
        ImGui::Text("[%.1f ms]", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::EndGroup();
        ImGui::End();
    }

}

void UIController::info_widget(Scene *scene) {
    ImGui::SetNextWindowPos({900, 20}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({300, 750}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Info", &wnd_->show_info, ImGuiWindowFlags_NoTitleBar);
    const auto flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::CollapsingHeader("Settings")) {
        if (ImGui::CollapsingHeader("Camera", flags)) {
            ImGui::PushItemWidth(150);
            ImGui::InputFloat2("Position", glm::value_ptr(camera_->pos_), 1);
            ImGui::InputFloat("Viewport size", &camera_->opt_.viewport_size_, 50.0, 1000.0, 0);
            ImGui::PopItemWidth();
        }
        if (ImGui::CollapsingHeader("Colors", flags)) {
            ImGui::ColorEdit3("Background", glm::value_ptr(clear_color_));
        }
    }
    if (ImGui::CollapsingHeader("Frame message", flags)) {
        ImGui::BeginChild("FrameMsg", {0, 0}, true);
        ImGui::TextWrapped("%s", scene->get_frame_user_message());
        ImGui::EndChild();
    }
    ImGui::End();
}

void UIController::playback_control_widget(Scene *scene) {
    static const ImVec2 button_size(0, 20);
    auto &io = ImGui::GetIO();
    auto height = button_size.y;
    auto width = io.DisplaySize.x;

    ImGui::SetNextWindowPos({0, io.DisplaySize.y - height - 2 * ImGui::GetStyle().WindowPadding.y});
    ImGui::SetNextWindowSize({width, -1});
    static const auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                              | ImGuiWindowFlags_NoMove
                              | ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("Playback control", &wnd_->show_playback_control, flags)) {
        ImGui::BeginGroup();

        int tick = scene->get_frame_index();

        if (!io.WantTextInput) {
            if (io.KeysDown[GLFW_KEY_LEFT] || io.KeysDown[GLFW_KEY_RIGHT]) {
                tick += io.KeysDown[GLFW_KEY_RIGHT] ? 1 : -1;
            }
        }

        if (ImGui::ButtonEx("Prev", button_size, ImGuiButtonFlags_Repeat)) {
            --tick;
        }
        ImGui::SameLine();

        if (autoplay_scene_) {
            autoplay_scene_ = !ImGui::Button("Pause", button_size);
        } else {
            autoplay_scene_ = ImGui::Button("Play", button_size);
        }
        ImGui::SameLine();

        if (ImGui::ButtonEx("Next", button_size, ImGuiButtonFlags_Repeat)) {
            ++tick;
        }
        ImGui::SameLine();

        tick += autoplay_scene_;

        //Make tick 1-indexed to better user view
        const auto frames_cnt = scene->get_frames_count();
        tick = std::min(tick + 1, frames_cnt);

        float ftick = tick;
        ImGui::PushItemWidth(-1);
        if (ImGui::TickBar("##empty", &ftick, 1, frames_cnt, {0.0f, 0.0f})) {
            tick = static_cast<int>(ftick);
        }
        ImGui::PopItemWidth();

        scene->set_frame_index(tick - 1);

        ImGui::EndGroup();
        ImGui::End();
    }
}
