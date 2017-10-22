//
// Created by valdemar on 22.10.17.
//

#pragma once

#include <cgutils/ResourceManager.h>
#include <cgutils/Shader.h>
#include <cgutils/Camera.h>

#include <glm/glm.hpp>

#include <memory>

/**
 * Represent whole game state.
 *  - contains all frames
 *  - draw current frame
 *  - drawing grid and background texture, maybe some very static data like terrain
 *  - get new Frame from NetClient
 *  - configurable from UI
 */
class Scene {
public:
    explicit Scene(ResourceManager *res);
    ~Scene();

    void render(const glm::mat4 &view, const glm::mat4 &proj);

private:
    void render_grid();
    void render_fancy_triangle();

    struct settings_t {
        const uint16_t grid_cells_count = 100;
        const glm::vec2 grid_dim = {100.0f, 100.0f};
        const glm::vec3 grid_color = {0.8f, 0.9f, 0.9f};

        glm::vec2 fancy_triangle_pos_ = {2.0f, 2.0f};
    };

    ResourceManager *rsm_;

    struct render_attrs_t;
    std::unique_ptr<render_attrs_t> attr_;
    settings_t settings_;

    Shader color_sh_;
};