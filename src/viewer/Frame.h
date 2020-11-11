//
// Created by valdemar on 22.02.2020.
//

#pragma once

#include <array>
#include <cstdlib>

#include <viewer/Popup.h>
#include <viewer/RenderContext.h>

class Frame {
 public:
    constexpr static size_t LAYERS_COUNT = 10;
    constexpr static size_t DEFAULT_LAYER = 2;

    using context_collection_t = std::array<RenderContext, LAYERS_COUNT>;
    using popup_collection_t = std::array<std::vector<Popup>, LAYERS_COUNT>;

    void update_from(const context_collection_t &from_contexts);
    void update_from(const Frame &other);

    const context_collection_t &all_contexts() const;
    const popup_collection_t &all_popups() const;

    const char *user_message() const;

 protected:
    context_collection_t contexts_;
    popup_collection_t popups_;
    std::string user_message_;
};
