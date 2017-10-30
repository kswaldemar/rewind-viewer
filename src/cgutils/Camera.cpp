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

    //Should depend from grid size
    if (!io.WantCaptureMouse) {
        int width, height;
        glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);

        if (io.MouseWheel != 0.0) {
            const float half_view = opt_.viewport_size * 0.5f;
            const float min_size = std::min<float>(width, height);
            const float x_half_view = half_view * width / min_size;
            const float y_half_view = half_view * height / min_size;

            const double mouse_x = cg::lerp(io.MousePos.x, 0, width, -x_half_view, x_half_view);
            const double mouse_y = cg::lerp(io.MousePos.y, height, 0, -y_half_view, y_half_view);

            const double zoom_k = (1 - zoom_speed * io.MouseWheel);

            const double new_mouse_x = mouse_x * zoom_k;
            const double new_mouse_y = mouse_y * zoom_k;

            pos_.x += mouse_x - new_mouse_x;
            pos_.y += mouse_y - new_mouse_y;

            opt_.viewport_size *= zoom_k;
        }

        //Map dragging
        if (ImGui::IsMouseDragging()) {
            glm::vec2 delta = {io.MouseDelta.x, io.MouseDelta.y};
            delta.x = -opt_.viewport_size * (delta.x / width);
            delta.y = opt_.viewport_size * (delta.y / height);
            pos_ += delta;
        }

        update_matrix();
    }
}

void Camera::update_matrix() {
    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);

    const float half_view = opt_.viewport_size * 0.5f;
    const float min_size = std::min<float>(width, height);
    const float x_half_view = half_view * width / min_size;
    const float y_half_view = half_view * height / min_size;

    pr_view_ = glm::ortho(pos_.x - x_half_view, pos_.x + x_half_view,
                          pos_.y - y_half_view, pos_.y + y_half_view,
                          -10.0f, 10.0f);
}
