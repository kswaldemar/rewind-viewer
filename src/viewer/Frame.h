//
// Created by valdemar on 22.10.17.
//

#pragma once

#include <viewer/Primitives.h>

/**
 * Represent one game frame.
 *  - contains all primitives to be drawn
 *  - contains user message to draw in message window
 */
struct Frame {
    std::vector<pod::Circle> circles;
    std::string user_message;
};