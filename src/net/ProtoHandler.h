//
// Created by valdemar on 20.11.18.
//

#pragma once

#include <viewer/Scene.h>


class ProtoHandler {
public:
    ProtoHandler(Scene *scene);
    ///Called whenever data from socket should be processed
    /// data should be copied if wanted to be used after function call
    virtual void handle_message(const uint8_t *data, uint32_t nbytes) = 0;

    ///Any saved data from old messages should be cleared on this call
    virtual void on_new_connection();

protected:
    ///Should be called when frame is finished (end message arrived)
    void send_to_scene(std::unique_ptr<Frame> &&frame);

private:
    Scene *scene_;
};