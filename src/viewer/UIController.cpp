//
// Created by valdemar on 14.10.17.
//

#include <cgutils/opengl.h>

#include "UIController.h"

#include <common/logger.h>
#include <version.h>

#include <fontawesome.h>
#include <imgui_impl/imgui_impl_glfw.h>
#include <imgui_impl/imgui_impl_opengl3.h>

#include <glm/gtc/type_ptr.hpp>

namespace {

bool key_modifier(const ImGuiIO &io) {
#ifdef __APPLE__
    return io.KeySuper;
#else
    return io.KeyCtrl;
#endif
}

float get_scale_factor() {
    int w, h;
    int display_w, display_h;

    auto window = glfwGetCurrentContext();

    glfwGetWindowSize(window, &w, &h);
    glfwGetFramebufferSize(window, &display_w, &display_h);

    return std::max(static_cast<float>(display_w) / static_cast<float>(w),
                    static_cast<float>(display_h) / static_cast<float>(h));
}

const float DEFAULT_FONT_SIZE = 13.0f;
const float FONT_AWESOME_FONT_SIZE = 14.0f;

const char *THEMES_COMBO = "Light\0Grey\0Dark\0";
void set_style_by_theme_id(int theme_id) {
    switch (theme_id) {
        case 0: ImGui::StyleColorsLight(); break;
        case 1: ImGui::StyleColorsClassic(); break;
        case 2: ImGui::StyleColorsDark(); break;
        default: LOG_WARN("Incorrect theme_id=%d", theme_id);
    }

    // Custom changes
    auto &style = ImGui::GetStyle();
    style.Alpha = 1.0f;
    style.FrameRounding = 3.0f;
    style.WindowPadding.y = 7;
    style.WindowRounding = 4;
    style.GrabRounding = 2;
    style.WindowBorderSize = 0.0f;
}

}  // namespace

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

UIController::UIController(Camera *camera, Config *conf) : camera_(camera), conf_(conf) {
    // Setup ImGui binding
    ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
    ImGui_ImplOpenGL3_Init();
    wnd_ = std::make_unique<wnd_t>();

    set_style_by_theme_id(conf_->ui.imgui_theme_id);

    auto &io = ImGui::GetIO();
    const float scale_factor = get_scale_factor();

    auto font_cfg = ImFontConfig();
    font_cfg.SizePixels = DEFAULT_FONT_SIZE * scale_factor;
    font_cfg.OversampleH = 1;
    font_cfg.OversampleV = 1;
    font_cfg.PixelSnapH = true;

    // Load and merge fontawesome to current font
    io.Fonts->AddFontDefault(&font_cfg);
    const ImWchar icons_range[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF("resources/fonts/fontawesome-webfont.ttf",
                                 FONT_AWESOME_FONT_SIZE * scale_factor, &icons_config, icons_range);
    io.FontGlobalScale = 1.0f / scale_factor;
    // Need to call it here, otherwise fontawesome glyph ranges would be corrupted on Windows
    ImGui_ImplOpenGL3_CreateDeviceObjects();
}

UIController::~UIController() {
    // Cleanup imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UIController::next_frame(Scene *scene, NetListener::ConStatus client_status) {
    // Start new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Clear data option
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu(ICON_FA_PENCIL_SQUARE_O " Preferences", true)) {
            ImGui::Checkbox("Close window by Escape key", &conf_->ui.close_with_esc);
            ImGui::Checkbox("Update window when not in focus", &conf_->ui.update_unfocused);
            ImGui::Separator();
            ImGui::Checkbox("Immediate mode", &immediate_send_mode_);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Update windows status
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
        ImGui::Begin(ICON_FA_INFO_CIRCLE " UI guide", &wnd_->show_ui_help,
                     ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::ShowUserGuide();
        ImGui::End();
    }
    if (wnd_->show_shortcuts_help) {
        ImGui::Begin(ICON_FA_KEYBOARD_O " Controls help", &wnd_->show_shortcuts_help,
                     ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::BulletText("Mouse drag on map to move camera");
        ImGui::BulletText("Mouse wheel to zoom");
        ImGui::BulletText("Space - play/stop frame playback");
        ImGui::BulletText(ICON_FA_ARROW_LEFT ", " ICON_FA_ARROW_RIGHT
                                             " - manually change frames\n"
                                             "press with modkey to change slowly");
        ImGui::BulletText("Esc - close application");
        ImGui::BulletText("g - Toggle grid draw state");
        ImGui::BulletText("i - Toggle immediate send mode");
        ImGui::BulletText("modkey + g - Go to tick");
        ImGui::BulletText("p - Show tooltip with cursor world coordinates");
        ImGui::BulletText("1-0 - Toggle layers visibility");

        ImGui::End();
    }

    const auto &io = ImGui::GetIO();
    if (!io.WantCaptureMouse && wnd_->show_mouse_pos_tooltip) {
        ImGui::BeginTooltip();
        auto wmouse_pos = camera_->screen2world(io.MousePos);
        ImGui::Text("(%.3f, %.3f)", wmouse_pos.x, wmouse_pos.y);
        ImGui::EndTooltip();
    }

    // Checking hotkeys
    if (!io.WantTextInput) {
        if (key_pressed_once(GLFW_KEY_SPACE)) {
            autoplay_scene_ = !autoplay_scene_;
        }
        if (io.KeysDown[GLFW_KEY_LEFT]) {
            autoplay_scene_ = false;
        }
        if (key_pressed_once(GLFW_KEY_G) && !key_modifier(io)) {
            conf_->scene.show_grid = !conf_->scene.show_grid;
        }
        if (key_pressed_once(GLFW_KEY_I)) {
            immediate_send_mode_ = !immediate_send_mode_;
        }
        if (key_pressed_once(GLFW_KEY_P)) {
            wnd_->show_mouse_pos_tooltip = !wnd_->show_mouse_pos_tooltip;
        }
        if (io.KeysDown[GLFW_KEY_D] && key_modifier(io)) {
            developer_mode_ = true;
        }

        // Layer toggle shortcuts
        auto &enabled_layers = conf_->scene.enabled_layers;
        static const std::array<int, Frame::LAYERS_COUNT> layer_shortcuts = {
            GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5,
            GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9, GLFW_KEY_0,
        };
        for (size_t i = 0; i < layer_shortcuts.size(); ++i) {
            if (key_pressed_once(layer_shortcuts[i])) {
                enabled_layers[i] = !enabled_layers[i];
            }
        }

        if (scene->has_data() && io.KeysDown[GLFW_KEY_R] && key_modifier(io)) {
            scene->clear_data();
        }
    }

    request_exit_ = conf_->ui.close_with_esc && io.KeysDown[GLFW_KEY_ESCAPE];

    // Hittest for detailed unit info
    if (!ImGui::GetIO().WantCaptureMouse) {
        scene->show_detailed_info(camera_->screen2world(ImGui::GetIO().MousePos));
    }

    // Background color
    glClearColor(conf_->ui.clear_color.r, conf_->ui.clear_color.g, conf_->ui.clear_color.b, 1.0f);
}

void UIController::frame_end() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool UIController::close_requested() const {
    return request_exit_;
}

bool UIController::immediate_mode_enabled() const {
    return immediate_send_mode_;
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
            // ImGui::MenuItem(ICON_FA_INFO_CIRCLE " UI guide", nullptr, &wnd_->show_ui_help);
            ImGui::MenuItem(ICON_FA_KEYBOARD_O " Controls", nullptr, &wnd_->show_shortcuts_help);
            ImGui::EndMenu();
        }
        ImGui::TextDisabled(ICON_FA_CODE_FORK " v%s", APP_VERSION);
        ImGui::EndMainMenuBar();
    }
}

void UIController::fps_overlay_widget(NetListener::ConStatus net_status) {
    ImGui::SetNextWindowPos(ImVec2(10, 20));
    const auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

    if (wnd_->show_fps_overlay) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.7);
        ImGui::Begin("FPS Overlay", nullptr, flags);
        ImGui::PopStyleVar();

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
        const ImVec4 mode_color = {0.3f, 0.0f, 0.0f, 1.000f};
        if (immediate_mode_enabled()) {
            ImGui::TextColored(mode_color,
                               ICON_FA_EXCLAMATION_CIRCLE " Immediate send mode enabled");
        }
        ImGui::End();
    }
}

void UIController::info_widget(Scene *scene) {
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    const int desired_width = 300;
    ImGui::SetNextWindowPos({static_cast<float>(width - desired_width), 20}, ImGuiCond_None);
    ImGui::SetNextWindowSize({desired_width, static_cast<float>(height - 20 - 30)}, ImGuiCond_None);
    ImGui::Begin(
        "Info", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
    const auto flags = ImGuiTreeNodeFlags_DefaultOpen;
    if (ImGui::CollapsingHeader(ICON_FA_COGS " Settings")) {
        if (ImGui::CollapsingHeader("Camera", flags)) {
            ImGui::PushItemWidth(150);
            ImGui::InputFloat2("Position", glm::value_ptr(camera_->pos_), "%.1f");
            ImGui::InputFloat("Viewport size", &camera_->viewport_size_, 50.0, 1000.0, "%.0f");
            if (ImGui::Button("Save state")) {
                auto &cam_conf = conf_->camera;
                cam_conf.start_position = camera_->pos_;
                cam_conf.start_viewport_size = camera_->viewport_size_;
            }
            ImGui::SameLine();
            if (ImGui::Button("Restore state")) {
                auto &cam_conf = conf_->camera;
                camera_->pos_ = cam_conf.start_position;
                camera_->viewport_size_ = cam_conf.start_viewport_size;
            }
            ImGui::PopItemWidth();
        }
        if (ImGui::CollapsingHeader("Colors", flags)) {
            ImGui::SetColorEditOptions(ImGuiColorEditFlags_NoInputs);
            ImGui::ColorEdit3("Background", glm::value_ptr(conf_->ui.clear_color));
            ImGui::ColorEdit3("Grid", glm::value_ptr(conf_->scene.grid_color));
            ImGui::ColorEdit3("Canvas", glm::value_ptr(conf_->scene.scene_color));
            if (ImGui::Combo("Theme", &conf_->ui.imgui_theme_id, THEMES_COMBO)) {
                set_style_by_theme_id(conf_->ui.imgui_theme_id);
            }
        }
        if (ImGui::CollapsingHeader("Options", flags)) {
            ImGui::Checkbox("World origin on top left", &conf_->camera.origin_on_top_left);
            ImGui::Checkbox("Draw grid", &conf_->scene.show_grid);
        }
    }
    if (ImGui::CollapsingHeader(ICON_FA_MAP_O " Layer visibility", flags)) {
        static const ImVec4 button_colors[] = {ImVec4(0.5, 0.5, 0.5, 1.0),
                                               ImVec4(0.38, 0.741, 0.229, 1.0)};
        size_t idx = 0;
        static const std::array<const char *, static_cast<size_t>(Frame::LAYERS_COUNT)> captions{
            {"##layer0", "##layer1", "##layer2", "##layer3", "##layer4", "##layer5", "##layer6",
             "##layer7", "##layer8", "##layer9"}};
        for (bool &enabled : conf_->scene.enabled_layers) {
            if (ImGui::ColorButton(captions[idx], button_colors[enabled],
                                   ImGuiColorEditFlags_NoTooltip)) {
                enabled = !enabled;
            }
            ++idx;
            if (idx < Frame::LAYERS_COUNT) {
                ImGui::SameLine();
            }
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
    static const auto flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("Playback control", &wnd_->show_playback_control, flags)) {
        ImGui::BeginGroup();

        // Tick is one indexed
        int tick = scene->get_frame_index() + 1;

        if (!io.WantTextInput) {
            int prev_tick = tick;
            if (key_modifier(io)) {
                tick -= key_pressed_once(GLFW_KEY_LEFT);
                tick += key_pressed_once(GLFW_KEY_RIGHT);
            } else {
                tick -= io.KeysDown[GLFW_KEY_LEFT];
                tick += io.KeysDown[GLFW_KEY_RIGHT];
            }

            if (prev_tick != tick) {
                // Manually changed
                autoplay_scene_ = false;
            }
        }

        ImGui::Button(ICON_FA_FAST_BACKWARD "##fastprev", button_size);
        if (ImGui::IsItemActive()) {
            tick -= conf_->ui.fast_skip_speed;
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
            tick += conf_->ui.fast_skip_speed;
        }
        ImGui::SameLine();

        tick += autoplay_scene_;

        const auto frames_cnt = scene->get_frames_count();
        if (frames_cnt > 0) {
            tick = cg::clamp(tick, 1, frames_cnt);
            ImGui::PushItemWidth(-1);
            if (key_modifier(io) && io.KeysDown[GLFW_KEY_G]) {
                ImGui::SetKeyboardFocusHere();
            }
            const std::string slider_fmt = "%5d/" + std::to_string(frames_cnt);
            if (ImGui::SliderInt("##empty", &tick, 1, frames_cnt, slider_fmt.data(),
                                 ImGuiSliderFlags_AlwaysClamp)) {
                autoplay_scene_ = false;
            }
            scene->set_frame_index(tick - 1);
            ImGui::PopItemWidth();
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
