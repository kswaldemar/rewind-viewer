#include "JsonHandler.h"

#include <common/logger.h>
#include <net/PrimitiveType.h>

#include <json.hpp>

namespace pod {
/*
 * Json deserialization
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

    p.pt_from.x = j["x1"].get<float>();
    p.pt_from.y = j["y1"].get<float>();
    p.pt_to.x = j["x2"].get<float>();
    p.pt_to.y = j["y2"].get<float>();
}

inline void from_json(const nlohmann::json &j, Circle &p) {
    from_json(j, static_cast<Color &>(p));

    p.radius = j["r"].get<float>();
    p.center.x = j["x"].get<float>();
    p.center.y = j["y"].get<float>();
    p.fill = value_or_default(j, "fill", false);
}

inline void from_json(const nlohmann::json &j, Popup &p) {
    p.radius = j["r"].get<float>();
    p.center.x = j["x"].get<float>();
    p.center.y = j["y"].get<float>();
    p.text = j["text"].get<std::string>();
}

inline void from_json(const nlohmann::json &j, Rectangle &p) {
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

} // namespace pod

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
        //Support fragmented messages
        if (fragment_msg_.empty()) {
            process_json_message(beg, end);
        } else {
            fragment_msg_ += std::string(beg, end);
            process_json_message(reinterpret_cast<const uint8_t *>(fragment_msg_.data()),
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

        if (!frame_) {
            //TODO: Think about factory from Scene to implement immediate mode
            frame_ = std::make_unique<Frame>();
        }

        auto &ctx = frame_->context();

        switch (type) {
            case PrimitiveType::end:
                LOG_V8("JsonHandler::End");
                send_to_scene(std::move(frame_));
                frame_ = nullptr;
                break;
            case PrimitiveType::circle: {
                LOG_V8("JsonHandler::Circle detected");
                auto obj = j.get<pod::Circle>();
                ctx.add_circle(obj.center, obj.radius, obj.color, obj.fill);
                break;
            }
            case PrimitiveType::rectangle: {
                LOG_V8("JsonHandler::Rectangle detected");
                auto obj = j.get<pod::Rectangle>();
                ctx.add_rectangle(obj.top_left, obj.bottom_right, obj.color, obj.fill);
                break;
            }
            case PrimitiveType::line: {
                LOG_V8("JsonHandler::Line detected");
                auto obj = j.get<pod::Line>();
                ctx.add_polyline({obj.pt_from, obj.pt_to}, obj.color);
                break;
            }
            case PrimitiveType::message:
                LOG_V8("JsonHandler::Message");
                frame_->add_user_text(j["message"].get<std::string>());
                break;
            case PrimitiveType::popup: {
                LOG_V8("JsonHandler::Popup");
                auto obj = j.get<pod::Popup>();
                frame_->add_round_popup(obj.center, obj.radius, std::move(obj.text));
                break;
            }
            case PrimitiveType::layer: {
                LOG_V8("JsonHandler::Layer");
                size_t layer = j["value"].get<size_t>();
                if (layer < 1 || layer > static_cast<size_t>(Frame::LAYERS_COUNT)) {
                    LOG_WARN("Got message with layer %zu, but should be in range 1-%zu",
                             layer, static_cast<size_t>(Frame::LAYERS_COUNT));
                }
                layer = cg::clamp<size_t>(layer - 1, 0, Frame::LAYERS_COUNT - 1);
                frame_->set_layer_id(layer);
                break;
            }
            case PrimitiveType::types_count:
                LOG_WARN("Got 'types_count' message");
                break;

        }
    } catch (const std::exception &e) {
        LOG_WARN("JsonClient::Exception: %s", e.what());
    }
}

void JsonHandler::on_new_connection() {
    ProtoHandler::on_new_connection();

    frame_ = nullptr;
}
