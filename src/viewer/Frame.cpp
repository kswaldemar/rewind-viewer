#include "Frame.h"

#include <cassert>

void Frame::update_from(const Frame::context_collection_t &from_contexts) {
    for (size_t i = 0; i < contexts_.size(); ++i) {
        contexts_[i].update_from(from_contexts[i]);
    }
}

void Frame::update_from(const Frame &other) {
    update_from(other.all_contexts());

    // copy popups
    for (size_t i = 0; i < popups_.size(); ++i) {
        const auto &from = other.popups_[i];
        auto &to = popups_[i];

        to.reserve(to.size() + from.size());
        for (const auto &popup : from) {
            to.push_back(popup);
        }
    }

    user_message_ += other.user_message_;
}
const Frame::context_collection_t &Frame::all_contexts() const {
    return contexts_;
}

const Frame::popup_collection_t &Frame::all_popups() const {
    return popups_;
}

const char *Frame::user_message() const {
    return user_message_.c_str();
}
