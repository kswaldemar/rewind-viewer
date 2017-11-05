//
// Created by valdemar on 22.10.17.
//

#pragma once

#include <cgutils/ResourceManager.h>
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

    ///Show detailed info in tooltip if mouse hover unit
    void show_detailed_info(const glm::vec2 &mouse) const;

private:
    struct settings_t {
        const uint16_t grid_cells_count = 10;
        const glm::vec2 grid_dim = {4000.0f, 4000.0f};
        glm::vec3 grid_color = {0.115f, 0.124f, 0.157f};
        bool show_full_hp_bars = false;
        bool show_detailed_info_on_hover = true;
    };

    void render_frame(const Frame &frame);
    void render_grid();
    void render_circle(const pod::Circle &circle);
    void render_rectangle(const pod::Rectangle &rect);
    void render_lines(const std::vector<pod::Line> &lines);
    void render_unit(const pod::Unit &unit);

    ResourceManager *mgr_;

    struct shaders_t;
    std::unique_ptr<shaders_t> shaders_;

    struct render_attrs_t;
    std::unique_ptr<render_attrs_t> attr_;
    settings_t opt_;

    std::vector<std::unique_ptr<Frame>> frames_;
    int cur_frame_idx_ = 0;

    std::map<Frame::UnitType, GLuint> unit2tex_;
};