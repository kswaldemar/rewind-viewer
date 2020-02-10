//
// Created by valdemar on 29.11.18.
//

#pragma once

#include "ShaderCollection.h"
#include "RenderContext.h"

#include <cgutils/Camera.h>
#include <cgutils/ResourceManager.h>

#include <glm/glm.hpp>

#include <memory>

class Renderer {
public:
    Renderer(ResourceManager *res, glm::u32vec2 area_size, glm::u16vec2 grid_cells);
    ~Renderer();

    void update_frustum(const Camera &cam);

    void render_background(glm::vec3 color);
    void render_grid(glm::vec3 color);
    void render_frame_layer(const Frame::primitives_t &slice);

private:
    void do_render_circle(const pod::Circle &circle);
    void do_render_rectangle(const pod::Rectangle &rect);
    void do_render_lines(const std::vector<pod::Line> &lines);

    ResourceManager *mgr_;

    struct shaders_t;
    std::unique_ptr<shaders_t> shaders_;

    //TODO: Убрать unique_ptr когда shader folder будут удалены старые шейдеры и shader folder будет выставлен в дефолт
    std::unique_ptr<ShaderCollection> shaders2_;
    RenderContext::context_vao_t gl_ctx_;
    RenderContext test_context_;

    struct render_attrs_t;
    std::unique_ptr<render_attrs_t> attr_;

    glm::vec2 area_size_;
    glm::u16vec2 grid_cells_;
};