#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include "UiConstants.hpp"
#include "StringUtils.hpp"
#include "catalogueManager.hpp"
#include "springGraph.hpp"

#include <array>

struct MixMatchApp
{
public:
    MixMatchApp();
    void RunFrame();

private:
	LibraryManager manager;
    int activeTrackId = -1;

    SpringGraph springGraph;
    std::vector<SpringGraph::Node> trackNodes;
    std::vector<SpringGraph::Edge> trackEdges;
    std::unordered_map<int, int> idToIndex;

    bool open = true;

    // Main search bar and result window
    LibraryManager::TrackSort mainSearchSort = LibraryManager::TrackSort::Artist;
    char mainSearchBuffer[64]{};
    bool showMainLibary = false;

    // 
    bool showSpringDiagram = false;

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

        LibraryManager::ValidationResult result =
            LibraryManager::ValidationResult::None;
    };

    InputData addData;
    InputData editData;




    void InputTrackDataWindow(InputData& data, TrackInputMode mode);
    void RemoveTrackWindow(int id); // TODO pass track

    void TrackSearchTable2(const std::vector<Track>& ibrary, float x, float width);
    void TrackSearchTable(const std::vector<Track>& ibrary, float x, float width);

    void DrawActiveTrackDisplay(int id);

    inline void DrawStarRating(int rating)
    {
        for (int i = 0; i < 5; i++)
        {
            if (i < rating)
            {
                ImGui::Text(Ui::Text::ICON_STAR);
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(80, 80, 80, 255));
                ImGui::Text(Ui::Text::ICON_STAR);
                ImGui::PopStyleColor();
            }

            if (i < 4)
                ImGui::SameLine();
        }
    }
};