//
// Created by valdemar on 22.10.17.
//

#pragma once

#include <cgutils/ResourceManager.h>
#include <cgutils/Camera.h>

#include <viewer/Frame.h>
#include <viewer/Config.h>

#include <glm/glm.hpp>

#include <memory>
#include <mutex>

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
    explicit Scene(ResourceManager *res, const Config::SceneConf *conf);
    ~Scene();

    void update_and_render(const glm::mat4 &proj_view, int y_axes_invert);

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

    ///Remove all frames and terrain
    void clear_data(bool clean_active);

    ///True if has at least one frame
    bool has_data() const;

private:
    void render_frame_layer(const Frame::primitives_t &slice);
    void render_grid();
    void render_circle(const pod::Circle &circle);
    void render_rectangle(const pod::Rectangle &rect);
    void render_lines(const std::vector<pod::Line> &lines);

    void render_progress_bar(const glm::vec2 up_left, float w, float h, const glm::vec4 &color);

    ResourceManager *mgr_;
    const Config::SceneConf &conf_;

    struct shaders_t;
    std::unique_ptr<shaders_t> shaders_;

    struct render_attrs_t;
    std::unique_ptr<render_attrs_t> attr_;

    std::mutex frames_mutex_;
    std::vector<std::shared_ptr<Frame>> frames_;
    int cur_frame_idx_ = 0;
    int frames_count_ = 0;
    std::shared_ptr<Frame> active_frame_ = nullptr;

    ///From camera, to properly draw hp bars above units
    int y_axes_invert_;
};