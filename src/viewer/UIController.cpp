//
// Created by valdemar on 14.10.17.
//

#include "UIController.h"

#include <imgui_impl/imgui_widgets.h>
#include <imgui_impl/imgui_impl_glfw_gl3.h>
#include <imgui_impl/style.h>
#include <fontawesome.h>

#include <glm/gtc/type_ptr.hpp>

struct UIController::wnd_t {
    bool show_style_editor = false;
    bool show_fps_overlay = true;
    bool show_info = true;
    bool show_playback_control = true;
    bool show_ui_help = false;
    bool show_shortcuts_help = false;
    bool show_metrics = false;
    bool show_mouse_pos_tooltip = false;
};

UIController::UIController(Camera *camera)
    : camera_(camera) {
    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(glfwGetCurrentContext(), true);
    wnd_ = std::make_unique<wnd_t>();

    setup_custom_style(false);

    auto &io = ImGui::GetIO();

    io.IniFilename = "rewindviewer.ini";

    //Load and merge fontawesome to current font
    io.Fonts->AddFontDefault();
    const ImWchar icons_range[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF("resources/fonts/fontawesome-webfont.ttf", 14.0f, &icons_config, icons_range);
    //Need to call it here, otherwise fontawesome glyph ranges would be corrupted on Windows
    ImGui_ImplGlfwGL3_CreateDeviceObjects();
}

UIController::~UIController() {
    // Cleanup imgui
    ImGui_ImplGlfwGL3_Shutdown();
}

void UIController::next_frame(Scene *scene, NetListener::ConStatus client_status) {
    // Start new frame
    ImGui_ImplGlfwGL3_NewFrame();

    //Clear data option
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu(ICON_FA_PENCIL_SQUARE_O " Edit", true)) {
            //if (ImGui::MenuItem(ICON_FA_RECYCLE " Clear frames data", "CTRL+R", false, scene->has_data())) {
            //    scene->clear_data(true);
            //}
            ImGui::Checkbox("Close window by Escape key", &close_with_esc_);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    //Update windows status
    main_menu_bar();

    if (wnd_->show_fps_overlay) {
        fps_overlay_widget(client_status);
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
    if (wnd_->show_metrics) {
        ImGui::ShowMetricsWindow(&wnd_->show_metrics);
    }
    if (wnd_->show_ui_help) {
        ImGui::Begin(ICON_FA_INFO_CIRCLE " UI guide", &wnd_->show_ui_help, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::ShowUserGuide();
        ImGui::End();
    }
    if (wnd_->show_shortcuts_help) {
        ImGui::Begin(ICON_FA_KEYBOARD_O " Controls help",
                     &wnd_->show_shortcuts_help,
                     ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::BulletText("Mouse drag on map to move camera");
        ImGui::BulletText("Mouse wheel to zoom");
        ImGui::BulletText("Space - play/stop frame playback");
        ImGui::BulletText("Left, right arrow - manually change frames");
        ImGui::BulletText("Esc - close application");
        ImGui::BulletText("Ctrl+g - go to tick");
        ImGui::BulletText("g - Toggle grid draw state");
        ImGui::BulletText("p - Show tooltip with cursor world coordinates");
        ImGui::BulletText("1-5 - Toggle layers visibility");

        ImGui::End();
    }

    const auto &io = ImGui::GetIO();
    if (!io.WantCaptureMouse && wnd_->show_mouse_pos_tooltip) {
        ImGui::BeginTooltip();
        auto wmouse_pos = camera_->screen2world(io.MousePos);
        ImGui::Text("(%.3f, %.3f)", wmouse_pos.x, wmouse_pos.y);
        ImGui::EndTooltip();
    }

    //Checking hotkeys
    if (!io.WantTextInput) {
        if (key_pressed_once(GLFW_KEY_SPACE)) {
            autoplay_scene_ = !autoplay_scene_;
        }
        if (key_pressed_once(GLFW_KEY_G) && !io.KeyCtrl) {
            scene->opt().show_grid = !scene->opt().show_grid;
        }
        if (key_pressed_once(GLFW_KEY_P)) {
            wnd_->show_mouse_pos_tooltip = !wnd_->show_mouse_pos_tooltip;
        }
        if (io.KeysDown[GLFW_KEY_D] && io.KeyCtrl) {
            developer_mode_ = true;
        }
        auto &enabled_layers = scene->opt().enabled_layers;
        for (size_t layer_idx = 0; layer_idx < enabled_layers.size(); ++layer_idx) {
            if (key_pressed_once(static_cast<int>(GLFW_KEY_1 + layer_idx))) {
                enabled_layers[layer_idx] = !enabled_layers[layer_idx];
            }
        }
        if (scene->has_data() && io.KeysDown[GLFW_KEY_R] && io.KeyCtrl) {
            scene->clear_data(false);
        }
    }

    request_exit_ = close_with_esc_ && io.KeysDown[GLFW_KEY_ESCAPE];

    //Hittest for detailed unit info
    if (!ImGui::GetIO().WantCaptureMouse) {
        scene->show_detailed_info(camera_->screen2world(ImGui::GetIO().MousePos));
    }

    //Background color
    glClearColor(clear_color_.r, clear_color_.g, clear_color_.b, 1.0f);
}

void UIController::frame_end() {
    ImGui::Render();
}

bool UIController::close_requested() {
    return request_exit_;
}

void UIController::main_menu_bar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu(ICON_FA_EYE " View", true)) {
            ImGui::Checkbox("FPS overlay", &wnd_->show_fps_overlay);
            ImGui::Checkbox("Utility window", &wnd_->show_info);
            if (developer_mode_) {
                ImGui::Separator();
                ImGui::Checkbox("Style editor", &wnd_->show_style_editor);
                ImGui::Checkbox("Metrics", &wnd_->show_metrics);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(ICON_FA_QUESTION_CIRCLE_O " Help", true)) {
            ImGui::MenuItem(ICON_FA_INFO_CIRCLE " UI guide", nullptr, &wnd_->show_ui_help);
            ImGui::MenuItem(ICON_FA_KEYBOARD_O " Controls", nullptr, &wnd_->show_shortcuts_help);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void UIController::fps_overlay_widget(NetListener::ConStatus net_status) {
    ImGui::SetNextWindowPos(ImVec2(10, 20));
    const auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                       | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("FPS Overlay", &wnd_->show_fps_overlay, ImVec2{0, 0}, 0.3, flags)) {
        ImGui::BeginGroup();
        ImGui::TextColored({1.0, 1.0, 0.0, 1.0}, "FPS %.1f", ImGui::GetIO().Framerate);
        ImGui::SameLine();
        ImGui::Text("[%.1f ms]", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::EndGroup();
        std::string strstatus;
        ImVec4 color;
        static const float intensity = 1.0;
        switch (net_status) {
            case NetListener::ConStatus::WAIT:
                strstatus = "WAITING";
                color = {intensity, intensity, 0.0, 1.0};
                break;
            case NetListener::ConStatus::ESTABLISHED:
                strstatus = "CONNECTED";
                color = {0.0, intensity, 0.0, 1.0};
                break;
            case NetListener::ConStatus::CLOSED:
                strstatus = "CLOSED";
                color = {intensity, 0.0, 0.0, 1.0};
                break;
        }
        ImGui::TextColored(color, ICON_FA_PLUG " %s", strstatus.c_str());
        ImGui::End();
    }
}

void UIController::info_widget(Scene *scene) {
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    const float desired_width = 300;
    ImGui::SetNextWindowPos({width - desired_width, 20}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({desired_width, static_cast<float>(height - 20 - 30)}, ImGuiCond_Always);
    ImGui::Begin("Info", &wnd_->show_info, ImGuiWindowFlags_NoTitleBar);
    const auto flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::CollapsingHeader(ICON_FA_COGS " Settings")) {
        if (ImGui::CollapsingHeader(ICON_FA_VIDEO_CAMERA " Camera", flags)) {
            ImGui::PushItemWidth(150);
            ImGui::InputFloat2("Position", glm::value_ptr(camera_->pos_), 1);
            ImGui::InputFloat("Viewport size", &camera_->opt().viewport_size, 50.0, 1000.0, 0);
            ImGui::PopItemWidth();
        }
        if (ImGui::CollapsingHeader(ICON_FA_EYEDROPPER " Colors", flags)) {
            ImGui::SetColorEditOptions(ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit3("Background", glm::value_ptr(clear_color_));
            ImGui::ColorEdit3("Grid", glm::value_ptr(scene->opt().grid_color));
            ImGui::ColorEdit3("Canvas", glm::value_ptr(scene->opt().scene_color));
        }
        if (ImGui::CollapsingHeader(ICON_FA_MAP_O " Options", flags)) {
            ImGui::Checkbox("World origin on top left", &camera_->opt().origin_on_top_left);
            ImGui::Checkbox("Draw grid", &scene->opt().show_grid);
            static const ImVec4 button_colors[] = {
                ImVec4(0.5, 0.5, 0.5, 1.0),
                ImVec4(0.58, 0.941, 0.429, 1.0)
            };
            size_t idx = 0;
            static const std::array<const char *, static_cast<size_t>(Frame::LAYERS_COUNT)> captions{
                {"##layer0", "##layer1", "##layer2", "##layer3", "##layer4"}
            };
            for (bool &enabled : scene->opt().enabled_layers) {
                if (ImGui::ColorButton(captions[idx], button_colors[enabled], ImGuiColorEditFlags_NoTooltip)) {
                    enabled = !enabled;
                }
                ++idx;
                ImGui::SameLine();
            }
            ImGui::LabelText("##layers", "%s", "Visible layers");
        }
    }
    if (ImGui::CollapsingHeader(ICON_FA_COMMENT_O " Frame message", flags)) {
        ImGui::BeginChild("FrameMsg", {0, 0}, true);
        ImGui::TextWrapped("%s", scene->get_frame_user_message());
        ImGui::EndChild();
    }
    ImGui::End();
}

void UIController::playback_control_widget(Scene *scene) {
    static const auto button_size = ImVec2{0, 0};
    static const float buttons_spacing = 5.0f;
    auto &io = ImGui::GetIO();
    auto width = io.DisplaySize.x;
    ImGui::SetNextWindowPos({0, io.DisplaySize.y - 20 - 2 * ImGui::GetStyle().WindowPadding.y});
    ImGui::SetNextWindowSize({width, 30});
    static const auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                              | ImGuiWindowFlags_NoMove
                              | ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("Playback control", &wnd_->show_playback_control, flags)) {
        ImGui::BeginGroup();

        int tick = scene->get_frame_index();

        if (!io.WantTextInput) {
            if (io.KeyCtrl) {
                tick -= key_pressed_once(GLFW_KEY_LEFT);
                tick += key_pressed_once(GLFW_KEY_RIGHT);
            } else {
                tick -= io.KeysDown[GLFW_KEY_LEFT];
                tick += io.KeysDown[GLFW_KEY_RIGHT];
            }
        }

        ImGui::Button(ICON_FA_FAST_BACKWARD "##fastprev", button_size);
        if (ImGui::IsItemActive()) {
            tick -= fast_skip_speed_;
        }
        ImGui::SameLine(0.0f, buttons_spacing);

        if (ImGui::Button(ICON_FA_BACKWARD "##prev", button_size)) {
            --tick;
        }
        ImGui::SameLine(0.0f, buttons_spacing);

        if (autoplay_scene_) {
            autoplay_scene_ = !ImGui::Button(ICON_FA_PAUSE "##pause", button_size);
        } else {
            autoplay_scene_ = ImGui::Button(ICON_FA_PLAY "##play", button_size);
        }
        ImGui::SameLine(0.0f, buttons_spacing);

        if (ImGui::Button(ICON_FA_FORWARD "##next", button_size)) {
            ++tick;
        }
        ImGui::SameLine(0.0f, buttons_spacing);

        ImGui::Button(ICON_FA_FAST_FORWARD "##fastnext", button_size);
        if (ImGui::IsItemActive()) {
            tick += fast_skip_speed_;
        }
        ImGui::SameLine();

        tick += autoplay_scene_;

        const auto frames_cnt = scene->get_frames_count();
        if (frames_cnt > 0) {
            tick = std::min(tick, frames_cnt);
            float ftick = tick;
            ImGui::PushItemWidth(-1);
            if (io.KeyCtrl && io.KeysDown[GLFW_KEY_G]) {
                ImGui::SetKeyboardFocusHere();
            }
            if (ImGui::TickBar("##empty", &ftick, 0, frames_cnt, {0.0f, 0.0f})) {
                tick = static_cast<int>(ftick);
            }
            ImGui::PopItemWidth();
            scene->set_frame_index(tick);
        } else {
            ImGui::Text("Frame list empty");
        }

        ImGui::EndGroup();
        ImGui::End();
    }
}

bool UIController::key_pressed_once(int key_desc) {
    const auto &io = ImGui::GetIO();
    if (io.KeysDown[key_desc]) {
        if (!key_pressed_[key_desc]) {
            key_pressed_[key_desc] = true;
            return true;
        }
    } else {
        key_pressed_[key_desc] = false;
    }
    return false;
}
