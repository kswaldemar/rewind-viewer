//
// Created by valdemar on 10.02.2020.
//
#pragma once

#include "cgutils/ResourceManager.h"
#include "cgutils/utils.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

struct ShaderCollection;

/**
 * RenderContext
 */
class RenderContext {
 public:
    struct context_vao_t {
        GLuint point_vao;
        GLuint circle_vao;

        GLuint point_vbo;
        GLuint circle_vbo;

        GLuint common_ebo;
    };
    static context_vao_t create_gl_context(ResourceManager &res);

    RenderContext();
    ~RenderContext();

    using TriangleColors = std::array<glm::vec4, 3>;
    using RectangleColors = std::array<glm::vec4, 4>;

    void add_circle(glm::vec2 center, float r, glm::vec4 color, bool fill);
    void add_triangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec4 color, bool fill);
    void add_rectangle(glm::vec2 top_left, glm::vec2 bottom_right, glm::vec4 color, bool fill);
    void add_polyline(const std::vector<glm::vec2> &points, glm::vec4 color);

    // Versions with gradient support
    void add_triangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, const TriangleColors &colors,
                      bool fill);
    void add_rectangle(glm::vec2 top_left, glm::vec2 bottom_right, const RectangleColors &colors,
                       bool fill);

    /// Add all primitves from other RenderContext
    void update_from(const RenderContext &other);

    /// Remove everything
    void clear();

    void draw(const context_vao_t &vaos, const ShaderCollection &shaders) const;

 private:
    struct memory_layout_t;
    std::unique_ptr<memory_layout_t> impl_;
};
