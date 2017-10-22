//
// Created by valdemar on 18.10.17.
//

#include "Camera.h"

#include <cgutils/utils.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {

glm::vec3 calc_front_dir(float yaw, float pitch) {
    glm::vec3 front;
    front.x = static_cast<float>(cos(glm::radians(pitch)) * cos(glm::radians(yaw)));
    front.y = static_cast<float>(cos(glm::radians(pitch)) * sin(glm::radians(yaw)));
    front.z = static_cast<float>(sin(glm::radians(pitch)));

    return glm::normalize(front);
}

glm::vec3 remove_z(glm::vec3 vec) {
    return {vec.x, vec.y, 0.0f};
}

} // anonymous namespace

Camera::Camera(const glm::vec3 &initial_pos, const glm::vec3 &up, float yaw, float pitch, float move_speed)
    : pos_(initial_pos)
    , up_dir_(up)
    , yaw_(yaw)
    , pitch_(pitch)
    , fake_up_(up)
{
    opt_.speed_per_second = move_speed;
    front_dir_ = calc_front_dir(yaw_, pitch_);
}

void Camera::forward() {
    pos_ += remove_z(front_dir_ * move_per_frame_);
}

void Camera::backward() {
    pos_ -= remove_z(front_dir_ * move_per_frame_);
}

void Camera::left() {
    pos_ += remove_z(glm::normalize(glm::cross(up_dir_, front_dir_)) * move_per_frame_);
}

void Camera::right() {
    pos_ -= remove_z(glm::normalize(glm::cross(up_dir_, front_dir_)) * move_per_frame_);
}

void Camera::update(float fps, float mouse_wheel) {
    move_per_frame_ = opt_.speed_per_second / fps;

    //Should depend from grid size
    auto new_pos = pos_ + front_dir_ * mouse_wheel;
    if (new_pos.z > opt_.min_z && new_pos.z < opt_.max_z ) {
        pos_ = new_pos;
        const float offset = pos_.z;
        pos_.x = cg::clamp(pos_.x, offset, 100.0f + offset);
        pos_.y = cg::clamp(pos_.y, -offset, 100.0f - offset);
    }

    const glm::vec3 right = glm::normalize(glm::cross(fake_up_, front_dir_));
    up_dir_ = glm::normalize(glm::cross(front_dir_, right));
    view_ = glm::lookAt(pos_, pos_ + front_dir_, up_dir_);
}

const glm::mat4 &Camera::view() const {
    return view_;
}

