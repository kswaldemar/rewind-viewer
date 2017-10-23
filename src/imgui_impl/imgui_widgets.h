//
// Created by valdemar on 23.10.17.
//

#pragma once

#include <imgui.h>

namespace ImGui {

// size_arg (for each axis) < 0.0f: align to end, 0.0f: auto, > 0.0f: specified size
bool TickBar(const char *label, float *v, float v_min, float v_max, const ImVec2 &size_arg);

}