//
// Created by valdemar on 22.10.17.
//

#pragma once

#include <viewer/Scene.h>

#include <csimplesocket/PassiveSocket.h>
#include <csimplesocket/ActiveSocket.h>

class ProtoHandler;

/**
 * Negotiation with running strategy
 *  - listen socket, read json primitives
 *  - decode json primitives to one of Primitives.h
 *  - configure Frame object
 *  - send Frame to Scene when ready
 *  - running in personal thread
 */
class NetListener {
public:
    enum class ConStatus {
        WAIT,
        ESTABLISHED,
        CLOSED
    };

    NetListener(const std::string &listen_host, uint16_t listen_port, std::unique_ptr<ProtoHandler> &&handler);

    ///Return current connection status.
    ///Will be wait until first data chunk, established while tcp connection alive
    ConStatus connection_status() const;

    ///Start gathering and operating information from socket
    ///Blocking call, should be running on personal thread
    void run();

    void stop();

private:
    void serve_connection(CActiveSocket *client);

    std::unique_ptr<CPassiveSocket> socket_;
    ConStatus status_;

    std::string host_;
    uint16_t port_;

    std::unique_ptr<ProtoHandler> handler_;

    std::atomic<bool> stop_{false};
};