//
// Created by valdemar on 14.10.17.
//

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>

#include <cgutils/Camera.h>
#include <net/NetListener.h>

#include <memory>

class Scene;

/**
 * Class for all ui interaction using ImGui
 */
class UIController {
public:
    UIController(Camera *camera);
    ~UIController();

    /**
     * Call ImGui next frame and clear up OpenGl background
     */
    void next_frame(Scene *scene, NetListener::ConStatus client_status);
    void frame_end();

    bool close_requested();

private:
    void check_hotkeys();

    void main_menu_bar();

    void fps_overlay_widget(NetListener::ConStatus net_status);
    void info_widget(Scene *scene);
    void playback_control_widget(Scene *scene);

    ///Handling flags whenever window should be drawed or not etc.
    struct wnd_t;
    std::unique_ptr<wnd_t> wnd_;

    Camera *camera_;

    bool request_exit_ = false;
    bool autoplay_scene_ = true;
    bool space_pressed_ = false;
    bool developer_mode_ = false;

    const uint16_t fast_skip_speed_ = 20; //In ticks per frame

    glm::vec3 clear_color_ = {0.673, 0.764, 0.794};
};
