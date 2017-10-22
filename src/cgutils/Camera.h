//
// Created by valdemar on 18.10.17.
//

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

class Camera {
public:
    Camera(const glm::vec3 &initial_pos, const glm::vec3 &up, float yaw, float pitch, float move_speed);
    ~Camera() = default;

    void forward();
    void backward();
    void left();
    void right();

    const glm::mat4 &view() const;

    void update(float fps, float mouse_wheel);

private:
    struct settings_t {
        const float min_z = 0.1f;
        const float max_z = 100.0f;
        float speed_per_second = 0.0f;
    };

    glm::vec3 pos_;
    glm::vec3 up_dir_;
    glm::vec3 front_dir_;
    float yaw_;
    float pitch_;
    float move_per_frame_;
    glm::vec3 fake_up_;

    glm::mat4 view_;

    settings_t opt_;
};




