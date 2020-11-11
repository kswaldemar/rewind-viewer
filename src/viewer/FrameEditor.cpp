//
// Created by valdemar on 01.03.2020.
//
#include "FrameEditor.h"

#include <utility>

void FrameEditor::add_box_popup(glm::vec2 center, glm::vec2 size, std::string message) {
    popups_[layer_id_].push_back(Popup::create_rect(center, size, std::move(message)));
}

void FrameEditor::add_round_popup(glm::vec2 center, float radius, std::string message) {
    popups_[layer_id_].push_back(Popup::create_circle(center, radius, std::move(message)));
}

void FrameEditor::add_user_text(const std::string &msg) {
    user_message_ += msg;
    user_message_ += '\n';
}

void FrameEditor::set_layer_id(size_t id) {
    assert(id < contexts_.size());
    layer_id_ = id;
}

void FrameEditor::clear() {
    for (size_t i = 0; i < Frame::LAYERS_COUNT; ++i) {
        contexts_[i].clear();
        popups_[i].clear();
    }
    user_message_.clear();
}

RenderContext &FrameEditor::context() {
    return contexts_[layer_id_];
}
