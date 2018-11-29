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
