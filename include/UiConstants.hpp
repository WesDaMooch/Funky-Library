
//#define ICON_ADD        u8"\uE145"
//#define ICON_REMOVE     u8"\uE5CD"
//#define ICON_REFRESH    u8"\uE627"
//#define ICON_SAVE		u8"\uE161"
//#define ICON_DELETE     u8"\uE872"
//#define ICON_SEARCH     u8"\uE8B6"
//#define ICON_EDIT       u8"\uE3C9"

#pragma once
#include "imgui.h"
#include <array>

namespace UI
{
    static constexpr std::array<uint8_t, 3> RGB_DEFAULT = { 200, 200, 200 };
    static constexpr ImVec4 COL_DEFAULT = ImVec4(200 / 255.f, 200 / 255.f, 200 / 255.f, 1.f);

    static constexpr float ALPHA_DEFAULT = 0.25f;
    static constexpr float ALPHA_HOVER = 0.5f;

    static constexpr const char* ICON_ADD       = u8"\uE145";
    static constexpr const char* ICON_REMOVE    = u8"\uE5CD";
    static constexpr const char* ICON_REFRESH   = u8"\uE627";
    static constexpr const char* ICON_SAVE      = u8"\uE161";
    static constexpr const char* ICON_DELETE    = u8"\uE872";
    static constexpr const char* ICON_SEARCH    = u8"\uE8B6";
    static constexpr const char* ICON_EDIT      = u8"\uE3C9";
}