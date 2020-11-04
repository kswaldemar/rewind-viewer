#include "ResourceManager.h"

#include <common/logger.h>

#include <stb_image.h>

#include <utility>

ResourceManager::ResourceManager(std::string path_to_res_folder)
    : res_folder_(std::move(path_to_res_folder)) {
    if (!res_folder_.empty() && res_folder_.back() != '/') {
        res_folder_ += '/';
    }
    stbi_set_flip_vertically_on_load(true);
}

ResourceManager::~ResourceManager() {
    glDeleteBuffers(static_cast<GLsizei>(buffers_.size()), buffers_.data());
    glDeleteVertexArrays(static_cast<GLsizei>(vaos_.size()), vaos_.data());
    glDeleteTextures(static_cast<GLsizei>(textures_.size()), textures_.data());
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

GLuint ResourceManager::load_texture(const std::string &path_to_texture, bool gen_mipmap,
                                     GLint wrap_s, GLint wrap_t, GLint flt_min, GLint flt_mag) {
    int width;
    int height;
    int nr_channels;
    auto absolute_path = res_folder_ + path_to_texture;
    unsigned char *data = stbi_load(absolute_path.c_str(), &width, &height, &nr_channels, 0);
    if (!data) {
        LOG_WARN("Cannot load texture %s", absolute_path.c_str());
        return 0;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    textures_.push_back(tex);

    GLenum format;
    if (nr_channels == 3) {
        format = GL_RGB;
    } else if (nr_channels == 4) {
        format = GL_RGBA;
    } else {
        format = GL_RED;
    }

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    if (gen_mipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, flt_min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, flt_mag);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    return tex;
}
