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

    // Table drawing
    const float tablePaddingX = 6;
    const float tablePaddingY = 16;

    const float bgRectGap = 3;
    const float bgPaddingY = tablePaddingY - bgRectGap;

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
    void DisplayMixList(const std::vector<Mix>& mixList);


    inline void StarRating(const Track* track)
    {
        if (track == nullptr)
            return;

        int hoverIndex = -1;
        ImVec2 starSize = ImGui::CalcTextSize(Ui::Text::ICON_STAR);
        ImVec2 startPos = ImGui::GetCursorPos();
        for (int i = 1; i <= 5; i++)
        {
            ImGui::PushID(i);

            if (ImGui::InvisibleButton("##Star Button", starSize))
            {
                Track editedTrack = *track;
                editedTrack.rating = i;
                manager.editTrack(editedTrack);
            }
         
            if (ImGui::IsItemHovered())
                hoverIndex = i;
            
            if (i <= 4)
                ImGui::SameLine(0.f, 0.f);

            ImGui::PopID();
        }

        ImGui::SetCursorPos(startPos);
        for (int i = 0; i < 5; i++)
        {
            if ((hoverIndex != -1 && hoverIndex > i) || (hoverIndex == -1 && track->rating > i))
            {
                ImGui::Text(Ui::Text::ICON_STAR);
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ColourUtil::RgbToU32(Ui::Colour::RGB_DEFAULT, 128));
                ImGui::Text(Ui::Text::ICON_STAR);
                ImGui::PopStyleColor();
            }

            if (i < 4)
                ImGui::SameLine(0.f, 0.f);
        }
    }

    // Requires PopStyleColor(3)
    inline void SetClearSelectableStyle()
    {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
    }

    // Requires PopStyleColor(1)
    inline void SetTextColourOnHover(bool hover)
    {
        if (hover)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
        else
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.66f, 0.66f, 0.66f, 1.f));
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

    // Table helpers
    inline bool DrawTableBg(ImDrawList* drawList, 
        float width, float height, 
        std::array<uint8_t, 3> colour,
        float bpm = 0)
    {
        const ImDrawFlags corners = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersBottomRight;

        ImVec2 rowPos = ImGui::GetCursorScreenPos();

        ImVec2 topLeft(rowPos.x, rowPos.y - bgPaddingY);
        ImVec2 bottomRight(rowPos.x + width, rowPos.y + height + bgPaddingY);

        ImVec2 mousePos = ImGui::GetMousePos();
        bool rectHovered = mousePos.x >= topLeft.x && mousePos.x <= bottomRight.x &&
            mousePos.y >= topLeft.y && mousePos.y <= bottomRight.y;

        //bool rectClicked = rectHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

        // TODO: make the colour darker then have a wider range of alpha to oscillate
        uint8_t colourAlpha = 40;
        if (rectHovered)
        {
            bpm = 0;
            if (bpm > 0.f)
            {
                float freq = bpm / 240.f;
                float minB = 32;
                float maxB = 42;

                float t = ImGui::GetTime();

                colourAlpha = (int)(minB + (maxB - minB) * 0.5f * (1.f + sinf(2.f * 3.14159f * freq * t)));
            }
            else
            {
                colourAlpha = 64;
            }

        }

        drawList->AddRectFilled(topLeft, bottomRight,
            ColourUtil::RgbToU32(colour, colourAlpha),
            10.f, corners);

        drawList->AddRect(topLeft, bottomRight,
            ColourUtil::RgbToU32(colour, colourAlpha + 24),
            10.f, corners, 1.f);

        return rectHovered;
    }

    inline void DrawTableDividerLine(ImDrawList* drawList, ImU32 colour,
        float height, float offsetX = 0)
    {
        // TODO: use table and the same pos as the inner borderV
        ImVec2 rowPos = ImGui::GetCursorScreenPos();
        float posX = rowPos.x - offsetX;
        float length = height + bgPaddingY - 2;

        drawList->AddLine(ImVec2(posX, rowPos.y), ImVec2(posX, rowPos.y - length), colour);
    }
};