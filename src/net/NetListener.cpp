#include "NetListener.h"

#include <net/PrimitiveType.h>
#include <common/logger.h>

#include <json.hpp>

NetListener::NetListener(Scene *scene, const std::string &listen_host, uint16_t listen_port)
    : scene_(scene) {
    socket_ = std::make_unique<CPassiveSocket>(CPassiveSocket::SocketTypeTcp);
    if (!socket_->Initialize()) {
        LOG_ERROR("NetListener:: Cannot initialize socket: %d", errno);
    } else {
        socket_->DisableNagleAlgoritm();
        if (!socket_->Listen(reinterpret_cast<const uint8_t *>(listen_host.data()), listen_port)) {
            LOG_ERROR("NetListener:: Cannot listen on socket: %d", errno);
        }
    }

    status_ = ConStatus::CLOSED;
}

NetListener::ConStatus NetListener::connection_status() const {
    return status_;
}

void NetListener::run() {
    status_ = ConStatus::WAIT;
    LOG_INFO("NetClient:: Start listening");
    client_ = socket_->Accept();
    if (!client_) {
        status_ = ConStatus::CLOSED;
        char buf[100];
        snprintf(buf, sizeof(buf), "Accept on socket returned NULL. errno=%d", errno);
        throw std::runtime_error(buf);
    } else {
        LOG_INFO("NetListener:: Got connection from %s:%d", client_->GetClientAddr(), client_->GetClientPort());
    }
    status_ = ConStatus::ESTABLISHED;
    std::string prev_block;
    while (true) {
        const int32_t nbytes = client_->Receive(1024);
        if (nbytes > 0) {
            auto data = client_->GetData();
            data[nbytes] = '\0';
            LOG_INFO("NetClient:: Message %d bytes, '%s'", nbytes, data);
            //Strategy can send several messages in one block
            const uint8_t *beg = data;
            const uint8_t *block_end = data + nbytes;
            while (true) {
                const uint8_t *end = std::find(beg, block_end, '}');
                if (end == block_end) {
                    if (beg != block_end) {
                        prev_block = std::string(beg, end);
                    }
                    break;
                }
                ++end;
                //Support fragmented messages
                if (prev_block.empty()) {
                    process_json_message(beg, end);
                } else {
                    prev_block += std::string(beg, end);
                    process_json_message(reinterpret_cast<const uint8_t *>(prev_block.data()),
                                         reinterpret_cast<const uint8_t *>(prev_block.data() + prev_block.size()));
                    prev_block.clear();
                }
                beg = end;
            }
        } else {
            socket_->Close();
            status_ = ConStatus::CLOSED;
            break;
        }
    }
}

void NetListener::process_json_message(const uint8_t *chunk_begin, const uint8_t *chunk_end) {
    using namespace nlohmann;
    try {
        auto j = json::parse(chunk_begin, chunk_end);
        PrimitiveType type = primitve_type_from_str(j["type"]);

        if (!frame_) {
            frame_ = std::make_unique<Frame>();
        }

        switch (type) {
            case PrimitiveType::begin:
                LOG_DEBUG("NetClient::Begin");
                break;
            case PrimitiveType::end:
                LOG_DEBUG("NetClient::End");
                scene_->add_frame(std::move(frame_));
                frame_ = nullptr;
                break;
            case PrimitiveType::circle:
                LOG_DEBUG("NetClient::Circle detected");
                frame_->circles.emplace_back(j);
                break;
            case PrimitiveType::rectangle:
                LOG_DEBUG("NetClient::Rectangle detected");
                frame_->rectangles.emplace_back(j);
                break;
            case PrimitiveType::line:
                break;
            case PrimitiveType::message:
                LOG_DEBUG("NetClient::Message");
                frame_->user_message = j["message"];
                break;
            case PrimitiveType::types_count:
                break;
        }
    } catch (const std::exception &e) {
        LOG_WARN("NetListener::Exception: %s", e.what());
    }
}
