//
// Created by valdemar on 25.08.18.
//

#include "Config.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <memory>

namespace {

// Config entries

void *callback_ReadOpen(ImGuiContext *, ImGuiSettingsHandler *handle, const char *) {
    assert(handle->UserData);
    return handle->UserData;
}

void callback_ReadLine(ImGuiContext *, ImGuiSettingsHandler *, void *entry, const char *line) {
    Config &cfg = *static_cast<Config *>(entry);
    glm::vec4 v;
    v.a = 1.0f;
    glm::vec2 p;
    int d1, d2;

    if (sscanf(line, "ui.fast_skip_speed=%d", &d1) == 1) {
        cfg.ui.fast_skip_speed = d1;
    } else if (sscanf(line, "ui.close_with_esc=%d", &d1) == 1) {
        cfg.ui.close_with_esc = d1;
    } else if (sscanf(line, "ui.clear_color=(%f,%f,%f)", &v.r, &v.g, &v.b) == 3) {
        cfg.ui.clear_color = v;
    } else if (sscanf(line, "ui.update_unfocused=%d", &d1) == 1) {
        cfg.ui.update_unfocused = d1;
    } else if (sscanf(line, "ui.imgui_theme_id=%d", &d1) == 1) {
        cfg.ui.imgui_theme_id = cg::clamp(d1, 0, 2);
    } else if (sscanf(line, "scene.grid_cells_count=(%d,%d)", &d1, &d2) == 2) {
        cfg.scene.grid_cells = {d1, d2};
    } else if (sscanf(line, "scene.grid_dim=(%f,%f)", &p.x, &p.y) == 2) {
        cfg.scene.grid_dim = p;
    } else if (sscanf(line, "scene.grid_color=(%f,%f,%f)", &v.r, &v.g, &v.b) == 3) {
        cfg.scene.grid_color = v;
    } else if (sscanf(line, "scene.scene_color=(%f,%f,%f)", &v.r, &v.g, &v.b) == 3) {
        cfg.scene.scene_color = v;
    } else if (sscanf(line, "scene.show_grid=%d", &d1) == 1) {
        cfg.scene.show_grid = d1;
    } else if (sscanf(line, "net.use_binary_protocol=%d", &d1) == 1) {
        cfg.net.use_binary_protocol = d1;
    } else if (sscanf(line, "camera.origin_on_top_left=%d", &d1) == 1) {
        cfg.camera.origin_on_top_left = d1;
    } else if (sscanf(line, "camera.start_position=(%f,%f)", &p.x, &p.y) == 2) {
        cfg.camera.start_position = p;
    } else if (sscanf(line, "camera.start_viewport_size=%f", &v.x) == 1) {
        cfg.camera.start_viewport_size = v.x;
    }
}

void write(ImGuiTextBuffer &to, const char *name, glm::vec3 color) {
    to.appendf("%s=(%.3f,%.3f,%.3f)\n", name, color.r, color.g, color.b);
}

void write(ImGuiTextBuffer &to, const char *name, int value) {
    to.appendf("%s=%d\n", name, value);
}

void write(ImGuiTextBuffer &to, const char *name, float value) {
    to.appendf("%s=%.3f\n", name, value);
}

void write(ImGuiTextBuffer &to, const char *name, uint16_t value) {
    to.appendf("%s=%hu\n", name, value);
}

void write(ImGuiTextBuffer &to, const char *name, glm::vec2 pos) {
    to.appendf("%s=(%.3f,%.3f)\n", name, pos.x, pos.y);
}

void write(ImGuiTextBuffer &to, const char *name, glm::u16vec2 pos) {
    to.appendf("%s=(%hu,%hu)\n", name, pos.x, pos.y);
}

template <typename T>
void write(ImGuiTextBuffer &to, const char *name, T value, const char *desc) {
    if (desc) {
        to.appendf(";%s\n", desc);
    }
    write(to, name, std::move(value));
}

void callback_WriteAll(ImGuiContext *, ImGuiSettingsHandler *handler, ImGuiTextBuffer *buf) {
    assert(handler->UserData);
    Config &cfg = *static_cast<Config *>(handler->UserData);

    buf->appendf("[%s][%s]\n", handler->TypeName, "Parameters configuration");

#define P(param) #param, param
    const auto &ui = cfg.ui;
    write(*buf, P(ui.fast_skip_speed), "How much frame to skip when fast forward arrow pressed");
    write(*buf, P(ui.close_with_esc), "If true, visualiser can be closed by Esc key");
    write(*buf, P(ui.clear_color), "Background color, rgb format");
    write(*buf, P(ui.update_unfocused), "Update when not in focus, consumes extra CPU");
    write(*buf, P(ui.imgui_theme_id));

    const auto &scene = cfg.scene;
    write(*buf, P(scene.grid_cells), "Grid cells count in each dimension (X, Y)");
    write(*buf, P(scene.grid_dim), "Scene size");
    write(*buf, "scene.grid_color", glm::vec3{cfg.scene.grid_color}, "Grid color, rgb format");
    write(*buf, "scene.scene_color", glm::vec3{cfg.scene.scene_color},
          "Scene background color, rgb format");
    write(*buf, P(scene.show_grid), "If true, grid will be shown by default");

    // const auto &net = cfg.net;
    // write(*buf, P(net.use_binary_protocol),
    //      "If true, binary protocol will be used instead of default json one");

    const auto &camera = cfg.camera;
    write(*buf, P(camera.origin_on_top_left),
          "If true, world origin will be in top left corner of screen, like in normal computer "
          "graphics");
    write(*buf, P(camera.start_position),
          "Initial settings for camera. In game camera movement won't rewrite that values");
    write(*buf, P(camera.start_viewport_size));
#undef P

    buf->append("\n");
}

}  // namespace

std::unique_ptr<Config> Config::init_with_imgui(const char* fname) {
    auto cfg = std::make_unique<Config>();

    ImGuiContext *context = ImGui::GetCurrentContext();
    if (!context) {
        context = ImGui::CreateContext();
    }

    auto &io = ImGui::GetIO();
    io.IniFilename = fname;

    // Create viewer configuration section
    ImGuiSettingsHandler ini_handler;
    ini_handler.TypeName = "Rewindviewer";
    ini_handler.TypeHash = ImHashStr(ini_handler.TypeName);
    ini_handler.ReadOpenFn = callback_ReadOpen;
    ini_handler.ReadLineFn = callback_ReadLine;
    ini_handler.WriteAllFn = callback_WriteAll;
    ini_handler.UserData = cfg.get();
    context->SettingsHandlers.push_back(ini_handler);

    ImGui::LoadIniSettingsFromDisk(io.IniFilename);

    return cfg;
}
