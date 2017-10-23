//
// Created by valdemar on 14.10.17.
//

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>

#include <cgutils/Camera.h>

#include <memory>
#include "Scene.h"

/**
 * Class for all ui interaction using ImGui
 */
class UIController {
public:
    UIController(GLFWwindow *window, Camera *camera);
    ~UIController();

    /**
     * Call ImGui next frame and clear up OpenGl background
     */
    void next_frame(Scene *scene);
    void frame_end();

    bool close_requested();

private:
    void check_hotkeys();

    void main_menu_bar();

    void fps_overlay_window();
    void camera_settings_window();
    void playback_control_window(Scene *scene);
    void user_message_window(Scene *scene);

    ///Handling flags whenever window should be drawed or not etc.
    struct wnd_t;
    std::unique_ptr<wnd_t> wnd_;

    Camera *camera_;

    bool request_exit_ = false;
    bool autoplay_scene_ = true;
    bool space_pressed_ = false;

};




