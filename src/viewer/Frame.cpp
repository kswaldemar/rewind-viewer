#include "Frame.h"

#include <cassert>

void Frame::update_from(const Frame &other) {
    for (size_t i = 0; i < contexts_.size(); ++i) {
        contexts_[i].update_from(other.contexts_[i]);
    }
}

const Frame::context_collection_t &Frame::all_contexts() const {
    return contexts_;
}

const std::vector<Frame::hittest_t> &Frame::popups() const {
    return popups_;
}

const char *Frame::user_message() const {
    return user_message_.c_str();
}
