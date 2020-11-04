#include "ProtoHandler.h"

namespace {

const size_t PERMANENT_IDX = 1;
const size_t NORMAL_IDX = 0;

}  // anonymous namespace

ProtoHandler::ProtoHandler(Scene *scene) : scene_(scene) {}

void ProtoHandler::on_new_connection() {
    scene_->clear_data(false);
    reset_state();
}

void ProtoHandler::on_frame_end() {
    scene_->add_frame(frame_editors_[NORMAL_IDX]);
    scene_->add_permanent_frame_data(frame_editors_[PERMANENT_IDX]);
    reset_state();
}

FrameEditor &ProtoHandler::get_frame_editor() {
    return frame_editors_[use_permanent_];
}

void ProtoHandler::use_permanent_frame(bool use) {
    use_permanent_ = use;
}

void ProtoHandler::reset_state() {
    frame_editors_[NORMAL_IDX].clear();
    frame_editors_[NORMAL_IDX].set_layer_id(Frame::DEFAULT_LAYER);

    frame_editors_[PERMANENT_IDX].clear();
    frame_editors_[PERMANENT_IDX].set_layer_id(Frame::DEFAULT_LAYER);

    use_permanent_ = false;
}
