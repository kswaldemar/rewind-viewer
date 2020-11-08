//
// Created by Vladimir A. Kiselev on 06.11.2020.
//
#include "Popup.h"

#include <glm/glm.hpp>

Popup Popup::create_circle(glm::vec2 center, float radius, std::string text) {
    Popup result;
    result.is_circle_ = true;
    result.center_ = center;
    result.w_ = radius;
    result.h_ = radius;
    result.text_ = std::move(text);
    return result;
}

Popup Popup::create_rect(glm::vec2 top_left, glm::vec2 bottom_right, std::string text) {
    Popup result;
    result.is_circle_ = false;
    auto diff = bottom_right - top_left;
    result.center_ = top_left + diff * 0.5f;
    result.w_ = std::abs(diff.x);
    result.h_ = std::abs(diff.y);
    result.text_ = std::move(text);
    return result;
}

bool Popup::hit_test(glm::vec2 point) const {
    auto diff = glm::abs(point - center_);
    if (is_circle_) {
        float r2 = glm::dot(diff, diff);
        return r2 <= w_ * w_;
    }
    return diff.x <= w_ && diff.y <= h_;
}

const char* Popup::text() const {
    return text_.c_str();
}
