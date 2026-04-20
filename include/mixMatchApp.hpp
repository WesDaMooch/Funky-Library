#pragma once

#include "imgui.h"
#include "UiConstants.hpp"
#include "catalogueManager.hpp"

#include <array>

struct MixMatchApp
{
public:
    MixMatchApp();
    void RunFrame();

private:
	CatalogueManager manager;
    int activeTrackId = -1;

    bool open = true;

    // Main search bar and result window
    char mainSearchBuffer[64]{};
    bool showSearchResultWindow = false;

    // Add new track window
    bool showAddTrackWindow = false;

    // Track view / edit
    bool showEditTrackWindow = false;
    bool showAddMixWindow = false;
    char mixSearchBuffer[64]{};


    // TODO: could use uint8,
    // and make static const

    // Input track window
    enum TrackInputMode { 
        ADD, 
        EDIT 
    };

    struct InputData
    {
        int id = -1;
        char artist[64]{};
        char title[64]{};
        char label[64]{};
        int bpm = 0;
        ImVec4 colour = UI::COL_DEFAULT;
    };

    InputData addData;
    InputData editData;

    void TrackInputWindow(InputData& data, TrackInputMode mode);

    //void TrackSearchBar??

    // TODO: add toLower and splitWords from manager

    // TODO: clamp input
    inline ImVec4 RgbToImVec4(const std::array<uint8_t, 3>& c)
    {
        return ImVec4(
            c[0] / 255.0f,
            c[1] / 255.0f,
            c[2] / 255.0f,
            1.0f
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
};