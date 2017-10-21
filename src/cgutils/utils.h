//
// Created by valdemar on 21.10.17.
//

#pragma once

#include <cstdint>

namespace cg {

template<typename T>
constexpr void *offset(uint16_t shift) {
    return reinterpret_cast<void *>(shift * sizeof(T));
}

template<typename T>
constexpr inline T clamp(T value, T min_val, T max_val) {
    if (value < min_val) {
        return min_val;
    }
    if (value > max_val) {
        return max_val;
    }
    return value;
}

}