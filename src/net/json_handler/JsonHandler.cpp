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

}

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
        //Support for fragmented messages
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
        auto layer_it = j.find("layer");
        size_t layer = Frame::DEFAULT_LAYER;
        if (layer_it != j.end()) {
            layer = layer_it->get<size_t>();
            if (layer < 1 || layer > static_cast<size_t>(Frame::LAYERS_COUNT)) {
                LOG_WARN("Got message with layer %zu, but should be in range 1-%zu",
                         layer, static_cast<size_t>(Frame::LAYERS_COUNT));
            }
            layer = cg::clamp<size_t>(layer - 1, 0, Frame::LAYERS_COUNT - 1);
        }

        if (!frame_) {
            frame_ = std::make_unique<Frame>();
        }

        auto &slice = frame_->primitives[layer];

        switch (type) {
            case PrimitiveType::begin:
                LOG_V8("JsonHandler::Begin");
                break;
            case PrimitiveType::end:
                LOG_V8("JsonHandler::End");
                send_to_scene(std::move(frame_));
                frame_ = nullptr;
                break;
            case PrimitiveType::circle:
                LOG_V8("JsonHandler::Circle detected");
                slice.circles.emplace_back(j);
                break;
            case PrimitiveType::rectangle:
                LOG_V8("JsonHandler::Rectangle detected");
                slice.rectangles.emplace_back(j);
                break;
            case PrimitiveType::line:
                LOG_V8("JsonHandler::Line detected");
                slice.lines.emplace_back(j);
                break;
            case PrimitiveType::message:
                LOG_V8("JsonHandler::Message");
                frame_->user_message += j["message"].get<std::string>();
                break;
            case PrimitiveType::popup:
                LOG_V8("JsonHandler::Popup");
                frame_->popups.emplace_back(j);
                break;
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
