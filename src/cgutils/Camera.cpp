//
// Created by valdemar on 18.10.17.
//

#include "Camera.h"

#include <cgutils/utils.h>

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
    opt_.viewport_size_ = viewport_size;
    update_matrix();
}

const glm::mat4 &Camera::proj_view() const {
    return pr_view_;
}

void Camera::update() {
    const auto &io = ImGui::GetIO();

    const float zoom_speed = 0.1;
    const float diff_sense = 0.1;


    //Should depend from grid size
    if (!ImGui::IsAnyItemActive()) {
        int width, height;
        glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);

        if (io.MouseWheel != 0.0) {
            glm::vec2 des_pos = {(io.MousePos.x / width) * 2.0f - 1.0f,
                                 1.0f - (io.MousePos.y / height) * 2.0f};
            des_pos = glm::inverse(pr_view_) * glm::vec4{des_pos.x, des_pos.y, 0.0f, 1.0f};
            glm::vec2 diff = des_pos - pos_;

            if (io.MouseWheel > 0) {
                glm::vec2 shift = diff * 0.1f * io.MouseWheel;
                //Adjust cursor and position if zoom in
                pos_ += shift;

                //TODO: Not exact zoom in, bug somewhere
                shift = pr_view_ * glm::vec4{shift.x, shift.y, 0.0f, 0.0f};
                shift.x = (shift.x + 1.0f) / 2.0f * width - 0.5f * width;
                shift.y = -(shift.y - 1.0f) / 2.0f * height - 0.5f * height;

                ImVec2 new_cursor = {io.MousePos.x - shift.x * diff_sense, io.MousePos.y - shift.y * diff_sense};
                glfwSetCursorPos(glfwGetCurrentContext(), new_cursor.x, new_cursor.y);
            }
            opt_.viewport_size_ -= (opt_.viewport_size_ * zoom_speed) * io.MouseWheel;
        }

        //Map dragging
        if (ImGui::IsMouseDragging()) {
            glm::vec2 delta = {io.MouseDelta.x, io.MouseDelta.y};
            delta.x = -opt_.viewport_size_ * (delta.x / width);
            delta.y = opt_.viewport_size_ * (delta.y / height);
            pos_ += delta;
        }

        update_matrix();
    }
}

void Camera::update_matrix() {
    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);

    const float half_view = opt_.viewport_size_ * 0.5f;
    const float min_size = std::min<float>(width, height);
    const float x_half_view = half_view * width / min_size;
    const float y_half_view = half_view * height / min_size;

    pr_view_ = glm::ortho(pos_.x - x_half_view, pos_.x + x_half_view,
                          pos_.y - y_half_view, pos_.y + y_half_view,
                          -10.0f, 10.0f);
}
