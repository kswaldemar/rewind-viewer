#include "JsonHandler.h"

#include <common/logger.h>
#include <net/PrimitiveType.h>
#include <viewer/FrameEditor.h>

#include <rapidjson/document.h>
#include <rapidjson/reader.h>
#include <rapidjson/stream.h>
#include <rapidjson/error/en.h>

#include <cassert>

namespace {

/// Helper functions
bool value_or_default(const rapidjson::Value &j, const char* name, bool def_val) {
    if (auto it = j.FindMember(name); it != j.MemberEnd()) {
        return it->value.GetBool();
    }
    return def_val;
}

bool contains(const rapidjson::Value &j, const char* key) {
    return j.HasMember(key);
}

std::vector<float> convert_to_float_array(const rapidjson::Value& v) {
    std::vector<float> result;
    result.reserve(v.Size());
    for (const auto& elem : v.GetArray()) {
        result.push_back(elem.GetFloat());
    }
    return result;
}

std::vector<uint32_t> convert_to_uint_array(const rapidjson::Value& v) {
    std::vector<uint32_t> result;
    result.reserve(v.Size());
    for (const auto& elem : v.GetArray()) {
        result.push_back(elem.GetUint());
    }
    return result;
}

void normalize(glm::vec2 &min_corner, glm::vec2 &max_corner) {
    if (min_corner.x > max_corner.x) {
        std::swap(min_corner.x, max_corner.x);
    }
    if (min_corner.y > max_corner.y) {
        std::swap(min_corner.y, max_corner.y);
    }
}

glm::vec4 convert_color(uint32_t value) {
    glm::vec4 result;
    result.r = static_cast<float>((value & 0xFF0000) >> 16) / 255.0f;
    result.g = static_cast<float>((value & 0x00FF00) >> 8) / 255.0f;
    result.b = static_cast<float>((value & 0x0000FF)) / 255.0f;

    uint8_t alpha = ((value & 0xFF000000) >> 24);
    if (alpha > 0) {
        result.a = static_cast<float>(alpha) / 255.0f;
    } else {
        result.a = 1.0f;
    }
    return result;
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

struct Rectangle {
    RenderContext::RectangleColors colors;
    bool fill;
    glm::vec2 top_left;
    glm::vec2 bottom_right;
};

struct Popup {
    bool is_round;
    std::string text;
    glm::vec2 center;
    union {
        struct {
            float radius;
        };
        struct {
            float w;
            float h;
        };
    };
};

struct Polyline : ColorShape {
    std::vector<glm::vec2> points;
};

struct Triangle {
    RenderContext::TriangleColors colors;
    bool fill;
    std::vector<glm::vec2> points;
};

std::vector<glm::vec2> convert_check(const GeoPoints &points) {
    if (points.size() % 2 != 0) {
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

inline void from_json(const rapidjson::Value &j, ColorShape &p) {
    p.color = convert_color(j["color"].GetUint());
    p.fill = value_or_default(j, "fill", true);
}

[[maybe_unused]] inline void from_json(const rapidjson::Value &j, Circle &p) {
    from_json(j, static_cast<ColorShape &>(p));
    p.radius = j["r"].GetFloat();
    p.center = convert_position(convert_to_float_array(j["p"]));
}

[[maybe_unused]] inline void from_json(const rapidjson::Value &j, Popup &p) {
    if (contains(j, "tl") && contains(j, "br")) {
        p.is_round = false;
        auto min_corner = convert_position(convert_to_float_array(j["tl"]));
        auto max_corner = convert_position(convert_to_float_array(j["br"]));
        normalize(min_corner, max_corner);

        const auto diff = max_corner - min_corner;
        p.center = min_corner + diff * 0.5f;
        p.w = diff.x;
        p.h = diff.y;
    } else if (contains(j, "r") && contains(j, "p")) {
        p.is_round = true;
        p.radius = j["r"].GetFloat();
        p.center = convert_position(convert_to_float_array(j["p"]));
    } else {
        throw ParsingError{"Popup should contain either fields [p, r] or [tl, br]"};
    }

    p.text = std::string(j["text"].GetString());
}

[[maybe_unused]] inline void from_json(const rapidjson::Value &j, Rectangle &p) {
    if (j["color"].IsArray()) {
        const auto colors = convert_to_uint_array(j["color"]);
        if (colors.size() != 4) {
            throw ParsingError{"Rectangle expect exactly 4 colors for gradient setup, got " +
                               std::to_string(colors.size())};
        }
        for (size_t i = 0; i < colors.size(); ++i) {
            p.colors[i] = convert_color(colors[i]);
        }
    } else {
        auto color = convert_color(j["color"].GetUint());
        p.colors.fill(color);
    }
    p.fill = value_or_default(j, "fill", true);

    p.top_left = convert_position(convert_to_float_array(j["tl"]));
    p.bottom_right = convert_position(convert_to_float_array(j["br"]));
    normalize(p.top_left, p.bottom_right);
}

[[maybe_unused]] inline void from_json(const rapidjson::Value &j, Polyline &p) {
    from_json(j, static_cast<ColorShape &>(p));

    p.points = convert_check(convert_to_float_array(j["points"]));
}

[[maybe_unused]] inline void from_json(const rapidjson::Value &j, Triangle &p) {
    if (j["color"].IsArray()) {
        const auto colors = convert_to_uint_array(j["color"]);
        if (colors.size() != 3) {
            throw ParsingError{"Triangle expect exactly 3 colors for gradient setup, got " +
                               std::to_string(colors.size())};
        }
        for (size_t i = 0; i < colors.size(); ++i) {
            p.colors[i] = convert_color(colors[i]);
        }
    } else {
        auto color = convert_color(j["color"].GetUint());
        p.colors.fill(color);
    }
    p.fill = value_or_default(j, "fill", true);

    p.points = convert_check(convert_to_float_array(j["points"]));
    if (p.points.size() != 3) {
        throw ParsingError{"Triangle should be created using exactly 3 points, got " +
                           std::to_string(p.points.size())};
    }
}

}  // namespace pod

void JsonHandler::handle_message(const uint8_t *data, uint32_t nbytes) {
    constexpr size_t kBufferMaxSize = 4096;
    if (fragment_msg_.size() + nbytes > kBufferMaxSize) {
        fragment_msg_.erase(0, buffer_start_);
        buffer_start_ = 0;
    }

    // copy to internal buffer, performance cost
    fragment_msg_.append(reinterpret_cast<const char*>(data), nbytes);

    size_t processed = 0;
    size_t available_data = fragment_msg_.size() - buffer_start_;
    while (processed < available_data) {
        rapidjson::Document doc;
        rapidjson::StringStream ss(fragment_msg_.c_str() + buffer_start_ + processed);
        doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(ss);

        if (doc.HasParseError()) {
            // Not enough data for complete JSON or invalid fragment; wait for more data
            break;
        }

        process_json_message(doc);
        processed += static_cast<size_t>(ss.Tell());
    }

    buffer_start_ += processed;
}

///////////////////////////////////////////////////////////////////////////////
void JsonHandler::process_json_message(const rapidjson::Document& j) {
    try {
        PrimitiveType type = primitve_type_from_str(std::string(j["type"].GetString()));

        auto &ctx = get_frame_editor().context();

        switch (type) {
            case PrimitiveType::END: {
                LOG_V8("JsonHandler::End");
                break;
            }
            case PrimitiveType::CIRCLE: {
                LOG_V8("JsonHandler::Circle detected");
                pod::Circle obj;
                from_json(j, obj);
                ctx.add_circle(obj.center, obj.radius, obj.color, obj.fill);
                break;
            }
            case PrimitiveType::RECTANGLE: {
                LOG_V8("JsonHandler::Rectangle detected");
                pod::Rectangle obj;
                from_json(j, obj);
                ctx.add_rectangle(obj.top_left, obj.bottom_right, obj.colors, obj.fill);
                break;
            }
            case PrimitiveType::TRIANGLE: {
                LOG_V8("JsonHandler::Triangle detected");
                pod::Triangle obj;
                from_json(j, obj);
                ctx.add_triangle(obj.points[0], obj.points[1], obj.points[2], obj.colors, obj.fill);
                break;
            }
            case PrimitiveType::POLYLINE: {
                LOG_V8("JsonHandler::Polyline detected");
                pod::Polyline obj;
                from_json(j, obj);
                ctx.add_polyline(obj.points, obj.color);
                break;
            }
            case PrimitiveType::MESSAGE:
                LOG_V8("JsonHandler::Message");
                get_frame_editor().add_user_text(std::string(j["message"].GetString()));
                break;
            case PrimitiveType::POPUP: {
                LOG_V8("JsonHandler::Popup");
                pod::Popup obj;
                from_json(j, obj);
                if (obj.is_round) {
                    get_frame_editor().add_round_popup(obj.center, obj.radius, std::move(obj.text));
                } else {
                    get_frame_editor().add_box_popup(obj.center, {obj.w, obj.h},
                                                     std::move(obj.text));
                }
                break;
            }
            case PrimitiveType::OPTIONS: {
                LOG_V8("JsonHandler::Layer");
                bool found_option = false;
                if (j.HasMember("permanent")) {
                    use_permanent_frame(j["permanent"].GetBool());
                    found_option = true;
                }

                if (j.HasMember("layer")) {
                    set_layer(j["layer"].GetUint64());
                    found_option = true;
                }

                if (!found_option) {
                    LOG_ERROR("useless 'options' without any option");
                }
                break;
            }
            case PrimitiveType::TYPES_COUNT: break;
        }

        on_message_processed(type == PrimitiveType::END);
    } catch (const std::exception &e) {
        LOG_WARN("JsonClient::Exception: %s", e.what());
    }
}
