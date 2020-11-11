//
// Created by Vladimir A. Kiselev on 06.11.2020.
//

#pragma once

#include <string>

#include <glm/vec2.hpp>

class Popup {
 public:
    static Popup create_circle(glm::vec2 center, float radius, std::string text);
    static Popup create_rect(glm::vec2 center, glm::vec2 size, std::string text);

    bool hit_test(glm::vec2 point) const;

    const char *text() const;

 private:
    Popup() = default;

    bool is_circle_{};
    glm::vec2 center_{};
    float half_width_{};
    float half_height_{};
    std::string text_{};
};
