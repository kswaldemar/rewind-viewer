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
    const Config::SceneConf &conf_;

    std::unique_ptr<Renderer> renderer_;

    std::mutex frames_mutex_;
    std::vector<std::shared_ptr<Frame>> frames_;
    int cur_frame_idx_ = 0;
    int frames_count_ = 0;
    std::shared_ptr<Frame> active_frame_ = nullptr;
};