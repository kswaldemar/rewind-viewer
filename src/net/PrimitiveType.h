//
// Created by valdemar on 25.10.17.
//

#pragma once

#include <string>

//@formatter:off
enum class PrimitiveType {
    CIRCLE,
    RECTANGLE,
    TRIANGLE,
    POLYLINE,
    MESSAGE,
    POPUP,
    OPTIONS,
    END,

    TYPES_COUNT
};
//@formatter:on

PrimitiveType primitve_type_from_str(const std::string &str);
