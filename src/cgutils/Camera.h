//
// Created by valdemar on 18.10.17.
//

#pragma once

#include <glm/glm.hpp>

#include <viewer/Config.h>

class Camera {
 public:
    friend class UIController;

    explicit Camera(const Config::CameraConf &conf);
    ~Camera() = default;

    const glm::mat4 &proj_view() const;

    void update();

    glm::vec2 screen2world(const glm::vec2 &coord) const;

    /// 1 if directed up, -1 if directed down
    int y_axes_invert() const;

 private:
    void update_matrix();

    const Config::CameraConf &conf_;

    glm::mat4 pr_view_{};
    glm::vec2 pos_{};
    float viewport_size_{};
};
