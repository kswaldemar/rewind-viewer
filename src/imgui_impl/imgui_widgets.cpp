//
// Created by valdemar on 23.10.17.
//

#include "imgui_widgets.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui_internal.h>

namespace ImGui {

bool TickBar(const char *label, float *v, float v_min, float v_max, const ImVec2 &size_arg) {
    ImGuiWindow *window = GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;

    ImVec2 pos = window->DC.CursorPos;
    ImRect bb(pos, pos + CalcItemSize(size_arg, CalcItemWidth(), g.FontSize + style.FramePadding.y * 2.0f));

    const ImGuiID id = window->GetID(label);
    if (!ItemAdd(bb, &id)) {
        ItemSize(bb, style.FramePadding.y);
        return false;
    }

    const bool hovered = IsHovered(bb, id);
    if (hovered) {
        SetHoveredID(id);
    }

    // Tabbing or CTRL-clicking on Slider turns it into an input box
    bool start_text_input = false;
    const bool tab_focus_requested = FocusableItemRegister(window, g.ActiveId == id);
    if (tab_focus_requested || (hovered && g.IO.MouseClicked[0])) {
        SetActiveID(id, window);
        FocusWindow(window);

        if (tab_focus_requested || g.IO.KeyCtrl) {
            start_text_input = true;
            g.ScalarAsInputTextId = 0;
        }
    }
    if (start_text_input || (g.ActiveId == id && g.ScalarAsInputTextId == id)) {
        return InputScalarAsWidgetReplacement(bb, label, ImGuiDataType_Float, v, id, 0);
    }

    ItemSize(bb, style.FramePadding.y);

    // Render
    const auto fraction = ImSaturate(*v / (v_max - v_min));
    RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
    bb.Expand(ImVec2(-window->BorderSize, -window->BorderSize));
    const ImVec2 fill_br = ImVec2(ImLerp(bb.Min.x, bb.Max.x, fraction), bb.Max.y);
    RenderFrame(bb.Min, fill_br, GetColorU32(ImGuiCol_PlotHistogram), false, style.FrameRounding);

    // Default displaying the fraction as percentage string, but user can override it
    char overlay_buf[32];
    ImFormatString(overlay_buf, IM_ARRAYSIZE(overlay_buf), "%d/%d", static_cast<int>(*v), static_cast<int>(v_max));
    const char *overlay = overlay_buf;

    ImVec2 overlay_size = CalcTextSize(overlay, NULL);
    if (overlay_size.x > 0.0f) {
        RenderTextClipped(ImVec2(ImClamp(fill_br.x + style.ItemSpacing.x,
                                         bb.Min.x,
                                         bb.Max.x - overlay_size.x - style.ItemInnerSpacing.x), bb.Min.y),
                          bb.Max,
                          overlay,
                          NULL,
                          &overlay_size,
                          ImVec2(0.0f, 0.5f),
                          &bb);
    }

    if (g.ActiveId == id) {
        ClearActiveID();
    }

    return false;
}

void ShowHelpMarker(const char *desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(450.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

}