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
    LibraryManager::TrackSort mainSearchSort = LibraryManager::TrackSort::Artist;
    char mainSearchBuffer[64]{};
    bool showMainLibary = false;

    // Add new track window
    bool showAddTrackWindow = false;

    // Track view / edit
    bool showEditTrackWindow = false;
    bool showAddMixWindow = false;
    char mixSearchBuffer[64]{};

    // Remove track window
    bool showRemoveTrackWindow = false;


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
        float bpm = 0.f;
        int rating = 0;
        ImVec4 colour = Ui::Colour::VEC4_DEFAULT;

        LibraryManager::TrackValidationResult result =
            LibraryManager::TrackValidationResult::None;
    };

    InputData addData;
    InputData editData;

    void RemoveTrackWindow(int id);
    void InputTrackDataWindow(InputData& data, TrackInputMode mode);

    void TrackSearchTable2(const std::vector<Track>& ibrary, float x, float width);
    void TrackSearchTable(const std::vector<Track>& ibrary, float x, float width);

    void DrawActiveTrackDisplay(int id);
};