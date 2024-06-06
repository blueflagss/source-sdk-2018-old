#pragma once
#include<string>
#include<sstream>
#include<vector>
#include<math.h>

#include <d3dx9.h>
#include <d3d9.h>
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

extern void DrawBackgroundBlur( ImDrawList *drawList, IDirect3DDevice9 *device, int opacity );