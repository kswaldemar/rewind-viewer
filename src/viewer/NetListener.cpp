#include "NetListener.h"

#include <common/logger.h>

NetListener::NetListener(Scene *scene, const std::string &listen_host, uint16_t listen_port)
    : scene_(scene)
{
    socket_ = std::make_unique<CPassiveSocket>(CPassiveSocket::SocketTypeTcp);
    if (!socket_->Initialize()) {
        LOG_ERROR("NetListener:: Cannot initialize socket: %d", errno);
    } else if (!socket_->Listen(reinterpret_cast<const uint8_t *>(listen_host.data()), listen_port)) {
        LOG_ERROR("NetListener:: Cannot listen on socket: %d", errno);
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
        socket_->SetReceiveTimeout(10);
        LOG_INFO("NetListener:: Got connection from %s:%d", client_->GetClientAddr(), client_->GetClientPort());
    }
    status_ = ConStatus::ESTABLISHED;

    while (true) {
        const int32_t nbytes = client_->Receive(1024);
        if (nbytes > 0) {
            LOG_INFO("NetClient:: Message %d bytes, '%s'", nbytes, client_->GetData());
        } else {
            socket_->Close();
            status_ = ConStatus::CLOSED;
            break;
        }
    }
}
