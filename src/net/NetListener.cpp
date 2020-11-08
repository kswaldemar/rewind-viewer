#include "NetListener.h"
#include "ProtoHandler.h"

#include <common/logger.h>
#include <net/PrimitiveType.h>

#ifdef __APPLE__
#include <cerrno>
#include <utility>
#endif

NetListener::NetListener(std::string listen_host, uint16_t listen_port,
                         std::unique_ptr<ProtoHandler> &&handler)
    : host_(std::move(listen_host)), port_(listen_port), handler_(std::move(handler)) {
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
            if (stop_) {
                return;
            }

            status_ = ConStatus::CLOSED;
            char buf[256];
            snprintf(buf, sizeof(buf), "Accept on socket returned NULL. errno=%d; %s", errno,
                     strerror(errno));
            throw std::runtime_error(buf);
        } else {
            LOG_INFO("NetListener:: Got connection from %s:%u", client_socket->GetClientAddr(),
                     static_cast<uint16_t>(client_socket->GetClientPort()));
        }
        status_ = ConStatus::ESTABLISHED;
        // Cleanup previous data
        handler_->on_new_connection();
        // Serve socket
        serve_connection(client_socket);
    }
}

void NetListener::set_immediate_mode(bool enable) {
    immediate_mode_.store(enable);
}

void NetListener::stop() {
    if (status_ != ConStatus::CLOSED) {
        LOG_INFO("Stopping network listening");
    }
    stop_ = true;
}

void NetListener::serve_connection(CActiveSocket *client) {
    while (!stop_) {
        const int32_t nbytes = client->Receive(1024);
        if (stop_) {
            break;
        }
        if (nbytes > 0) {
            auto data = client->GetData();
            // todo: Maybe remove that zero, because it doesn't make sense in case of binary data
            //  same for debug print
            data[nbytes] = '\0';
            LOG_V9("NetClient:: Message %d bytes, '%s'", nbytes, data);
            handler_->set_immediate_mode(immediate_mode_.load());
            // Strategy can send several messages in one block
            handler_->handle_message(data, static_cast<uint32_t>(nbytes));
        } else {
            client->Close();
            status_ = ConStatus::CLOSED;
            break;
        }
    }
}
