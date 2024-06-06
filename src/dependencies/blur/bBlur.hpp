#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "blur.hpp"

#include <map>

#define IM_USE using namespace ImGui

class c_gui
{
public:
    // Blur render
    auto blur_background(ImDrawList* drawList, IDirect3DDevice9* device, int opacity = 255 ) -> void {
        return DrawBackgroundBlur(drawList, device, opacity);
    }
};

inline auto bBlur = new c_gui;
