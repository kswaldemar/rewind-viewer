//
// Created by valdemar on 18.10.17.
//

#include "Camera.h"
#include "utils.h"

#include <imgui.h>
#include <imgui_impl/imgui_widgets.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>


namespace {

} // anonymous namespace

Camera::Camera(const glm::vec2 &initial_pos, float viewport_size)
    : pos_(initial_pos)
{
    opt_.viewport_size = viewport_size;
    update_matrix();
}

const glm::mat4 &Camera::proj_view() const {
    return pr_view_;
}

void Camera::update() {
    const auto &io = ImGui::GetIO();
    static const float zoom_speed = 0.1;

    if (!io.WantCaptureMouse) {
        //Zoom
        if (io.MouseWheel != 0.0) {
            const float zoom_k = 1.0f - zoom_speed * io.MouseWheel;
            const glm::vec2 rel_mouse_pos = screen2world(io.MousePos) - pos_;
            const glm::vec2 new_mouse_pos = rel_mouse_pos * zoom_k;

            pos_ += rel_mouse_pos - new_mouse_pos;

            opt_.viewport_size *= zoom_k;
        }

        //Map dragging
        if (ImGui::IsMouseDragging()) {
            int width, height;
            glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);

            glm::vec2 delta = io.MouseDelta;
            delta.x = -opt_.viewport_size * (delta.x / width);
            delta.y = opt_.viewport_size * (delta.y / height) * y_axes_invert();
            pos_ += delta;
        }

        update_matrix();
    }
}

glm::vec2 Camera::screen2world(const glm::vec2 &coord) const {
    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);

    const float half_view = opt_.viewport_size * 0.5f;
    const float min_size = std::min<float>(width, height);
    const float x_half_view = half_view * width / min_size;
    const float y_half_view = half_view * height / min_size;

    const double proj_x = cg::lerp(coord.x, 0, width, -x_half_view, x_half_view);
    const double proj_y = cg::lerp(coord.y, height, 0, -y_half_view, y_half_view);

    return pos_ + glm::vec2{proj_x, proj_y * y_axes_invert()};
}

int Camera::y_axes_invert() const {
    return opt_.origin_on_top_left ? -1 : 1;
}

void Camera::update_matrix() {
    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);

    const float half_view = opt_.viewport_size * 0.5f;
    const float min_size = std::min<float>(width, height);
    const float x_half_view = half_view * width / min_size;
    const float y_half_view = half_view * height / min_size;

    pr_view_ = glm::ortho(pos_.x - x_half_view, pos_.x + x_half_view,
                          pos_.y - y_half_view * y_axes_invert(), pos_.y + y_half_view * y_axes_invert(),
                          -1.0f, 1.0f);
}
