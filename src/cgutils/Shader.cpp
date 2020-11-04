//
// Created by valdemar on 14.10.17.
//

#include "Shader.h"

#include <common/logger.h>

#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>

#ifdef __APPLE__
#include <cerrno>
#endif

namespace {

std::string load_file(const std::string &file_path) {
    FILE *fd = fopen(file_path.c_str(), "r");
    if (!fd) {
        char err_buf[512];
        sprintf(err_buf, "Load file(%s): %s", file_path.c_str(), strerror(errno));
        throw std::runtime_error(err_buf);
    }

    constexpr uint16_t CHUNK_SIZE = 256;
    char buf[CHUNK_SIZE + 1];
    std::string content;
    while (size_t sz = fread(buf, 1, CHUNK_SIZE, fd)) {
        buf[sz] = '\0';
        content += buf;
    }
    return content;
}

bool validate_shader(GLuint shader) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar buf[512];
        GLsizei len;
        glGetShaderInfoLog(shader, 512, &len, buf);
        LOG_ERROR("Compile shader:: %*s", len, buf);
        return false;
    }
    return true;
}

bool validate_program(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar buf[512];
        GLsizei len;
        glGetShaderInfoLog(program, 512, &len, buf);
        LOG_ERROR("Error::Link shader program:: %*s", len, buf);
        return false;
    }
    return true;
}

GLuint create_shader(GLenum type, const std::string &source) {
    GLuint shader = glCreateShader(type);
    const auto source_ptr = source.data();
    glShaderSource(shader, 1, &source_ptr, nullptr);
    glCompileShader(shader);
    if (!validate_shader(shader)) {
        LOG_ERROR("Validation error, shader type: %s\n",
                  (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"));
        throw std::runtime_error("Compile shader");
    }
    return shader;
}

GLuint create_shader_program(GLuint vert_shader, GLuint frag_shader, GLuint geom_shader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vert_shader);
    if (geom_shader != 0) {
        glAttachShader(program, geom_shader);
    }
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    if (!validate_program(program)) {
        throw std::runtime_error("Link shader program");
    }
    return program;
}

}  // anonymous namespace

void Shader::set_shaders_folder(const std::string &path) {
    if (path.empty()) {
        LOG_WARN("set_shaders_folder called with empty path");
        return;
    }
    path_to_shaders_ = path;
    if (path_to_shaders_.back() != '/') {
        path_to_shaders_ += '/';
    }
}

std::string Shader::path_to_shaders_;

Shader::Shader(const std::string &vertex, const std::string &fragment,
               const std::string &geom /*=""*/) {
    if (!geom.empty()) {
        LOG_INFO("Start compiling shader in directory '%s': vertex=%s, fragment=%s, geometry=%s",
                 path_to_shaders_.c_str(), vertex.c_str(), fragment.c_str(), geom.c_str());
    } else {
        LOG_INFO("Start compiling shader in directory '%s': vertex=%s, fragment=%s",
                 path_to_shaders_.c_str(), vertex.c_str(), fragment.c_str());
    }

    LOG_INFO("Load Vertex shader");
    const auto vs_src = load_file(path_to_shaders_ + vertex);
    LOG_INFO("Compile Vertex shader");
    auto v_shader = create_shader(GL_VERTEX_SHADER, vs_src);

    LOG_INFO("Load Fragment shader");
    const auto fs_src = load_file(path_to_shaders_ + fragment);
    LOG_INFO("Compile Fragment shader");
    auto f_shader = create_shader(GL_FRAGMENT_SHADER, fs_src);

    GLuint geom_shader = 0;
    if (!geom.empty()) {
        LOG_INFO("Load Geometry shader");
        const auto geom_src = load_file(path_to_shaders_ + geom);
        LOG_INFO("Compile Geometry shader");
        geom_shader = create_shader(GL_GEOMETRY_SHADER, geom_src);
    }

    LOG_INFO("Link shader program");
    program_ = create_shader_program(v_shader, f_shader, geom_shader);

    glDeleteShader(v_shader);
    glDeleteShader(f_shader);
    if (geom_shader != 0) {
        glDeleteShader(geom_shader);
    }
}

Shader::~Shader() {
    glDeleteProgram(program_);
}

void Shader::use() const {
    glUseProgram(program_);
}

GLuint Shader::id() const {
    return program_;
}

GLint Shader::uniform(const std::string &name) const {
    GLint loc = glGetUniformLocation(program_, name.c_str());
    if (loc == -1) {
        LOG_WARN("No such uniform:: %s", name.c_str());
    }
    return loc;
}

void Shader::set_mat4(const std::string &name, const glm::mat4 &v) const {
    glUniformMatrix4fv(uniform(name), 1, GL_FALSE, glm::value_ptr(v));
}

void Shader::set_vec2(const std::string &name, const glm::vec2 &v) const {
    glUniform2f(uniform(name), v.x, v.y);
}

void Shader::set_vec3(const std::string &name, const glm::vec3 &v) const {
    glUniform3f(uniform(name), v.x, v.y, v.z);
}

void Shader::set_vec4(const std::string &name, const glm::vec4 &v) const {
    glUniform4f(uniform(name), v.x, v.y, v.z, v.w);
}

void Shader::set_mat4(const std::string &name, float *pv) const {
    glUniformMatrix4fv(uniform(name), 1, GL_FALSE, pv);
}

void Shader::set_float(const std::string &name, float val) const {
    glUniform1f(uniform(name), val);
}

void Shader::bind_uniform_block(const std::string &name, GLuint binding_point) const {
    auto index = glGetUniformBlockIndex(program_, name.c_str());
    if (index == GL_INVALID_INDEX) {
        LOG_WARN("No such uniform block:: %s", name.c_str());
    } else {
        glUniformBlockBinding(program_, index, binding_point);
    }
}

void Shader::set_int(const std::string &name, GLint val) const {
    glUniform1i(uniform(name), val);
}

void Shader::set_uint(const std::string &name, GLuint val) const {
    glUniform1ui(uniform(name), val);
}
