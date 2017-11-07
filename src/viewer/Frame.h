//
// Created by valdemar on 22.10.17.
//

#pragma once

#include <cstdint>

#include <glm/glm.hpp>
#include <json.hpp>

namespace pod {
struct Circle;
struct Rectangle;
struct Line;
struct Unit;
}

/**
 * Represent one game frame.
 *  - contains all primitives to be drawn
 *  - contains user message to draw in message window
 */
struct Frame {
    enum class UnitType {
        undefined = 0,
        helicopter = 1,
        tank = 2,
        fighter = 3,
    };

    enum class AreaType {
        unknown = 0,
        forest,
        swamp,
        rain,
        cloud,
    };

    std::vector<pod::Circle> circles;
    std::vector<pod::Rectangle> rectangles;
    std::vector<pod::Line> lines;
    std::vector<pod::Unit> units;
    std::string user_message;
};


namespace pod {

struct Colored {
    glm::vec3 color;
};

struct Line : Colored {
    float x1;
    float y1;
    glm::vec3 surprise; //TODO: Need to use geometric shader, to avoid that
    float x2;
    float y2;
};

struct Circle : Colored {
    glm::vec2 center;
    float radius;
};

struct Rectangle : Colored {
    glm::vec2 center;
    float w;
    float h;
};

struct Unit : Circle {
    int hp;
    int max_hp;
    Frame::UnitType utype = Frame::UnitType::undefined;
    float course = 0;
};

struct AreaDesc {
    //Cell coordinates
    int x;
    int y;
    Frame::AreaType type;
};


/*
 * Json deserialisation
 */
inline void from_json(const nlohmann::json &j, Colored &p) {
    auto color = j["color"].get<uint32_t>();
    p.color.r = ((color & 0xFF0000) >> 16) / 256.0f;
    p.color.g = ((color & 0x00FF00) >> 8) / 256.0f;
    p.color.b = ((color & 0x0000FF)) / 256.0f;
}

inline void from_json(const nlohmann::json &j, Line &p) {
    from_json(j, static_cast<Colored &>(p));
    p.surprise = p.color;

    p.x1 = j["x1"].get<float>();
    p.y1 = j["y1"].get<float>();
    p.x2 = j["x2"].get<float>();
    p.y2 = j["y2"].get<float>();
}

inline void from_json(const nlohmann::json &j, Circle &p) {
    from_json(j, static_cast<Colored &>(p));

    p.radius = j["r"].get<float>();
    p.center.x = j["x"].get<float>();
    p.center.y = j["y"].get<float>();
}

inline void from_json(const nlohmann::json &j, Rectangle &p) {
    from_json(j, static_cast<Colored &>(p));
    float x1 = j["x1"].get<float>();
    float y1 = j["y1"].get<float>();
    float x2 = j["x2"].get<float>();
    float y2 = j["y2"].get<float>();

    p.w = x2 - x1;
    p.h = y1 - y2;
    p.center.x = x1 + p.w * 0.5f;
    p.center.y = y2 + p.h * 0.5f;
}

inline void from_json(const nlohmann::json &j, Unit &p) {
    p.radius = j["r"].get<float>();
    p.center.x = j["x"].get<float>();
    p.center.y = j["y"].get<float>();

    static const glm::vec3 our_color{0.0f, 0.0f, 1.0f};
    static const glm::vec3 enemy_color{1.0f, 0.0f, 0.0f};
    static const glm::vec3 neutral_color{0.5f, 0.5f, 0.0f};
    int is_enemy = j["enemy"].get<int>();
    if (is_enemy == 1) {
        p.color = enemy_color;
    } else if (is_enemy == -1) {
        p.color = our_color;
    } else {
        p.color = neutral_color;
    }
    p.hp = j["hp"].get<int>();
    p.max_hp = j["max_hp"].get<int>();

    //Optional values
    const auto it = j.find("unit_type");
    if (it != j.end()) {
        p.utype = static_cast<Frame::UnitType>(it->get<int>());
    }

    //Course, needed to rotate texture, angle in radians [0, 2 * pi) counter clockwise
    const auto it2 = j.find("course");
    if (it2 != j.end()) {
        p.course = it2->get<float>();
    }
}

inline void from_json(const nlohmann::json &j, AreaDesc &p) {
    p.x = j["x"].get<int>();
    p.y = j["y"].get<int>();
    p.type = static_cast<Frame::AreaType>(j["area_type"].get<int>());
}


} // namespace pod