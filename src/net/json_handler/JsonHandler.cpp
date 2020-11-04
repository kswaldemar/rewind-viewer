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

namespace pod {

struct Color {
    glm::vec4 color;
};

struct Line : Color {
    glm::vec2 pt_from;
    glm::vec2 pt_to;
};

struct Circle : Color {
    glm::vec2 center;
    float radius;
    bool fill;
};

struct Rectangle : Color {
    glm::vec2 top_left;
    glm::vec2 bottom_right;
    bool fill;
};

struct Popup {
    glm::vec2 center;
    float radius;
    std::string text;
};

/*
 * Json deserialization
 */

inline void from_json(const nlohmann::json &j, Color &p) {
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
}

[[maybe_unused]] inline void from_json(const nlohmann::json &j, Line &p) {
    from_json(j, static_cast<Color &>(p));

    p.pt_from.x = j["x1"].get<float>();
    p.pt_from.y = j["y1"].get<float>();
    p.pt_to.x = j["x2"].get<float>();
    p.pt_to.y = j["y2"].get<float>();
}

[[maybe_unused]] inline void from_json(const nlohmann::json &j, Circle &p) {
    from_json(j, static_cast<Color &>(p));

    p.radius = j["r"].get<float>();
    p.center.x = j["x"].get<float>();
    p.center.y = j["y"].get<float>();
    p.fill = value_or_default(j, "fill", false);
}

[[maybe_unused]] inline void from_json(const nlohmann::json &j, Popup &p) {
    p.radius = j["r"].get<float>();
    p.center.x = j["x"].get<float>();
    p.center.y = j["y"].get<float>();
    p.text = j["text"].get<std::string>();
}

[[maybe_unused]] inline void from_json(const nlohmann::json &j, Rectangle &p) {
    from_json(j, static_cast<Color &>(p));

    p.top_left.x = j["x1"].get<float>();
    p.top_left.y = j["y1"].get<float>();
    p.bottom_right.x = j["x2"].get<float>();
    p.bottom_right.y = j["y2"].get<float>();
    p.fill = value_or_default(j, "fill", true);

    if (p.top_left.x > p.bottom_right.x) {
        std::swap(p.top_left, p.bottom_right);
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
            case PrimitiveType::LINE: {
                LOG_V8("JsonHandler::Line detected");
                auto obj = j.get<pod::Line>();
                ctx.add_polyline({obj.pt_from, obj.pt_to}, obj.color);
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
                if (it != j.end()) {
                    use_permanent_frame(it->get<bool>());
                }

                it = j.find("layer");
                if (it != j.end()) {
                    auto layer = it->get<size_t>();
                    if (layer < 1 || layer > static_cast<size_t>(Frame::LAYERS_COUNT)) {
                        LOG_WARN("Got message with layer %zu, but should be in range 1-%zu", layer,
                                 static_cast<size_t>(Frame::LAYERS_COUNT));
                    }
                    layer = cg::clamp<size_t>(layer - 1, 0, Frame::LAYERS_COUNT - 1);
                    get_frame_editor().set_layer_id(layer);
                }
                break;
            }
            case PrimitiveType::TYPES_COUNT: break;
        }
    } catch (const std::exception &e) {
        LOG_WARN("JsonClient::Exception: %s", e.what());
    }
}
