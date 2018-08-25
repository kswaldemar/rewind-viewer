//
// Created by valdemar on 22.10.17.
//

#pragma once

#include <cgutils/utils.h>

#include <glm/glm.hpp>
#include <json.hpp>

#include <cstdint>
#include <unordered_map>

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

/*
 * Json deserialisation
 */
///Helper function
template<typename T>
inline T value_or_default(const nlohmann::json &j, const std::string &name, T def_val) {
    const auto it = j.find(name);
    if (it != j.end()) {
        return it->get<T>();
    }
    return def_val;
}


inline void from_json(const nlohmann::json &j, Color &p) {
    auto color = j["color"].get<uint32_t>();
    p.color.r = ((color & 0xFF0000) >> 16) / 256.0f;
    p.color.g = ((color & 0x00FF00) >> 8) / 256.0f;
    p.color.b = ((color & 0x0000FF)) / 256.0f;

    int alpha = ((color & 0xFF000000) >> 24);
    if (alpha > 0) {
        p.color.a = alpha / 256.0f;
    } else {
        p.color.a = 1.0f;
    }
}

inline void from_json(const nlohmann::json &j, Line &p) {
    from_json(j, static_cast<Color &>(p));
    p.color2 = p.color;

    p.x1 = j["x1"].get<float>();
    p.y1 = j["y1"].get<float>();
    p.x2 = j["x2"].get<float>();
    p.y2 = j["y2"].get<float>();
}

inline void from_json(const nlohmann::json &j, Circle &p) {
    from_json(j, static_cast<Color &>(p));

    p.radius = j["r"].get<float>();
    p.center.x = j["x"].get<float>();
    p.center.y = j["y"].get<float>();
}

inline void from_json(const nlohmann::json &j, Popup &p) {
    p.radius = j["r"].get<float>();
    p.center.x = j["x"].get<float>();
    p.center.y = j["y"].get<float>();
    p.text = j["text"].get<std::string>();
}

inline void from_json(const nlohmann::json &j, Rectangle &p) {
    from_json(j, static_cast<Color &>(p));
    float x1 = j["x1"].get<float>();
    float y1 = j["y1"].get<float>();
    float x2 = j["x2"].get<float>();
    float y2 = j["y2"].get<float>();

    p.w = x2 - x1;
    p.h = y2 - y1;
    p.center.x = x1 + p.w * 0.5f;
    p.center.y = y1 + p.h * 0.5f;
}

} // namespace pod