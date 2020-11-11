#include "ProtoHandler.h"

#include <common/logger.h>

ProtoHandler::ProtoHandler(Scene *scene) : scene_(scene) {}

void ProtoHandler::on_new_connection() {
    scene_->clear_data();
    reset_state();
}

void ProtoHandler::set_immediate_mode(bool enabled) {
    send_mode_ = enabled ? Mode::IMMEDIATE : Mode::BATCH;
}

void ProtoHandler::on_message_processed(bool end_frame) {
    if (send_mode_ == Mode::BATCH && !end_frame) {
        return;
    }

    if (immediate_data_sent_) {
        // Update last frame, because something already sent to it
        scene_->add_frame_data(*frame_);
    } else {
        // Add new frame, nothing was appended to last one
        scene_->add_frame(std::move(frame_));
        frame_ = nullptr;
    }
    immediate_data_sent_ = !end_frame;
    scene_->add_permanent_frame_data(permanent_frame_);

    if (end_frame) {
        reset_state();
    } else {
        if (!frame_) {
            frame_ = std::make_shared<FrameEditor>();
            frame_->set_layer_id(last_layer_id_);
        } else {
            frame_->clear();
        }
        permanent_frame_.clear();
    }
}

FrameEditor &ProtoHandler::get_frame_editor() {
    return use_permanent_ ? permanent_frame_ : *frame_;
}

void ProtoHandler::use_permanent_frame(bool use) {
    use_permanent_ = use;
}

void ProtoHandler::reset_state() {
    permanent_frame_.clear();
    frame_ = std::make_shared<FrameEditor>();
    use_permanent_ = false;
    immediate_data_sent_ = false;
}

void ProtoHandler::set_layer(size_t layer) {
    const auto clamped_layer = cg::clamp<size_t>(layer, 1, Frame::LAYERS_COUNT);
    if (layer < 1 || layer > static_cast<size_t>(Frame::LAYERS_COUNT)) {
        LOG_WARN("Incorrect layer id %zu, should be in range 1-%zu. Will use %zu instead", layer,
                 static_cast<size_t>(Frame::LAYERS_COUNT), clamped_layer);
    }

    last_layer_id_ = clamped_layer - 1;
    frame_->set_layer_id(last_layer_id_);
    permanent_frame_.set_layer_id(last_layer_id_);
}
