//
// Created by Vladimir A. Kiselev on 04.11.2020.
//
#include "PrimitiveType.h"

#include <common/logger.h>

#include <unordered_map>

PrimitiveType primitve_type_from_str(const std::string &str) {
    static const std::unordered_map<std::string, PrimitiveType> mapping = {
        {"circle", PrimitiveType::CIRCLE},
        {"rectangle", PrimitiveType::RECTANGLE},
        {"triangle", PrimitiveType::TRIANGLE},
        {"polyline", PrimitiveType::POLYLINE},
        {"message", PrimitiveType::MESSAGE},
        {"popup", PrimitiveType::POPUP},
        {"options", PrimitiveType::OPTIONS},
        {"end", PrimitiveType::END}};

    auto it = mapping.find(str);
    if (it != mapping.end()) {
        return it->second;
    }

    std::string valid_values = "[";
    for (auto v = mapping.begin(); v != mapping.end(); ++v) {
        valid_values += v->first;
        if (std::next(v) != mapping.end()) {
            valid_values += ", ";
        } else {
            valid_values += "]";
        }
    }

    LOG_ERROR("Unknown primitve type '%s', should be on of %s", str.c_str(), valid_values.c_str());
    return PrimitiveType::TYPES_COUNT;
}
