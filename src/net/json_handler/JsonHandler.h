#include <net/ProtoHandler.h>

#include <rapidjson/fwd.h>
#include <cstdint>

class JsonHandler : public ProtoHandler {
 public:
    using ProtoHandler::ProtoHandler;

    void handle_message(const uint8_t *data, uint32_t nbytes) override;

 private:
    void process_json_message(const rapidjson::Document& doc);

    std::string fragment_msg_;
    size_t buffer_start_ = 0;
};
