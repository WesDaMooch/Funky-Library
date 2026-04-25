#pragma once
#include "imgui.h"
#include <array>

//TODO: Rename to hpp to Constants ?

namespace Ui
{
    // Text & Font
    static constexpr float GLYPH_OFFSET = 4.f;

    extern ImFont* Small;
    extern ImFont* Main;

    static constexpr const char* ICON_ADD       = u8"\uE145";
    static constexpr const char* ICON_REMOVE    = u8"\uE5CD";
    static constexpr const char* ICON_REFRESH   = u8"\uE627";
    static constexpr const char* ICON_SAVE      = u8"\uE161";
    static constexpr const char* ICON_DELETE    = u8"\uE872";
    static constexpr const char* ICON_SEARCH    = u8"\uE8B6";
    static constexpr const char* ICON_EDIT      = u8"\uE3C9";

    // Colour
    static constexpr float ALPHA_BG = 0.15f;
    static constexpr float ALPHA_HOVER = 0.5f;
    
    static constexpr float TINT_BG = 0.15f;
    static constexpr float TINT_HOVER = 0.33f;
    static constexpr float TINT_ACTIVE = 0.75f;

    static constexpr std::array<uint8_t, 3> RGB_DEFAULT = { 75, 75, 75 };
    static constexpr ImVec4 VEC4_DEFAULT = ImVec4(75 / 255.f, 75 / 255.f, 75 / 255.f, 1.f);
}