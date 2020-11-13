//
// Created by valdemar on 21.10.17.
//

#pragma once

#include <glad/glad.h>

#include <cstdint>

namespace cg {
namespace details {

void gl_check_error(const char *file, int line);

}  // namespace details

template <typename T>
constexpr void *offset(uint16_t shift) {
    return reinterpret_cast<void *>(shift * sizeof(T));
}

template <typename T>
constexpr inline T clamp(T value, T min_val, T max_val) {
    return value < min_val ? min_val : value > max_val ? max_val : value;
}

constexpr inline double lerp(double v, double v0, double v1, double t0, double t1) {
    return (v - v0) / (v1 - v0) * (t1 - t0) + t0;
}

void APIENTRY debug_output_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                                    GLsizei length, const GLchar *message, const void *userParam);

}  // namespace cg

#define glCheckError() cg::details::gl_check_error(__FILE__, __LINE__)
