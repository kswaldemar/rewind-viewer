//
// Created by valdemar on 14.10.17.
//

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

/**
 * Class representing ShaderProgram
 */
class Shader {
 public:
    static void set_shaders_folder(const std::string &path);

    Shader(const std::string &vertex, const std::string &fragment, const std::string &geom = "");
    ~Shader();

    void use() const;

    GLuint id() const;

    GLint uniform(const std::string &name) const;

    void set_mat4(const std::string &name, const glm::mat4 &v) const;
    void set_mat4(const std::string &name, float *pv) const;
    void set_vec2(const std::string &name, const glm::vec2 &v) const;
    void set_vec3(const std::string &name, const glm::vec3 &v) const;
    void set_vec4(const std::string &name, const glm::vec4 &v) const;
    void set_float(const std::string &name, float val) const;
    void set_int(const std::string &name, GLint val) const;
    void set_uint(const std::string &name, GLuint val) const;

    void bind_uniform_block(const std::string &name, GLuint binding_point) const;

 private:
    static std::string path_to_shaders_;

    GLuint program_ = 0;
};
