#include "ProtoHandler.h"

ProtoHandler::ProtoHandler(Scene *scene)
    : scene_(scene)
{}

void ProtoHandler::on_new_connection() {
    scene_->clear_data(false);
}

void ProtoHandler::send_to_scene(std::unique_ptr<Frame> &&frame) {
    scene_->add_frame(std::move(frame));
}
