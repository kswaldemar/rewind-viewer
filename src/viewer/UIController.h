//
// Created by valdemar on 14.10.17.
//

#pragma once

#include <imgui.h>

#include <cgutils/Camera.h>
#include <net/NetListener.h>
#include <viewer/Config.h>

#include <memory>

class Scene;

/**
 * Class for all ui interaction using ImGui
 */
class UIController {
 public:
    UIController(Camera *camera, Config *conf);
    ~UIController();

    /**
     * Call ImGui next frame and clear up OpenGl background
     */
    void next_frame(Scene *scene, NetListener::ConStatus client_status);
    void frame_end();

    bool close_requested() const;

    bool immediate_mode_enabled() const;

 private:
    void main_menu_bar();

    void fps_overlay_widget(NetListener::ConStatus net_status);
    void info_widget(Scene *scene);
    void playback_control_widget(Scene *scene);

    bool key_pressed_once(int key_desc);

    /// Handling flags whenever window should be drawn or not etc.
    struct wnd_t;
    std::unique_ptr<wnd_t> wnd_;

    Camera *camera_;
    Config *conf_;

    bool request_exit_ = false;
    bool autoplay_scene_ = true;
    bool developer_mode_ = false;
    bool immediate_send_mode_ = false;

    /// Last remembered state
    bool key_pressed_[512] = {};
};
