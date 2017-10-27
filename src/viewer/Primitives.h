//
// Created by valdemar on 22.10.17.
//
///Drawable primitives for Russian AI Cup 2017
#pragma once

#include <cstdint>

#include <glm/vec3.hpp>
#include <json.hpp>

namespace pod {

struct Colored {
    glm::vec3 color;
};

inline void from_json(const nlohmann::json &j, Colored &p) {
    auto color = j["color"].get<uint32_t>();
    p.color.r = ((color & 0xFF0000) >> 16) / 256.0f;
    p.color.g = ((color & 0x00FF00) >> 8) / 256.0f;
    p.color.b = ((color & 0x0000FF)) / 256.0f;
}

struct Line : Colored {
    float x1;
    float y1;
    float x2;
    float y2;
};

inline void from_json(const nlohmann::json &j, Line &p) {
    from_json(j, static_cast<Colored&>(p));

    p.x1 = j["x1"].get<float>();
    p.y1 = j["y1"].get<float>();
    p.x2 = j["x2"].get<float>();
    p.y2 = j["y2"].get<float>();
}

struct Circle : Colored {
    glm::vec2 center;
    float radius;
};

inline void from_json(const nlohmann::json &j, Circle &p) {
    from_json(j, static_cast<Colored&>(p));

    p.radius = j["r"].get<float>();
    p.center.x = j["x"].get<float>();
    p.center.y = j["y"].get<float>();
}

struct Rectangle : Colored {
    glm::vec2 center;
    float w;
    float h;
};

inline void from_json(const nlohmann::json &j, Rectangle &p) {
    from_json(j, static_cast<Colored&>(p));
    float x1 = j["x1"].get<float>();
    float y1 = j["y1"].get<float>();
    float x2 = j["x2"].get<float>();
    float y2 = j["y2"].get<float>();

    p.w = x2 - x1;
    p.h = y1 - y2;
    p.center.x = x1 + p.w * 0.5f;
    p.center.y = y2 + p.h * 0.5f;
}

} // namespace pod