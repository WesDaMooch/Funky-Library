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
    bool open = true;

	LibraryManager manager;
    int activeTrackId = -1;
    std::string activeLabel{};
    std::string activeRelease{};

    SpringGraph springGraph;
    std::vector<SpringGraph::Node> trackNodes;
    std::vector<SpringGraph::Edge> trackEdges;
    std::unordered_map<int, int> idToIndex;

    // Main search bar and result window
    LibraryManager::TrackSort mainSearchSort = LibraryManager::TrackSort::Artist;
    char mainSearchBuffer[64]{};
    bool showMainLibary = false;

    // 
    bool showSpringDiagram = false;

    // Add new track window
    bool showAddTrackPopup = false;

    // Track view / edit
    bool showEditTrackPopup = false;
    bool showDeleteTrackPopup = false;
    bool showAddMixWindow = false;
    bool showDeleteMixPopup = false;

    char mixSearchBuffer[64]{};

    enum class MainPage
    {
        NONE,
        ACTIVE_TRACK,
        LABEL,
        RELEASE,
        TAG,
        MAP
    };
    MainPage mainPage = MainPage::NONE;

    // Input Data Window //
    enum class TrackInputMode 
    { 
        ADD, 
        EDIT 
    };
    struct InputData
    {
        int id = -1;
        char artist[64]{};
        char title[64]{};
        char label[64]{};
        char release[64]{};
        char position[64]{};
        float bpm = 0.f;
        int rating = 0;
        ImVec4 colour = Ui::Colour::VEC4_DEFAULT;

        LibraryManager::ValidationResult result =
            LibraryManager::ValidationResult::None;
    };
    InputData addTrackData;
    InputData editTrackData;
    void InputTrackDataPopup(InputData& data, TrackInputMode mode);

    // Delete Conformation Window //
    enum class DeleteConformationMode 
    {
        TRACK,
        MIX
    };
    struct DeleteData
    {
        int trackId = -1;
        int mixId = -1;
        std::string label{};
    };
    DeleteData deleteTrackData;
    DeleteData deleteMixData;
    void DeleteConformationPopup(DeleteData& data, DeleteConformationMode mode);

    void TrackSearchTable(const std::vector<Track>& ibrary, float x, float width);

    enum class TrackListMode
    {
        LABEL,
        RELEASE
    };
    void DisplayTrackList(std::vector<const Track*> trackList, TrackListMode mode);

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

    inline void SetModalPopupPosAndSize(const char* headerText)
    {
        float closeButtonSize = 70.f;
        float headerTextWidth = ImGui::CalcTextSize(headerText).x;
        float headerTextMinWidth = headerTextWidth + closeButtonSize;
        ImGui::SetNextWindowSizeConstraints(ImVec2(headerTextMinWidth, 0.0f), ImVec2(FLT_MAX, FLT_MAX));

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }
};