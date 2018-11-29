#include "Scene.h"
#include "Renderer.h"

#include <cgutils/utils.h>
#include <cgutils/Shader.h>
#include <common/logger.h>

#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>

namespace {

bool hittest(const glm::vec2 &wmouse, const pod::Popup &popup) {
    auto d = wmouse - popup.center;
    return (d.x * d.x + d.y * d.y) <= popup.radius * popup.radius;
}

} // anonymous namespace


Scene::Scene(ResourceManager *res, const Config::SceneConf *conf)
    : conf_(*conf) {

    renderer_ = std::make_unique<Renderer>(res, conf_.grid_dim, conf_.grid_cells);
}

Scene::~Scene() = default;

void Scene::update_and_render(const Camera &cam) {
    //Update current frame
    {
        std::lock_guard<std::mutex> f(frames_mutex_);
        frames_count_ = static_cast<int>(frames_.size());
        if (cur_frame_idx_ >= 0 && cur_frame_idx_ < frames_count_) {
            active_frame_ = frames_[cur_frame_idx_];
        }
    }

    renderer_->update_frustum(cam);
    renderer_->render_background(conf_.scene_color);

    //Grid
    if (conf_.show_grid) {
        renderer_->render_grid(conf_.grid_color);
    }

    //Frame
    if (active_frame_) {
        for (size_t idx = 0; idx < active_frame_->primitives.size(); ++idx) {
            if (conf_.enabled_layers[idx]) {
                renderer_->render_frame_layer(active_frame_->primitives[idx]);
            }
        }
    }
}

void Scene::set_frame_index(int idx) {
    cur_frame_idx_ = cg::clamp(idx, 0, frames_count_ - 1);
}

int Scene::get_frame_index() {
    return cur_frame_idx_;
}

int Scene::get_frames_count() {
    return frames_count_;
}

const char *Scene::get_frame_user_message() {
    if (active_frame_) {
        return active_frame_->user_message.c_str();
    }
    return "";
}

void Scene::add_frame(std::unique_ptr<Frame> &&frame) {
    std::lock_guard<std::mutex> f(frames_mutex_);
    //Sort units for proper draw order
    frames_.emplace_back(std::move(frame));
}

void Scene::show_detailed_info(const glm::vec2 &mouse) const {
    if (!active_frame_) {
        return;
    }

    for (const auto &popup : active_frame_->popups) {
        if (hittest(mouse, popup)) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", popup.text.c_str());
            ImGui::EndTooltip();
        }
    }
}

void Scene::clear_data(bool clean_active) {
    std::lock_guard<std::mutex> _(frames_mutex_);
    frames_.clear();
    if (clean_active) {
        active_frame_ = nullptr;
    }
    frames_count_ = 0;
    cur_frame_idx_ = 0;
}

bool Scene::has_data() const {
    return frames_count_ > 0;
}

