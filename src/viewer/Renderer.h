//
// Created by valdemar on 29.11.18.
//

#pragma once

#include "RenderContext.h"
#include "ShaderCollection.h"

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
    void render_primitives(const RenderContext &ctx);

 private:
    ResourceManager *mgr_;

    std::unique_ptr<ShaderCollection> shaders_;
    RenderContext::context_vao_t ctx_render_params_;

    struct render_attrs_t;
    std::unique_ptr<render_attrs_t> attr_;

    glm::vec2 area_size_;
    glm::u16vec2 grid_cells_;
};
