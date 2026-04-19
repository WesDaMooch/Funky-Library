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
    char mainSearchBuffer[64] = "";
    bool showSearchResultWindow = false;

    // Add new track window
    bool showAddTrackWindow = false;

    char inputArtistBuffer[64]{};
    char inputTrackBuffer[64]{};

    // Track view / edit
    bool showEditTrackWindow = false;
    bool showAddMixWindow = false;
    char mixSearchBuffer[64] = "";

    // Style
    static constexpr float ALPHA_HOVER = 0.1f;
    static constexpr float ALPHA_DEFAULT = 0.25f;

    // TODO: could use uint8,
    // and make static const
    std::array<int, 3> DEFAULT_COL_RGB = {200, 200, 200};

    // TODO: could pass const Track* track where nullptr means add track + mode for safetly,
    // void TrackInputWindow(TrackInputMode mode, Track* track = nullptr);
    enum TrackInputMode { 
        ADD, 
        EDIT 
    };
    void TrackInputWindow(TrackInputMode mode, Track* track = nullptr);

    //void TrackSearchBar??

    // TODO: add toLower and splitWords from manager
};