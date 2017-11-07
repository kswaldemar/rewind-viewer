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

    const glm::mat4 &proj_view() const;

    void update();

    glm::vec2 screen2world(const glm::vec2 &coord) const;

    ///1 if directed up, -1 if directed down
    int y_axes_invert() const;

private:
    void update_matrix();

    struct settings_t {
        float viewport_size;
        bool origin_on_top_left = true;
    };

    glm::mat4 pr_view_;
    glm::vec2 pos_;

    settings_t opt_;
};




