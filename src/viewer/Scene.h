//
// Created by valdemar on 22.10.17.
//

#pragma once

#include <cgutils/ResourceManager.h>
#include <cgutils/Shader.h>
#include <cgutils/Camera.h>

#include <viewer/Frame.h>

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

    ///Set currently drawed frame, index should be in range [0, frames_count)
    void set_frame_index(int idx);
    int get_frame_index();

    ///Total loaded frames count
    int get_frames_count();

    ///Arbitrary message written in current frame
    const char *get_frame_user_message();

private:
    void render_grid();
    void render_fancy_triangle();

    struct settings_t {
        const uint16_t grid_cells_count = 100;
        const glm::vec2 grid_dim = {100.0f, 100.0f};
        const glm::vec3 grid_color = {0.8f, 0.9f, 0.9f};

        glm::vec2 fancy_triangle_pos_ = {2.0f, 2.0f};
        glm::vec3 clear_color = {0.447, 0.565, 0.604};
    };

    ResourceManager *rsm_;

    Shader color_sh_;

    struct render_attrs_t;
    std::unique_ptr<render_attrs_t> attr_;
    settings_t opt_;

    std::vector<Frame> frames_;
    int cur_frame_idx_ = 0;
};