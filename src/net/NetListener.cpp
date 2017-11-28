#include "NetListener.h"

#include <net/PrimitiveType.h>
#include <common/logger.h>

#ifdef __APPLE__
#include <errno.h>
#endif

NetListener::NetListener(Scene *scene, const std::string &listen_host, uint16_t listen_port)
    : scene_(scene), host_(listen_host), port_(listen_port) {
    socket_ = std::make_unique<CPassiveSocket>(CPassiveSocket::SocketTypeTcp);
    if (socket_->Initialize()) {
        socket_->DisableNagleAlgoritm();
        if (!socket_->Listen(reinterpret_cast<const uint8_t *>(host_.data()), port_)) {
            LOG_ERROR("NetListener:: Cannot listen on socket: %d", errno);
        }
    } else {
        LOG_ERROR("NetListener:: Cannot initialize socket: %d", errno);
    }
    status_ = ConStatus::CLOSED;
}

NetListener::ConStatus NetListener::connection_status() const {
    return status_;
}

void NetListener::run() {
    while (!stop_) {
        status_ = ConStatus::WAIT;
        LOG_INFO("NetClient:: Start listening");
        CActiveSocket *client_socket = socket_->Accept();
        if (!client_socket) {
            status_ = ConStatus::CLOSED;
            char buf[256];
            snprintf(buf, sizeof(buf), "Accept on socket returned NULL. errno=%d; %s", errno, strerror(errno));
            throw std::runtime_error(buf);
        } else {
            LOG_INFO("NetListener:: Got connection from %s:%d",
                     client_socket->GetClientAddr(), client_socket->GetClientPort());
        }
        status_ = ConStatus::ESTABLISHED;
        //Cleanup previous data
        scene_->clear_data(false);
        frame_ = nullptr;
        //Serve socket
        serve_connection(client_socket);
    }
}

void NetListener::stop() {
    if (status_ != ConStatus::CLOSED) {
        LOG_INFO("Stopping network listening")
    }
    stop_ = true;
}

void NetListener::process_json_message(const uint8_t *chunk_begin, const uint8_t *chunk_end) {
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

        static std::vector<pod::AreaDesc> areas;

        switch (type) {
            case PrimitiveType::begin:
                LOG_V8("NetClient::Begin");
                break;
            case PrimitiveType::end:
                LOG_V8("NetClient::End");
                if (!stop_) {
                    scene_->add_frame(std::move(frame_));
                    if (!areas.empty()) {
                        std::sort(areas.begin(), areas.end(),
                                  [](const pod::AreaDesc &lhs, const pod::AreaDesc &rhs) {
                            return lhs.type < rhs.type;
                        });
                        for (const auto &area : areas) {
                            scene_->add_area_description(area);
                        }
                        areas.clear();
                    }
                }
                frame_ = nullptr;
                break;
            case PrimitiveType::circle:
                LOG_V8("NetClient::Circle detected");
                slice.circles.emplace_back(j);
                break;
            case PrimitiveType::rectangle:
                LOG_V8("NetClient::Rectangle detected");
                slice.rectangles.emplace_back(j);
                break;
            case PrimitiveType::line:
                LOG_V8("NetClient::Line detected");
                slice.lines.emplace_back(j);
                break;
            case PrimitiveType::message:
                LOG_V8("NetClient::Message");
                frame_->user_message += j["message"].get<std::string>();
                break;
            case PrimitiveType::unit:
                LOG_V8("NetClient::Unit");
                frame_->primitives[Frame::DEFAULT_LAYER].units.emplace_back(j);
                break;
            case PrimitiveType::area:
                LOG_V8("NetClient::Area");
                areas.emplace_back(j);
                break;
            case PrimitiveType::popup:
                LOG_V8("NetClient::Popup");
                frame_->popups.emplace_back(j);
                break;
            case PrimitiveType::facility:
                LOG_V8("NetClient::Facility");
                frame_->primitives[Frame::FACILITY_LAYER].facilities.emplace_back(j);
                break;
            case PrimitiveType::types_count:
                LOG_WARN("Got 'types_count' message");
                break;
        }
    } catch (const std::exception &e) {
        LOG_WARN("NetListener::Exception: %s", e.what());
    }
}

void NetListener::serve_connection(CActiveSocket *client) {
    std::string prev_block;
    while (!stop_) {
        const int32_t nbytes = client->Receive(1024);
        if (stop_) {
            break;
        }
        if (nbytes > 0) {
            auto data = client->GetData();
            data[nbytes] = '\0';
            LOG_V9("NetClient:: Message %d bytes, '%s'", nbytes, data);
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
                //Support for fragmented messages
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
            client->Close();
            status_ = ConStatus::CLOSED;
            break;
        }
    }
}
