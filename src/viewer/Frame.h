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
        UNKNOWN = 0,
        TANK = 1,
        IFV = 2,
        ARRV = 3,
        HELICOPTER = 4,
        FIGHTER = 5,
    };

    enum class AreaType {
        UNKNOWN = 0,
        FOREST,
        SWAMP,
        RAIN,
        CLOUD,
    };

    static const char *unit_name(UnitType type) {
        static const std::map<UnitType, const char *> type2name{
            {UnitType::UNKNOWN,    "Unknown"},
            {UnitType::TANK,       "Tank"},
            {UnitType::IFV,        "Ifv"},
            {UnitType::ARRV,       "Arrv"},
            {UnitType::HELICOPTER, "Helicopter"},
            {UnitType::FIGHTER,    "Fighter"},
        };
        return type2name.at(type);
    }

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

struct Unit {
    glm::vec2 center;
    float radius;
    int hp;
    int max_hp;
    int rem_cooldown; //remaining cooldown ticks
    int cooldown; //maximum cooldown
    Frame::UnitType utype;
    float course;
    bool selected;
    int enemy; //-1 for ally, 0 for neutral, 1 for enemy
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
///Helper function
template<typename T>
inline T value_or_default(const nlohmann::json &j, const std::string &name, T def_val) {
    const auto it = j.find(name);
    if (it != j.end()) {
        return it->get<T>();
    }
    return def_val;
}


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

    p.enemy = j["enemy"].get<int>();
    p.hp = j["hp"].get<int>();
    p.max_hp = j["max_hp"].get<int>();

    //Optional values
    p.utype = static_cast<Frame::UnitType>(
        value_or_default(j, "unit_type", static_cast<int>(Frame::UnitType::UNKNOWN))
    );
    p.course = value_or_default(j, "course", 0.0f);
    p.selected = static_cast<bool>(value_or_default(j, "selected", 0));
    auto it = j.find("rem_cooldown");
    if (it != j.end()) {
        p.rem_cooldown = it->get<int>();
        p.cooldown = j["cooldown"].get<int>();
    } else {
        p.rem_cooldown = 0;
        p.cooldown = 0;
    }
}

inline void from_json(const nlohmann::json &j, AreaDesc &p) {
    p.x = j["x"].get<int>();
    p.y = j["y"].get<int>();
    p.type = static_cast<Frame::AreaType>(j["area_type"].get<int>());
}


} // namespace pod