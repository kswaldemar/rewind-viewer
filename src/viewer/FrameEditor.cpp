//
// Created by valdemar on 01.03.2020.
//
#include "FrameEditor.h"

namespace {

struct rect_popup_t : IPopup {
    rect_popup_t(glm::vec2 top_left, glm::vec2 bottom_right, const std::string &msg)
        : top_left(top_left),
          bottom_right(bottom_right),
          msg(msg)
    { }

    bool hit_test(glm::vec2 point) const override {
        return point.x >= top_left.x && point.x <= bottom_right.x
               && point.y >= top_left.y && point.y <= bottom_right.y;
    }

    const char *text() const override {
        return msg.c_str();
    }

    glm::vec2 top_left;
    glm::vec2 bottom_right;
    std::string msg;
};

struct round_popup_t : IPopup {
    round_popup_t(glm::vec2 center, float radius2, const std::string &msg)
        : center(center),
          radius2(radius2),
          msg(msg)
    { }

    bool hit_test(glm::vec2 point) const override {
        auto diff = point - center;
        float r2 = glm::dot(diff, diff);
        return r2 <= radius2;
    }

    const char *text() const override {
        return msg.c_str();
    }

    glm::vec2 center;
    float radius2;
    std::string msg;
};

} // anonymous namespace


void FrameEditor::add_box_popup(glm::vec2 top_left, glm::vec2 bottom_right, std::string message) {
    popups_.emplace_back(std::make_unique<rect_popup_t>(top_left, bottom_right, message));
}

void FrameEditor::add_round_popup(glm::vec2 center, float radius, std::string message) {
    popups_.emplace_back(std::make_unique<round_popup_t>(center, radius, message));
}

void FrameEditor::add_user_text(const std::string &msg) {
    user_message_ += msg;
}

void FrameEditor::set_layer_id(size_t id) {
    assert(id < contexts_.size());
    layer_id_ = id;
}

void FrameEditor::clear() {
    for (auto &ctx : contexts_) {
        ctx.clear();
    }
}

RenderContext &FrameEditor::context() {
    return contexts_[layer_id_];
}

