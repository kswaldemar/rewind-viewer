//
// Created by valdemar on 10.02.2020.
//
#pragma once

#include "cgutils/utils.h"
#include "cgutils/ResourceManager.h"

#include <glm/glm.hpp>

#include <vector>
#include <memory>


class ShaderCollection;

/**
 * RenderContext
 */
class RenderContext {
public:
    struct context_vao_t {
        GLuint point_vao;
        GLuint circle_vao;
    };
    static context_vao_t create_gl_context(ResourceManager &res);

    RenderContext();
    ~RenderContext();

    ///Filled circle
    void add_circle(glm::vec2 center, float r, glm::vec4 color);

    ///Polyline
    void add_polyline(const std::vector<glm::vec2> &points, glm::vec4 color);

    ///Convex polygone, points should be passed in clockwise order
    void add_polygone(const std::vector<glm::vec2> &points, glm::vec4 color);

    void draw(const context_vao_t &vaos, const ShaderCollection &shaders) const;

private:
    struct memory_layout_t;
    std::unique_ptr<memory_layout_t> impl_;
};
