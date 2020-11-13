//
// Created by valdemar on 22.10.17.
//

#pragma once

#include <cgutils/Camera.h>
#include <cgutils/ResourceManager.h>
#include <common/Spinlock.h>
#include <viewer/Config.h>
#include <viewer/FrameEditor.h>

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

    /// @note: Called from render thread
    void update_and_render(const Camera &cam);

    /// Set frame to draw now, index should be in range [0, frames_count)
    /// @note Called from render thread
    void set_frame_index(int idx);

    /// @note Called from render thread
    int get_frame_index() const;

    /// Total loaded frames count
    /// @note Called from render thread
    int get_frames_count() const;

    /// Arbitrary message written in current frame
    /// @note Called from render thread
    const char *get_frame_user_message();

    /// Called from network listener when next frame is ready
    void add_frame(std::shared_ptr<Frame> frame);

    /// Add data to last appended frame
    /// @note Called from network thread
    void add_frame_data(const Frame &data);

    /// Add primitives to permanent frame
    /// @note Called from network thread
    void add_permanent_frame_data(const Frame &data);

    /// Show detailed info in tooltip if mouse hover unit
    /// @note Called from render thread
    void show_detailed_info(const glm::vec2 &mouse) const;

    /// Remove all frames and clear permanent frame
    /// @note May be called from network thread or render thread
    void clear_data();

    /// True if has at least one frame
    /// @note Called from render thread
    bool has_data() const;

 private:
    const Config::SceneConf &conf_;

    std::unique_ptr<Renderer> renderer_;

    Spinlock frame_access_lock_;

    int cur_frame_idx_ = 0;
    int frames_count_ = 0;
    std::shared_ptr<Frame> active_frame_ = nullptr;
    std::vector<std::shared_ptr<Frame>> frames_;

    /// Permanent frame rendered each time before active_frame
    /// Use FrameEditor to clear() on clear_data() calls
    FrameEditor permanent_frame_;
};
