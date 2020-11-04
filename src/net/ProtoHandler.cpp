#include "ProtoHandler.h"

#include <common/logger.h>

ProtoHandler::ProtoHandler(Scene *scene) : scene_(scene) {}

void ProtoHandler::on_new_connection() {
    scene_->clear_data(false);
    reset_state();
}

void ProtoHandler::on_frame_end() {
    scene_->add_frame(std::move(frame_));
    scene_->add_permanent_frame_data(permanent_frame_);
    reset_state();
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
}

void ProtoHandler::set_layer(size_t layer) {
    const auto clamped_layer = cg::clamp<size_t>(layer, 1, Frame::LAYERS_COUNT);
    if (layer < 1 || layer > static_cast<size_t>(Frame::LAYERS_COUNT)) {
        LOG_WARN("Incorrect layer id %zu, should be in range 1-%zu. Will use %zu instead", layer,
                 static_cast<size_t>(Frame::LAYERS_COUNT), clamped_layer);
    }

    frame_->set_layer_id(clamped_layer - 1);
    permanent_frame_.set_layer_id(clamped_layer - 1);
}
