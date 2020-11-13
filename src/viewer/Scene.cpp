#include "Scene.h"
#include "Renderer.h"

#include <cgutils/utils.h>

#include <imgui.h>

#include <unordered_map>

using SpinGuard = std::unique_lock<Spinlock>;

Scene::Scene(ResourceManager *res, const Config::SceneConf *conf) : conf_(*conf) {
    renderer_ = std::make_unique<Renderer>(res, conf_.grid_dim, conf_.grid_cells);
}

Scene::~Scene() = default;

void Scene::update_and_render(const Camera &cam) {
    // Update current frame
    {
        SpinGuard lock(frame_access_lock_);
        frames_count_ = static_cast<int>(frames_.size());
        if (cur_frame_idx_ >= 0 && cur_frame_idx_ < frames_count_) {
            active_frame_ = frames_[cur_frame_idx_];
        }
    }

    renderer_->update_frustum(cam);
    renderer_->render_background(conf_.scene_color);

    // Grid
    if (conf_.show_grid) {
        renderer_->render_grid(conf_.grid_color);
    }

    // Draw currently selected frame
    if (active_frame_) {
        SpinGuard lock(frame_access_lock_);
        const auto &perm_frame_contexts = permanent_frame_.all_contexts();
        const auto &frame_contexts = active_frame_->all_contexts();
        for (size_t idx = 0; idx < Frame::LAYERS_COUNT; ++idx) {
            if (conf_.enabled_layers[idx]) {
                renderer_->render_primitives(perm_frame_contexts[idx]);
                renderer_->render_primitives(frame_contexts[idx]);
            }
        }
    }
}

void Scene::set_frame_index(int idx) {
    cur_frame_idx_ = cg::clamp(idx, 0, frames_count_ - 1);
}

int Scene::get_frame_index() const {
    return cur_frame_idx_;
}

int Scene::get_frames_count() const {
    return frames_count_;
}

const char *Scene::get_frame_user_message() {
    if (active_frame_) {
        return active_frame_->user_message();
    }
    return "";
}

void Scene::add_frame(std::shared_ptr<Frame> frame) {
    SpinGuard lock(frame_access_lock_);
    frames_.emplace_back(std::move(frame));
}

void Scene::add_frame_data(const Frame &data) {
    SpinGuard lock(frame_access_lock_);
    if (frames_.empty()) {
        throw std::runtime_error("called add_frame_data, but frames list is empty");
    }

    frames_.back()->update_from(data);
}

void Scene::add_permanent_frame_data(const Frame &data) {
    SpinGuard lock(frame_access_lock_);
    permanent_frame_.update_from(data.all_contexts());
}

void Scene::show_detailed_info(const glm::vec2 &mouse) const {
    if (!active_frame_) {
        return;
    }

    const auto &popups = active_frame_->all_popups();
    for (size_t idx = 0; idx < popups.size(); ++idx) {
        if (!conf_.enabled_layers[idx]) {
            continue;
        }
        for (const auto &popup : popups[idx]) {
            if (popup.hit_test(mouse)) {
                ImGui::BeginTooltip();
                ImGui::Text("%s", popup.text());
                ImGui::EndTooltip();
            }
        }
    }
}

void Scene::clear_data() {
    SpinGuard lock(frame_access_lock_);
    frames_.clear();
    active_frame_ = nullptr;
    permanent_frame_.clear();
    frames_count_ = 0;
    cur_frame_idx_ = 0;
}

bool Scene::has_data() const {
    return frames_count_ > 0;
}
