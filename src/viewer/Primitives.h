//
// Created by valdemar on 22.10.17.
//
///Drawable primitives for Russian AI Cup 2017
#pragma once

#include <cstdint>

#include <glm/vec3.hpp>
#include <json.hpp>

namespace pod {

struct Point {
    float x;
    float y;
};

struct Fillable {
    bool fill;
};

struct Colored {
    glm::vec3 color;
};

inline void from_json(const nlohmann::json &j, Colored &p) {
    auto color = j["color"].get<uint32_t>();
    p.color.r = ((color & 0xFF0000) >> 16) / 256.0f;
    p.color.g = ((color & 0x00FF00) >> 8) / 256.0f;
    p.color.b = ((color & 0x0000FF)) / 256.0f;
}

struct OrientPoint : Point {
    float course;
};

struct Unit : OrientPoint {
    uint16_t hp;
    uint32_t id;
};

struct Line : Colored {
    Point p1;
    Point p2;
};

struct Circle : Colored {
    Point center;
    float radius;
};

inline void from_json(const nlohmann::json &j, Circle &p) {
    auto color = j["color"].get<uint32_t>();
    p.color.r = ((color & 0xFF0000) >> 16) / 256.0f;
    p.color.g = ((color & 0x00FF00) >> 8) / 256.0f;
    p.color.b = ((color & 0x0000FF)) / 256.0f;

    p.radius = j["r"].get<float>();
    p.center.x = j["x"].get<float>();
    p.center.y = j["y"].get<float>();
}

struct Rectangle : Colored {
    Point top_left;
    Point bot_right;
};

} // namespace pod