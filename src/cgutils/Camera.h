//
// Created by valdemar on 18.10.17.
//

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

class Camera {
public:
    friend class UIController;

    Camera(const glm::vec2 &initial_pos, float viewport_size);
    ~Camera() = default;

    void up();
    void down();
    void left();
    void right();

    const glm::mat4 &proj_view() const;

    void update();

private:
    struct settings_t {
        float speed_coef_per_second = 2.0f;
        float viewport_size_;
    };

    float move_per_frame_;

    glm::mat4 pr_view_;
    glm::vec2 pos_;

    settings_t opt_;
};




