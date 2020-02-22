//
// Created by valdemar on 22.02.2020.
//

#pragma once

#include <cstdlib>
#include <array>

#include <viewer/RenderContext.h>

struct IPopup {
    virtual bool hit_test(glm::vec2 pt) const = 0;
    virtual const char *text() const = 0;
};

class Frame {
public:
    constexpr static size_t LAYERS_COUNT = 5;
    constexpr static size_t DEFAULT_LAYER = 2;

    using context_collection_t = std::array<RenderContext, LAYERS_COUNT>;
    using hittest_t = std::unique_ptr<IPopup>;

    void add_box_popup(glm::vec2 top_left, glm::vec2 bottom_right, std::string message);

    void add_round_popup(glm::vec2 center, float radius, std::string message);

    void add_user_text(const std::string &msg);

    void set_layer_id(size_t id);

    RenderContext &context();

    const context_collection_t &all_contexts() const;

    const std::vector<hittest_t> &popups() const;

    const char *user_message() const;

private:
    size_t layer_id_ = DEFAULT_LAYER;
    context_collection_t contexts_;
    std::vector<hittest_t> popups_;
    std::string user_message_;
};