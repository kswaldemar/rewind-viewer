//
// Created by valdemar on 01.03.2020.
//
#pragma once

#include "Frame.h"

class FrameEditor : public Frame {
 public:
    void add_box_popup(glm::vec2 center, glm::vec2 size, std::string message);

    void add_round_popup(glm::vec2 center, float radius, std::string message);

    void add_user_text(const std::string &msg);

    void set_layer_id(size_t id);

    void clear();

    RenderContext &context();

 private:
    size_t layer_id_ = DEFAULT_LAYER;
};
