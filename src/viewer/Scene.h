//
// Created by valdemar on 22.10.17.
//

#pragma once

#include <cgutils/Camera.h>
#include <cgutils/ResourceManager.h>

#include <viewer/Config.h>
#include <viewer/Frame.h>

#include <glm/glm.hpp>

#include <memory>
#include <mutex>

class Renderer;

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

    void update_and_render(const Camera &cam);

    /// Set frame to draw now, index should be in range [0, frames_count)
    void set_frame_index(int idx);
    int get_frame_index() const;

    /// Total loaded frames count
    int get_frames_count() const;

    /// Arbitrary message written in current frame
    const char *get_frame_user_message();

    /// Called from network listener when next frame is ready
    void add_frame(const Frame &frame);

    /// Add primitives to permanent frame
    void add_permanent_frame_data(const Frame &data);

    /// Show detailed info in tooltip if mouse hover unit
    void show_detailed_info(const glm::vec2 &mouse) const;

    /// Remove all frames and terrain
    void clear_data(bool clean_active);

    /// True if has at least one frame
    bool has_data() const;

 private:
    const Config::SceneConf &conf_;

    std::unique_ptr<Renderer> renderer_;

    std::mutex frames_mutex_;
    std::vector<std::shared_ptr<Frame>> frames_;
    int cur_frame_idx_ = 0;
    int frames_count_ = 0;
    std::shared_ptr<Frame> active_frame_ = nullptr;

    // Permanent frame get rendered each time before active_frame
    Frame permanent_frame_;

    std::atomic_flag lock_permanent_frame_ = ATOMIC_FLAG_INIT;
};