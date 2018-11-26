#include <net/ProtoHandler.h>
#include <viewer/Frame.h>

#include <json.hpp>

#include <cstdint>

class JsonHandler
    : public ProtoHandler
{
public:
    using ProtoHandler::ProtoHandler;

    void handle_message(const uint8_t *data, uint32_t nbytes) override;

    void on_new_connection() override;

private:
    void process_json_message(const uint8_t *chunk_begin, const uint8_t *chunk_end);

    std::unique_ptr<Frame> frame_;
    std::string fragment_msg_;
};


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