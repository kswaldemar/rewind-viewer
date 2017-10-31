//
// Created by valdemar on 25.10.17.
//

#pragma once

#include <string>

#define X_PRIMITIVE_TYPES_LIST \
    X(begin) \
    X(end) \
    X(circle) \
    X(rectangle) \
    X(line) \
    X(message) \
    X(unit)

enum class PrimitiveType {
#define X(type) type,
    X_PRIMITIVE_TYPES_LIST
#undef X
    types_count
};

inline PrimitiveType primitve_type_from_str(const std::string &str) {
#define X(type) if (str == #type) return PrimitiveType::type;
    X_PRIMITIVE_TYPES_LIST
#undef X
    return PrimitiveType::types_count;
}