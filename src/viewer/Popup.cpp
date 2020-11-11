//
// Created by Vladimir A. Kiselev on 06.11.2020.
//
#include "Popup.h"

#include <glm/glm.hpp>

Popup Popup::create_circle(glm::vec2 center, float radius, std::string text) {
    Popup result;
    result.is_circle_ = true;
    result.center_ = center;
    result.half_width_ = radius;
    result.half_height_ = radius;
    result.text_ = std::move(text);
    return result;
}

Popup Popup::create_rect(glm::vec2 center, glm::vec2 size, std::string text) {
    Popup result;
    result.is_circle_ = false;
    result.center_ = center;
    result.half_width_ = size.x * 0.5f;
    result.half_height_ = size.y * 0.5f;
    result.text_ = std::move(text);
    return result;
}

bool Popup::hit_test(glm::vec2 point) const {
    auto diff = glm::abs(point - center_);
    if (is_circle_) {
        float r2 = glm::dot(diff, diff);
        return r2 <= half_width_ * half_width_;
    }
    return diff.x <= half_width_ && diff.y <= half_height_;
}

const char* Popup::text() const {
    return text_.c_str();
}
