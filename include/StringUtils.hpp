#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cctype>

//TODO: Rename hpp to Utils

namespace TextUtil
{
    inline std::string ToLower(std::string s)
    {
        std::transform(
            s.begin(),
            s.end(),
            s.begin(),
            [](unsigned char c) {return std::tolower(c); }
        );
        return s;
    }

    inline std::string Trim(const std::string& s)
    {
        auto start = std::find_if_not(s.begin(), s.end(),
            [](unsigned char ch) { return std::isspace(ch); });

        auto end = std::find_if_not(s.rbegin(), s.rend(),
            [](unsigned char ch) { return std::isspace(ch); }).base();

        if (start >= end)
            return "";

        return std::string(start, end);
    }

    // TODO: reserve some vector space, most titles and names are a few words long...
    inline std::vector<std::string> Split(const std::string& s)
    {
        std::stringstream ss(s);
        std::vector<std::string> words;
        words.reserve(6);

        std::string word;

        while (ss >> word)
            words.emplace_back(word);

        return words;
    }

    // Call after TableSetColumnIndex & before Text
    inline void DrawCenterJustifyTableText(const std::string& text)
    {
        float columnWidth = ImGui::GetColumnWidth();
        float textWidth = ImGui::CalcTextSize(text.c_str()).x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (columnWidth - textWidth) * 0.5f);
        ImGui::TextUnformatted(text.c_str());
    }
}

namespace ColourUtil
{
    // TODO: clamp input
    inline ImU32 RgbToU32(const std::array<uint8_t, 3>& c, uint8_t alpha = 255)
    {
        return IM_COL32(c[0], c[1], c[2], alpha);
    }

    inline ImVec4 RgbToImVec4(const std::array<uint8_t, 3>& c, float alpha = 1.f)
    {
        return ImVec4(
            c[0] / 255.0f,
            c[1] / 255.0f,
            c[2] / 255.0f,
            alpha
        );
    }

    inline std::array<uint8_t, 3> ImVec4ToRgb(const ImVec4& c)
    {
        return {
            (uint8_t)(c.x * 255.f),
            (uint8_t)(c.y * 255.f),
            (uint8_t)(c.z * 255.f)
        };
    }

    // Use when alpha must be 1.
    inline ImVec4 AlphaVec4(const ImVec4& c, float alpha)
    {
        alpha = (std::max)(0.f, (std::min)(alpha, 1.f));
        return ImVec4(c.x, c.y, c.z, alpha);
    }

    // Use when alpha must be 1.
    inline ImVec4 TintVec4(const ImVec4& c, float tint)
    {
        tint = (std::max)(0.f, (std::min)(tint, 1.f));

        return ImVec4(
            c.x * tint,
            c.y * tint,
            c.z * tint,
            1.f
        );
    }
}

namespace TrackUtil
{
    inline std::string FormartTitle(const std::string& artist, const std::string& title)
    {
        if (artist.empty() || title.empty())
            return "";

        return artist + " - " + title;
    }

}