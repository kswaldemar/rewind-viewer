//
// Created by valdemar on 18.10.17.
//

#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static glm::vec3 calc_front_dir(float yaw, float pitch) {
    glm::vec3 front;
    front.x = static_cast<float>(cos(glm::radians(pitch)) * cos(glm::radians(yaw)));
    front.y = static_cast<float>(cos(glm::radians(pitch)) * sin(glm::radians(yaw)));
    front.z = static_cast<float>(sin(glm::radians(pitch)));

    return glm::normalize(front);
}

static Camera *g_this = nullptr;



void Camera::mouse_callback(GLFWwindow *, double xpos, double ypos) {
    g_this->mouse_move(static_cast<float>(xpos), static_cast<float>(ypos));
}

Camera::Camera(const glm::vec3 &initial_pos, const glm::vec3 &up, float yaw, float pitch, float move_speed)
    : pos_(initial_pos)
    , up_dir_(up)
    , yaw_(yaw)
    , pitch_(pitch)
    , des_move_speed_(move_speed)
    , fake_up_(up)
{
    front_dir_ = calc_front_dir(yaw_, pitch_);
    g_this = this;
}

void Camera::forward() {
    pos_ += front_dir_ * move_per_frame_;
}

void Camera::backward() {
    pos_ -= front_dir_ * move_per_frame_;
}

void Camera::up() {
    pos_ += up_dir_ * move_per_frame_;
}

void Camera::down() {
    pos_ -= up_dir_ * move_per_frame_;
}

void Camera::left() {
    pos_ += glm::normalize(glm::cross(up_dir_, front_dir_)) * move_per_frame_;
}

void Camera::right() {
    pos_ -= glm::normalize(glm::cross(up_dir_, front_dir_)) * move_per_frame_;
}

void Camera::mouse_move(double xpos, double ypos) {
    if (first_capture_) {
        xprev_ = xpos;
        yprev_ = ypos;
        first_capture_ = false;
    }

    const float sensetivity = 0.05;
    const auto xoff = static_cast<float>(xprev_ - xpos) * sensetivity;
    const auto yoff = static_cast<float>(yprev_ - ypos) * sensetivity;
    xprev_ = xpos;
    yprev_ = ypos;

    pitch_ += yoff;
    if (pitch_ > 89.0f) {
        pitch_ = 89.0f;
    }
    if (pitch_ < -89.0f) {
        pitch_ = -89.0f;
    }
    yaw_ += fmod(xoff, 360.0f);

    front_dir_ = calc_front_dir(yaw_, pitch_);
}

void Camera::update(float fps) {
    move_per_frame_ = des_move_speed_ / fps;

    const glm::vec3 right = glm::normalize(glm::cross(fake_up_, front_dir_));
    up_dir_ = glm::normalize(glm::cross(front_dir_, right));
    view_ = glm::lookAt(pos_, pos_ + front_dir_, up_dir_);
}

const glm::mat4 &Camera::view() const {
    return view_;
}
