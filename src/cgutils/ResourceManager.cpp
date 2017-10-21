//
// Created by valdemar on 22.10.17.
//

#include "ResourceManager.h"

#include <common/logger.h>
#include <cstring>

namespace {

} // anonymous namespace

ResourceManager::~ResourceManager() {
    glDeleteBuffers(static_cast<GLsizei>(buffers_.size()), buffers_.data());
    glDeleteVertexArrays(static_cast<GLsizei>(vaos_.size()), vaos_.data());
}

GLuint ResourceManager::gen_vertex_array() {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    vaos_.push_back(vao);
    return vao;
}

GLuint ResourceManager::gen_buffer() {
    GLuint buf;
    glGenBuffers(1, &buf);
    buffers_.push_back(buf);
    return buf;
}
