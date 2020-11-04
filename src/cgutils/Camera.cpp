//
// Created by valdemar on 18.10.17.
//

#include "opengl.h"

#include "Camera.h"
#include "utils.h"

#include <imgui.h>

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {

const float VIEWPORT_MIN_SIZE = 1.0f;

bool is_ok_viewport(float viewport) {
    return viewport >= VIEWPORT_MIN_SIZE;
}

}  // namespace

Camera::Camera(const Config::CameraConf &conf)
    : conf_(conf), pos_{conf.start_position}, viewport_size_{conf.start_viewport_size} {
    update_matrix();
}

const glm::mat4 &Camera::proj_view() const {
    return pr_view_;
}

void Camera::update() {
    const auto &io = ImGui::GetIO();
    static const float zoom_speed = 0.1;

    if (!io.WantCaptureMouse) {
        // Zoom
        if (io.MouseWheel != 0.0) {
            const float zoom_k = 1.0f - zoom_speed * io.MouseWheel;
            const glm::vec2 rel_mouse_pos = screen2world(io.MousePos) - pos_;
            const glm::vec2 new_mouse_pos = rel_mouse_pos * zoom_k;

            if (is_ok_viewport(viewport_size_ * zoom_k)) {
                pos_ += rel_mouse_pos - new_mouse_pos;
                viewport_size_ *= zoom_k;
            }
        }

        // Map dragging
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            int width, height;
            glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);

            const int min_size = std::min(width, height);

            glm::vec2 delta = io.MouseDelta;
            delta.x = -viewport_size_ * (delta.x / static_cast<float>(min_size));
            delta.y = viewport_size_ * (delta.y / static_cast<float>(min_size)) *
                      static_cast<float>(y_axes_invert());
            pos_ += delta;
        }
    }
    update_matrix();
}

glm::vec2 Camera::screen2world(const glm::vec2 &coord) const {
    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);

    const float half_view = viewport_size_ * 0.5f;
    const float min_size = std::min<float>(width, height);
    const float x_half_view = half_view * static_cast<float>(width) / min_size;
    const float y_half_view = half_view * static_cast<float>(height) / min_size;

    const double proj_x = cg::lerp(coord.x, 0, width, -x_half_view, x_half_view);
    const double proj_y = cg::lerp(coord.y, height, 0, -y_half_view, y_half_view);

    return pos_ + glm::vec2{proj_x, proj_y * y_axes_invert()};
}

int Camera::y_axes_invert() const {
    return conf_.origin_on_top_left ? -1 : 1;
}

void Camera::update_matrix() {
    if (!is_ok_viewport(viewport_size_)) {
        viewport_size_ = VIEWPORT_MIN_SIZE;
    }

    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);

    const float half_view = viewport_size_ * 0.5f;
    const float min_size = std::min<float>(width, height);
    const float x_half_view = half_view * width / min_size;
    const float y_half_view = half_view * height / min_size;

    pr_view_ = glm::ortho(pos_.x - x_half_view, pos_.x + x_half_view,
                          pos_.y - y_half_view * y_axes_invert(),
                          pos_.y + y_half_view * y_axes_invert(), -1.0f, 1.0f);
}
