//
// Created by valdemar on 14.10.17.
//

#pragma once

#include <imgui.h>
#include <GLFW/glfw3.h>

#include <cgutils/Camera.h>

#include <memory>

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
    void next_frame();
    void frame_end();

    bool close_requested();

private:
    ///Draw all enabled windows
    void update_windows();

    void debug_window();
    void fps_overlay_window();
    void check_input();

    ///Handling flags whenever window should be drawed or not etc.
    struct wnd_t;
    std::unique_ptr<wnd_t> wnd_;

    ImVec4 clear_color_ = ImColor(114, 144, 154);

    Camera *camera_;

    bool request_exit_ = false;
};




