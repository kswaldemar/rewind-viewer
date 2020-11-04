#include "JsonHandler.h"

#include <common/logger.h>
#include <net/PrimitiveType.h>
#include <viewer/FrameEditor.h>

#include <json.hpp>

#include <cassert>

namespace {

/// Helper function
template <typename T>
inline T value_or_default(const nlohmann::json &j, const std::string &name, T def_val) {
    const auto it = j.find(name);
    if (it != j.end()) {
        return it->get<T>();
    }
    return def_val;
}

}  // anonymous namespace

struct ParsingError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

namespace pod {

/// GeoPoints format [x1, y1, x2, y2, ...]
using GeoPoints = std::vector<float>;

struct ColorShape {
    glm::vec4 color;
    bool fill;
};

struct Circle : ColorShape {
    glm::vec2 center;
    float radius;
};

struct Rectangle : ColorShape {
    glm::vec2 top_left;
    glm::vec2 bottom_right;
};

struct Popup {
    glm::vec2 center;
    float radius;
    std::string text;
};

struct Polyline : ColorShape {
    std::vector<glm::vec2> points;
};

struct Triangle : ColorShape {
    std::vector<glm::vec2> points;
};

std::vector<glm::vec2> convert_check(const GeoPoints &points) {
    if (!points.size() % 2 != 0) {
        throw ParsingError{
            "Invalid geopoints format: number of elements should be divisible by 2, got" +
            std::to_string(points.size())};
    }

    std::vector<glm::vec2> result;
    result.reserve(points.size() / 2);
    for (size_t idx = 0; idx < points.size(); idx += 2) {
        result.emplace_back(points[idx], points[idx + 1]);
    }
    return result;
}

glm::vec2 convert_position(const GeoPoints &points) {
    auto pts = convert_check(points);
    if (pts.size() != 1) {
        throw ParsingError{"Too many points, expected only one, but got " +
                           std::to_string(pts.size())};
    }
    return pts[0];
}

/*
 * Json deserialization
 */

inline void from_json(const nlohmann::json &j, ColorShape &p) {
    auto color = j["color"].get<uint32_t>();
    p.color.r = static_cast<float>((color & 0xFF0000) >> 16) / 256.0f;
    p.color.g = static_cast<float>((color & 0x00FF00) >> 8) / 256.0f;
    p.color.b = static_cast<float>((color & 0x0000FF)) / 256.0f;

    uint8_t alpha = ((color & 0xFF000000) >> 24);
    if (alpha > 0) {
        p.color.a = static_cast<float>(alpha) / 256.0f;
    } else {
        p.color.a = 1.0f;
    }

    p.fill = value_or_default(j, "fill", true);
}

[[maybe_unused]] inline void from_json(const nlohmann::json &j, Circle &p) {
    from_json(j, static_cast<ColorShape &>(p));

    p.radius = j["r"].get<float>();
    p.center = convert_position(j["p"].get<GeoPoints>());
}

[[maybe_unused]] inline void from_json(const nlohmann::json &j, Popup &p) {
    p.radius = j["r"].get<float>();
    p.center = convert_position(j["p"].get<GeoPoints>());
    p.text = j["text"].get<std::string>();
}

[[maybe_unused]] inline void from_json(const nlohmann::json &j, Rectangle &p) {
    from_json(j, static_cast<ColorShape &>(p));

    p.top_left = convert_position(j["tl"].get<GeoPoints>());
    p.bottom_right = convert_position(j["br"].get<GeoPoints>());
    if (p.top_left.x > p.bottom_right.x) {
        std::swap(p.top_left, p.bottom_right);
    }
}

[[maybe_unused]] inline void from_json(const nlohmann::json &j, Polyline &p) {
    from_json(j, static_cast<ColorShape &>(p));

    p.points = convert_check(j["points"].get<GeoPoints>());
}

[[maybe_unused]] inline void from_json(const nlohmann::json &j, Triangle &p) {
    from_json(j, static_cast<ColorShape &>(p));
    p.points = convert_check(j["points"].get<GeoPoints>());
    if (p.points.size() != 3) {
        throw ParsingError{"Triangle should be created using exactly 3 points, got " +
                           std::to_string(p.points.size())};
    }
}

}  // namespace pod

void JsonHandler::handle_message(const uint8_t *data, uint32_t nbytes) {
    const uint8_t *beg = data;
    const uint8_t *block_end = data + nbytes;
    while (true) {
        const uint8_t *end = std::find(beg, block_end, '}');
        if (end == block_end) {
            if (beg != block_end) {
                fragment_msg_ = std::string(beg, end);
            }
            break;
        }
        ++end;
        // Support fragmented messages
        if (fragment_msg_.empty()) {
            process_json_message(beg, end);
        } else {
            fragment_msg_ += std::string(beg, end);
            process_json_message(
                reinterpret_cast<const uint8_t *>(fragment_msg_.data()),
                reinterpret_cast<const uint8_t *>(fragment_msg_.data() + fragment_msg_.size()));
            fragment_msg_.clear();
        }
        beg = end;
    }
}

///////////////////////////////////////////////////////////////////////////////
void JsonHandler::process_json_message(const uint8_t *chunk_begin, const uint8_t *chunk_end) {
    using namespace nlohmann;
    try {
        auto j = json::parse(chunk_begin, chunk_end);
        PrimitiveType type = primitve_type_from_str(j["type"]);

        auto &ctx = get_frame_editor().context();

        switch (type) {
            case PrimitiveType::END: {
                LOG_V8("JsonHandler::End");
                on_frame_end();
                break;
            }
            case PrimitiveType::CIRCLE: {
                LOG_V8("JsonHandler::Circle detected");
                auto obj = j.get<pod::Circle>();
                ctx.add_circle(obj.center, obj.radius, obj.color, obj.fill);
                break;
            }
            case PrimitiveType::RECTANGLE: {
                LOG_V8("JsonHandler::Rectangle detected");
                auto obj = j.get<pod::Rectangle>();
                ctx.add_rectangle(obj.top_left, obj.bottom_right, obj.color, obj.fill);
                break;
            }
            case PrimitiveType::TRIANGLE: {
                LOG_V8("JsonHandler::Triangle detected");
                auto obj = j.get<pod::Triangle>();
                if (obj.fill) {
                    ctx.add_filled_triangle(obj.points[0], obj.points[1], obj.points[2], obj.color);
                } else {
                    ctx.add_polyline(obj.points, obj.color);
                }
                break;
            }
            case PrimitiveType::POLYLINE: {
                LOG_V8("JsonHandler::Polyline detected");
                auto obj = j.get<pod::Polyline>();
                ctx.add_polyline(obj.points, obj.color);
                break;
            }
            case PrimitiveType::MESSAGE:
                LOG_V8("JsonHandler::Message");
                get_frame_editor().add_user_text(j["message"].get<std::string>());
                break;
            case PrimitiveType::POPUP: {
                LOG_V8("JsonHandler::Popup");
                auto obj = j.get<pod::Popup>();
                get_frame_editor().add_round_popup(obj.center, obj.radius, std::move(obj.text));
                break;
            }
            case PrimitiveType::OPTIONS: {
                LOG_V8("JsonHandler::Layer");
                auto it = j.find("permanent");
                bool found_option = false;
                if (it != j.end()) {
                    use_permanent_frame(it->get<bool>());
                    found_option = true;
                }

                it = j.find("layer");
                if (it != j.end()) {
                    set_layer(it->get<size_t>());
                    found_option = true;
                }

                if (!found_option) {
                    LOG_ERROR("useless 'options' without any option");
                }
                break;
            }
            case PrimitiveType::TYPES_COUNT: break;
        }
    } catch (const std::exception &e) {
        LOG_WARN("JsonClient::Exception: %s", e.what());
    }
}
