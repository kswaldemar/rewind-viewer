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
    friend class UIController;

    explicit Scene(ResourceManager *res);
    ~Scene();

    void render(const glm::mat4 &proj_view);

    ///Set frame to draw now, index should be in range [0, frames_count)
    void set_frame_index(int idx);
    int get_frame_index();

    ///Total loaded frames count
    int get_frames_count();

    ///Arbitrary message written in current frame
    const char *get_frame_user_message();

    ///Called from network listener when next frame is ready
    void add_frame(std::unique_ptr<Frame> &&frame);

private:
    void render_frame(const Frame &frame);
    void render_grid();
    void render_circle(const pod::Circle &circle);
    void render_rectangle(const pod::Rectangle &rect);
    void render_lines(const std::vector<pod::Line> &lines);

    struct settings_t {
        const uint16_t grid_cells_count = 10;
        const glm::vec2 grid_dim = {4000.0f, 4000.0f};
        glm::vec3 grid_color = {0.8f, 0.9f, 0.9f};
    };

    ResourceManager *mgr_;

    Shader color_sh_;
    Shader circle_sh_;
    Shader lines_sh_;

    struct render_attrs_t;
    std::unique_ptr<render_attrs_t> attr_;
    settings_t opt_;

    std::vector<std::unique_ptr<Frame>> frames_;
    int cur_frame_idx_ = 0;

};