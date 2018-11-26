//
// Created by valdemar on 22.10.17.
//

#pragma once

#include <cgutils/utils.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace pod {
struct Circle;
struct Rectangle;
struct Line;
struct Popup;
}

/**
 * Represent one game frame.
 *  - contains all primitives to be drawn
 *  - contains user message to draw in message window
 */
struct Frame {
    constexpr static size_t LAYERS_COUNT = 5;
    constexpr static size_t DEFAULT_LAYER = 2;

    struct primitives_t {
        std::vector<pod::Circle> circles;
        std::vector<pod::Rectangle> rectangles;
        std::vector<pod::Line> lines;
    };

    std::array<primitives_t, LAYERS_COUNT> primitives;
    std::vector<pod::Popup> popups;
    std::string user_message;
};


namespace pod {

struct Color {
    glm::vec4 color;
};

struct Line : Color {
    float x1;
    float y1;
    //TODO: Rewrite it without color duplication
    glm::vec4 color2;
    float x2;
    float y2;
};

struct Circle : Color {
    glm::vec2 center;
    float radius;
};

struct Rectangle : Color {
    glm::vec2 center;
    float w;
    float h;
};

struct Popup {
    glm::vec2 center;
    float radius;
    std::string text;
};

} // namespace pod