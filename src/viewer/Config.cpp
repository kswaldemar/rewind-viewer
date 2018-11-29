//
// Created by valdemar on 25.08.18.
//

#include "Config.h"

#include <common/logger.h>

namespace {

inline bool read_bool(const char *str) {
    int flag;
    sscanf(str, "%d", &flag);
    return static_cast<bool>(flag);
}

//Config entries
const char *UI_FAST_SKIP_SPEED = "ui.fast_skip_speed";
const char *UI_CLOSE_WITH_ESC = "ui.close_with_esc";
const char *UI_CLEAR_COLOR = "ui.clear_color";

const char *SCENE_GRID_CELLS_COUNT = "scene.grid_cells_count";
const char *SCENE_GRID_DIM = "scene.grid_dim";
const char *SCENE_GRID_COLOR = "scene.grid_color";
const char *SCENE_SCENE_COLOR = "scene.scene_color";
const char *SCENE_SHOW_GRID = "scene.show_grid";

const char *NET_USE_BINARY_PROTOCOL = "net.use_binary_protocol";

const char *CAMERA_ORIGIN_ON_TOP_LEFT = "camera.origin_on_top_left";
const char *CAMERA_START_POSITION = "camera.start_position";
const char *CAMERA_START_VIEWPORT_SIZE = "camera.start_viewport_size";

}


Config Config::load_from_file(const std::string &fname) {
    FILE *f = fopen(fname.c_str(), "r");
    if (!f) {
        LOG_INFO("Cannot open configuration file %s, use default values", fname.c_str());
        return {};
    }

    Config cfg;

    char buf[512];
    std::string line;

    Config::present_keys_t keys;

    while (fgets(buf, sizeof(buf), f)) {
        line = buf;
        if (line.back() == '\n') {
            line.pop_back();
        }

        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t eq_idx = line.find('=');
        if (eq_idx == std::string::npos) {
            LOG_WARN("Invalid config line (%s): '=' sign is missing", line.c_str());
            continue;
        }

        std::string value = line.substr(eq_idx + 2); //+2 to skip space
        std::string key = line.substr(0, eq_idx - 1);

        keys.insert(key);

        if (key == UI_FAST_SKIP_SPEED) {
            sscanf(value.c_str(), "%hu", &cfg.ui.fast_skip_speed);
        } else if (key == UI_CLOSE_WITH_ESC) {
            cfg.ui.close_with_esc = read_bool(value.c_str());
        } else if (key == UI_CLEAR_COLOR) {
            auto &v = cfg.ui.clear_color;
            sscanf(value.c_str(), "(%f, %f, %f)", &v.r, &v.g, &v.b);
        } else if (key == SCENE_GRID_CELLS_COUNT) {
            sscanf(value.c_str(), "(%hu, %hu)", &cfg.scene.grid_cells.x, &cfg.scene.grid_cells.y);
        } else if (key == SCENE_GRID_DIM) {
            auto &v = cfg.scene.grid_dim;
            sscanf(value.c_str(), "(%f, %f)", &v.x, &v.y);
        } else if (key == SCENE_GRID_COLOR) {
            auto &v = cfg.scene.grid_color;
            sscanf(value.c_str(), "(%f, %f, %f)", &v.r, &v.g, &v.b);
        } else if (key == SCENE_SCENE_COLOR) {
            auto &v = cfg.scene.scene_color;
            sscanf(value.c_str(), "(%f, %f, %f)", &v.r, &v.g, &v.b);
        } else if (key == SCENE_SHOW_GRID) {
            cfg.scene.show_grid = read_bool(value.c_str());
        } else if (key == NET_USE_BINARY_PROTOCOL) {
            cfg.net.use_binary_protocol = read_bool(value.c_str());
        } else if (key == CAMERA_ORIGIN_ON_TOP_LEFT) {
            cfg.camera.origin_on_top_left = read_bool(value.c_str());
        } else if (key == CAMERA_START_POSITION) {
            auto &v = cfg.camera.start_position;
            sscanf(value.c_str(), "(%f, %f)", &v.x, &v.y);
        } else if (key == CAMERA_START_VIEWPORT_SIZE) {
            sscanf(value.c_str(), "%f", &cfg.camera.start_viewport_size);
        } else {
            LOG_WARN("Invalid config line (%s): unknown key '%s'", line.c_str(), key.c_str());
        }
    }

    cfg.update_dynamic_values(keys);

    return cfg;
}

void Config::save_to_file(const std::string &fname) const {
    FILE *f = fopen(fname.c_str(), "w");
    if (!f) {
        LOG_ERROR("Cannot open file %s to write config", fname.c_str());
        return;
    }

    fputs("# How much frame to skip when fast forward arrow pressed\n", f);
    fprintf(f, "%s = %hu\n", UI_FAST_SKIP_SPEED, ui.fast_skip_speed);
    fputs("\n# If true, visualiser can be closed by Esc key\n", f);
    fprintf(f, "%s = %d\n", UI_CLOSE_WITH_ESC, ui.close_with_esc);
    fputs("\n# Background color, rgb format\n", f);
    fprintf(f, "%s = (%.3f, %.3f, %.3f)\n", UI_CLEAR_COLOR, ui.clear_color.r, ui.clear_color.g, ui.clear_color.b);

    fputs("\n# Grid cells count in each dimension (X, Y)\n", f);
    fprintf(f, "%s = (%hu, %hu)\n", SCENE_GRID_CELLS_COUNT, scene.grid_cells.x, scene.grid_cells.y);
    fputs("\n# Scene size\n", f);
    fprintf(f, "%s = (%.0f, %.0f)\n", SCENE_GRID_DIM, scene.grid_dim.x, scene.grid_dim.y);
    fputs("\n# Grid color, rgb format\n", f);
    fprintf(f, "%s = (%.3f, %.3f, %.3f)\n", SCENE_GRID_COLOR,
            scene.grid_color.r, scene.grid_color.g, scene.grid_color.b);
    fputs("\n# Scene background color, rgb format\n", f);
    fprintf(f, "%s = (%.3f, %.3f, %.3f)\n", SCENE_SCENE_COLOR,
            scene.scene_color.r, scene.scene_color.g, scene.scene_color.b);
    fputs("\n# If true, grid will be shown by default\n", f);
    fprintf(f, "%s = %d\n", SCENE_SHOW_GRID, scene.show_grid);

    fputs("\n# If true, binary protocol will be used instead of default json one\n", f);
    fprintf(f, "%s = %d\n", NET_USE_BINARY_PROTOCOL, net.use_binary_protocol);

    fputs("\n# If true, world origin will be in top left corner of screen, like in normal computer graphics\n", f);
    fprintf(f, "%s = %d\n", CAMERA_ORIGIN_ON_TOP_LEFT, camera.origin_on_top_left);
    fputs("\n# Initial settings for camera. In game camera movement won't rewrite that values\n", f);
    fprintf(f, "%s = (%.3f, %.3f)\n", CAMERA_START_POSITION, camera.start_position.x, camera.start_position.y);
    fprintf(f, "%s = %.3f\n", CAMERA_START_VIEWPORT_SIZE, camera.start_viewport_size);
}

void Config::update_dynamic_values(const Config::present_keys_t &keys) {
    if (keys.find("camera.start_position") == keys.end()) {
        camera.start_position = scene.grid_dim * 0.5f;
    }
    if (keys.find("camera.start_viewport_size") == keys.end()) {
        camera.start_viewport_size = std::max(scene.grid_dim.x, scene.grid_dim.y);
    }
}
