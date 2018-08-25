//
// Created by valdemar on 25.08.18.
//

#include "Config.h"

#include <common/logger.h>

namespace {

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
        if (key == "ui.fast_skip_speed") {
            sscanf(value.c_str(), "%hu", &cfg.ui.fast_skip_speed);
        } else if (key == "ui.close_with_esc") {
            int flag;
            sscanf(value.c_str(), "%d", &flag);
            cfg.ui.close_with_esc = static_cast<bool>(flag);
        } else if (key == "ui.clear_color") {
            auto &v = cfg.ui.clear_color;
            sscanf(value.c_str(), "(%f, %f, %f)", &v.r, &v.g, &v.b);
        } else if (key == "scene.grid_cells_count") {
            sscanf(value.c_str(), "%hu", &cfg.scene.grid_cells_count);
        } else if (key == "scene.grid_dim") {
            auto &v = cfg.scene.grid_dim;
            sscanf(value.c_str(), "(%f, %f)", &v.x, &v.y);
        } else if (key == "scene.grid_color") {
            auto &v = cfg.scene.grid_color;
            sscanf(value.c_str(), "(%f,%f,%f)", &v.r, &v.g, &v.b);
        } else if (key == "scene.scene_color") {
            auto &v = cfg.scene.scene_color;
            sscanf(value.c_str(), "(%f, %f, %f)", &v.r, &v.g, &v.b);
        } else if (key == "scene.show_grid") {
            int flag;
            sscanf(value.c_str(), "%d", &flag);
            cfg.scene.show_grid = static_cast<bool>(flag);
        } else {
            LOG_WARN("Invalid config line (%s): unknown key '%s'", line.c_str(), key.c_str());
        }
    }

    return cfg;
}

void Config::save_to_file(const std::string &fname) const {
    FILE *f = fopen(fname.c_str(), "w");
    if (!f) {
        LOG_ERROR("Cannot open file %s to write config", fname.c_str());
        return;
    }

    fputs("# How much frame to skip when fast forward arrow pressed\n", f);
    fprintf(f, "ui.fast_skip_speed = %hu\n", ui.fast_skip_speed);
    fputs("\n# If true, visualiser can be closed by Esc key\n", f);
    fprintf(f, "ui.close_with_esc = %d\n", ui.close_with_esc);
    fputs("\n# Background color, rgb format\n", f);
    fprintf(f, "ui.clear_color = (%.3f, %.3f, %.3f)\n", ui.clear_color.r, ui.clear_color.g, ui.clear_color.b);

    fputs("\n# Grid cells count\n", f);
    fprintf(f, "scene.grid_cells_count = %hu\n", scene.grid_cells_count);
    fputs("\n# Scene size\n", f);
    fprintf(f, "scene.grid_dim = (%.3f, %.3f)\n", scene.grid_dim.x, scene.grid_dim.y);
    fputs("\n# Grid color, rgb format\n", f);
    fprintf(f, "scene.grid_color = (%.3f, %.3f, %.3f)\n", scene.grid_color.r, scene.grid_color.g, scene.grid_color.b);
    fputs("\n# Scene background color, rgb format\n", f);
    fprintf(f, "scene.scene_color = (%.3f, %.3f, %.3f)\n", scene.scene_color.r, scene.scene_color.g, scene.scene_color.b);
    fputs("\n# If true, grid will be shown by default\n", f);
    fprintf(f, "scene.show_grid = %d\n", scene.show_grid);
}
