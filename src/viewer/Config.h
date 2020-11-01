//
// Created by valdemar on 25.08.18.
//

#pragma once

#include <viewer/Frame.h>

#include <glm/glm.hpp>

#include <unordered_set>
#include <cstdint>

class Config {
public:
    struct UIConf {
        uint16_t fast_skip_speed = 20;
        bool close_with_esc = false;
        glm::vec3 clear_color = {0.2, 0.3, 0.3};
    } ui;

    struct SceneConf {
        glm::u16vec2 grid_cells = {32, 32};
        glm::vec2 grid_dim = {1024.0f, 1024.0f};
        glm::vec4 grid_color = {0.3219f, 0.336f, 0.392f, 1.0f};
        glm::vec4 scene_color = {0.757f, 0.856f, 0.882f, 1.0f};
        bool show_grid = true;
        std::array<bool, Frame::LAYERS_COUNT> enabled_layers = {{true, true, true, true, true}};
    } scene;

    struct NetConf {
        bool use_binary_protocol = false;
    } net;

    struct CameraConf {
        bool origin_on_top_left = true;
        glm::vec2 start_position;
        float start_viewport_size = 10.0;
    } camera;

    static std::unique_ptr<Config> init_with_imgui(const std::string &fname);

private:
    using present_keys_t = std::unordered_set<std::string>;

    ///Update config values which depends from other fields
    //void update_dynamic_values(const present_keys_t &);
};