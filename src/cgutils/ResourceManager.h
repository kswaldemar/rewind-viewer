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
    ~ResourceManager();

    GLuint gen_vertex_array();
    GLuint gen_buffer();

private:
    std::vector<GLuint> vaos_;
    std::vector<GLuint> buffers_;
};




