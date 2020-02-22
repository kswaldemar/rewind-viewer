#include <net/ProtoHandler.h>
#include <viewer/Frame.h>

#include <json.hpp>

#include <cstdint>

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

} // namespace pod

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
