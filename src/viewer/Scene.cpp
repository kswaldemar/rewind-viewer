#include "Scene.h"
#include "Renderer.h"

#include <cgutils/utils.h>

#include <imgui.h>

#include <unordered_map>

Scene::Scene(ResourceManager *res, const Config::SceneConf *conf) : conf_(*conf) {
    renderer_ = std::make_unique<Renderer>(res, conf_.grid_dim, conf_.grid_cells);
}

Scene::~Scene() = default;

void Scene::update_and_render(const Camera &cam) {
    // Update current frame
    {
        std::lock_guard<std::mutex> f(frames_mutex_);
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

    // Draw permanent frame
    {
        while (lock_permanent_frame_.test_and_set(std::memory_order_acquire))
            ;
        const auto &contexts = permanent_frame_.all_contexts();
        for (size_t idx = 0; idx < contexts.size(); ++idx) {
            if (conf_.enabled_layers[idx]) {
                renderer_->render_primitives(contexts[idx]);
            }
        }
        lock_permanent_frame_.clear(std::memory_order_release);
    }

    // Draw currently selected frame
    if (active_frame_) {
        const auto &contexts = active_frame_->all_contexts();
        for (size_t idx = 0; idx < contexts.size(); ++idx) {
            if (conf_.enabled_layers[idx]) {
                renderer_->render_primitives(contexts[idx]);
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
    std::lock_guard<std::mutex> f(frames_mutex_);
    frames_.emplace_back(std::move(frame));
}

void Scene::add_permanent_frame_data(const Frame &data) {
    while (lock_permanent_frame_.test_and_set(std::memory_order_acquire))
        ;
    permanent_frame_.update_from(data.all_contexts());
    lock_permanent_frame_.clear(std::memory_order_release);
}

void Scene::show_detailed_info(const glm::vec2 &mouse) const {
    if (!active_frame_) {
        return;
    }

    for (const auto &popup : active_frame_->popups()) {
        if (popup->hit_test(mouse)) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", popup->text());
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
