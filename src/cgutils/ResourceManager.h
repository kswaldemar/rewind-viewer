//
// Created by valdemar on 22.10.17.
//

#pragma once

#include <glad/glad.h>

#include <map>
#include <string>
#include <vector>

/**
 * Class for handling any OpenGl resources - vertex arrays, buffers, textures, shaders etc.
 */
class ResourceManager {
 public:
    explicit ResourceManager(std::string path_to_res_folder = "");
    ~ResourceManager();

    GLuint gen_vertex_array();
    GLuint gen_buffer();
    GLuint load_texture(const std::string &path_to_texture, bool gen_mipmap = true,
                        GLint wrap_s = GL_CLAMP_TO_EDGE, GLint wrap_t = GL_CLAMP_TO_EDGE,
                        GLint flt_min = GL_LINEAR, GLint flt_mag = GL_LINEAR);

 private:
    std::vector<GLuint> vaos_;
    std::vector<GLuint> buffers_;
    std::vector<GLuint> textures_;

    std::string res_folder_;
};
