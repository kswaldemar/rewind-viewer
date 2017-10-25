//
// Created by valdemar on 22.10.17.
//

#pragma once

#include <viewer/Scene.h>

#include <csimplesocket/PassiveSocket.h>
#include <csimplesocket/ActiveSocket.h>

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

    NetListener(Scene *scene, const std::string &listen_host, uint16_t listen_port);

    ///Return current connection status.
    ///Will be wait until first data chunk, established while tcp connection alive and closed in case of error
    ///or connection termination
    ConStatus connection_status() const;

    ///Start gathering and operating information from socket
    ///Blocking call, should be running on personal thread
    void run();

private:
    Scene *scene_;
    std::unique_ptr<CPassiveSocket> socket_;
    CActiveSocket *client_ = nullptr;
    ConStatus status_;
};