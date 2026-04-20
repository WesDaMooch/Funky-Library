#pragma once

#include "imgui.h"
#include "icons.hpp"
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

    // Style
    static constexpr ImVec4 COL_DEFAULT = ImVec4(200 / 255.f, 200 / 255.f, 200 / 255.f, 1.f);
    static constexpr float ALPHA_DEFAULT = 0.25f;
    static constexpr float ALPHA_HOVER = 0.1f;


    // TODO: could use uint8,
    // and make static const
    std::array<int, 3> COL_DEFAULT_RGB = {200, 200, 200}; // TODO: remove



    // Input track window
    enum TrackInputMode { 
        ADD, 
        EDIT 
    };

    struct InputData
    {
        char artist[64]{};
        char title[64]{};
        ImVec4 colour = COL_DEFAULT;
    };

    InputData addData;
    InputData editData;

    void TrackInputWindow(InputData& data, TrackInputMode mode, const Track* track = nullptr);

    //void TrackSearchBar??

    // TODO: add toLower and splitWords from manager
};