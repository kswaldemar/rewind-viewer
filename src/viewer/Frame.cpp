#include "Frame.h"

#include <cassert>
#include <memory>

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


void Frame::add_box_popup(glm::vec2 top_left, glm::vec2 bottom_right, std::string message) {
    popups_.emplace_back(std::make_unique<rect_popup_t>(top_left, bottom_right, message));
}

void Frame::add_round_popup(glm::vec2 center, float radius, std::string message) {
    popups_.emplace_back(std::make_unique<round_popup_t>(center, radius, message));
}

void Frame::add_user_text(const std::string &msg) {
    user_message_ += msg;
}

void Frame::set_layer_id(size_t id) {
    assert(id < contexts_.size());
    layer_id_ = id;
}

RenderContext &Frame::context() {
    return contexts_[layer_id_];
}

const Frame::context_collection_t &Frame::all_contexts() const {
    return contexts_;
}

const std::vector<Frame::hittest_t> &Frame::popups() const {
    return popups_;
}

const char *Frame::user_message() const {
    return user_message_.c_str();
}
