#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include "UiConstants.hpp"
#include "StringUtils.hpp"
#include "catalogueManager.hpp"

#include <array>

struct MixMatchApp
{
public:
    MixMatchApp();
    void RunFrame();

private:
	LibraryManager manager;
    int activeTrackId = -1;

    bool open = true;

    // Main search bar and result window
    char mainSearchBuffer[64]{};
    bool showMainLibary = false;
    bool showSearchResultWindow = false; //TODO: Remove

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
        float bpm = 0;
        ImVec4 colour = Ui::COL_DEFAULT;
    };

    InputData addData;
    InputData editData;

    void TrackInputWindow(InputData& data, TrackInputMode mode);
    void TrackCardTable(const std::vector<Track>& searchLibrary);
    void TrackInfoCard(int id);

    //void TrackSearchBar??
};