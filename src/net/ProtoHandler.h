//
// Created by valdemar on 20.11.18.
//

#pragma once

#include <viewer/Scene.h>
#include <viewer/FrameEditor.h>

class ProtoHandler {
public:
    explicit ProtoHandler(Scene *scene);
    virtual ~ProtoHandler() = default;

    ///Called whenever data from socket should be processed
    /// data should be copied if wanted to be used after function call
    virtual void handle_message(const uint8_t *data, uint32_t nbytes) = 0;

    ///Any saved data from old messages should be cleared on this call
    virtual void on_new_connection();

protected:
    ///Should be called by specific handler when 'end_frame' received
    void on_frame_end();

    ///Get frame editor for normal or permanent frame
    ///Specific handler should use it to create primitives
    FrameEditor &get_frame_editor();

    ///Whenever to use normal or permanent frame for drawing
    void use_permanent_frame(bool use);

private:
    void reset_state();

    Scene *scene_;
    FrameEditor frame_editors_[2];
    bool use_permanent_ = false;
};