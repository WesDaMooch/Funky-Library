#pragma once
#include "imgui.h"
#include <array>

//TODO: Rename to hpp to Constants ?

namespace Ui
{
    namespace Text
    {
        inline ImFont* DefaultFont = nullptr;
        inline ImFont* SmallFont = nullptr;

        constexpr float GLYPH_OFFSET = 5.f; // TODO: Remove not used?

        constexpr const char* ICON_ADD = u8"\uE145";
        constexpr const char* ICON_CLOSE = u8"\uE5CD";
        constexpr const char* ICON_REMOVE = u8"\uE15B"; 
        constexpr const char* ICON_REFRESH = u8"\uE627";
        constexpr const char* ICON_SAVE = u8"\uE161";
        constexpr const char* ICON_DELETE = u8"\uE872";
        constexpr const char* ICON_SEARCH = u8"\uE8B6";
        constexpr const char* ICON_EDIT = u8"\uE3C9";
        constexpr const char* ICON_STAR = u8"\uE838";
        constexpr const char* ICON_HALFSTAR = u8"\uE839"; 
        constexpr const char* ICON_STARBORDER = u8"\uE83A";
        constexpr const char* ICON_MINIMIZE = u8"\uE931";
        constexpr const char* ICON_ARROWBACK = u8"\uE5C4"; 
        constexpr const char* ICON_ARROWFORWARD = u8"\uE5C8";
        constexpr const char* ICON_SYNCALT = u8"\uEA18"; 
    }

    namespace Colour
    {
        constexpr float ALPHA_BG = 0.15f;
        constexpr float ALPHA_HOVER = 0.5f;
        constexpr float ALPHA_ACTIVE = 0.75f;

        constexpr float TINT_BG = 0.15f;
        constexpr float TINT_HOVER = 0.33f;
        constexpr float TINT_ACTIVE = 0.75f;

        constexpr std::array<uint8_t, 3> RGB_DEFAULT = { 75, 75, 75 };
        constexpr ImVec4 VEC4_DEFAULT = ImVec4(75 / 255.f, 75 / 255.f, 75 / 255.f, 1.f);

        constexpr ImVec4 TEXT_WHITE = ImVec4(1.f, 1.f, 1.f, 1.f);
        constexpr ImVec4 TEXT_GRAY = ImVec4(130 / 255.f, 130 / 255.f, 130 / 255.f, 1.f);
    }
}