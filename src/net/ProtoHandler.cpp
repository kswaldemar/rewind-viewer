#include "ProtoHandler.h"

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
