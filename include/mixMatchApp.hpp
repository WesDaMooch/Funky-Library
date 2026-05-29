#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include "UiConstants.hpp"
#include "StringUtils.hpp"
#include "catalogueManager.hpp"
#include "springGraph.hpp"

#include <array>
#include <string> // needed?
#include <algorithm> // needed? 



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
    const float minTableColumnWidth = 120;
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

        LibraryManager::ValidationResult result = LibraryManager::ValidationResult::None;
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

    void DisplayLabelTable(std::vector<const Track*> trackList);
    void DisplayReleaseTable(std::vector<const Track*> trackList);
    void DisplayMixTable(const std::vector<Mix>& mixList);

    inline int StarRating(const int rating)
    {
        int newRating = -1;
        int hoverIndex = -1;

        ImVec2 starSize = ImGui::CalcTextSize(Ui::Text::ICON_STAR);
        ImVec2 startPos = ImGui::GetCursorPos();

        for (int i = 1; i <= 5; i++)
        {
            ImGui::PushID(i);

            if (ImGui::InvisibleButton("##Star Button", starSize))
                newRating = i;
         
            if (ImGui::IsItemHovered())
                hoverIndex = i;
            
            if (i <= 4)
                ImGui::SameLine(0.f, 0.f);

            ImGui::PopID();
        }

        ImGui::SetCursorPos(startPos);
        for (int i = 0; i < 5; i++)
        {
            if ((hoverIndex != -1 && hoverIndex > i) || (hoverIndex == -1 && rating > i))
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

        return newRating;
    }

    inline void SetClearSelectableStyle()
    {
        // IMPORTANT: Requires PopStyleColor(3)
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

    // Input data helpers
    inline void InputDataSaveButton(InputData& data, TrackInputMode mode)
    {
        if (ImGui::Button(Ui::Text::ICON_SAVE))
        {
            Track t;
            t.artist = data.artist;
            t.title = data.title;
            t.label = data.label;
            t.release = data.release;
            t.position = data.position;
            t.bpm = data.bpm;
            t.rating = data.rating;
            t.colour = ColourUtil::Vec4ToRgb(data.colour);

            if (mode == TrackInputMode::ADD)
            {
                data.result = manager.addTrack(t);

                if (data.result == LibraryManager::ValidationResult::ValidTrack)
                {
                    if (!manager.getCatalogueForDisplay().empty())
                    {
                        activeTrackId = manager.getCatalogueForDisplay().back().id;
                        mainPage = MainPage::ACTIVE_TRACK;
                    }
                }
            }
            else if (mode == TrackInputMode::EDIT)
            {
                t.id = activeTrackId;
                data.result = manager.editTrack(t);
            }

            if (data.result == LibraryManager::ValidationResult::ValidTrack)
            {
                data = {};
                ImGui::CloseCurrentPopup();
            }
        }

        if (data.result != LibraryManager::ValidationResult::None || LibraryManager::ValidationResult::ValidTrack)
        {
            ImGui::SameLine();

            std::string waringText = "";

            switch (data.result)
            {
            case LibraryManager::ValidationResult::MissingArtist:
                waringText = "Missing Artist Entry";
                break;
            case LibraryManager::ValidationResult::MissingTitle:
                waringText = "Missing Title Entry";
                break;
            case LibraryManager::ValidationResult::TrackNotFound:
                waringText = "Track Not In Library";
                break;
            case LibraryManager::ValidationResult::DuplicateTrack:
                waringText = "Track Already In Library";
                break;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, Ui::Colour::ALPHA_ACTIVE));
            ImGui::Text(waringText.c_str());
            ImGui::PopStyleColor();
        }
    }


    // Table helpers
    inline bool DrawTableBg(ImDrawList* drawList, 
        float width, float height, 
        std::array<uint8_t, 3> colour,
        float bpm = 0)
    {
        /* // B&W look
        ImVec2 rowPos = ImGui::GetCursorScreenPos();

        ImVec2 topLeft(rowPos.x, rowPos.y - bgPaddingY);
        ImVec2 bottomRight(rowPos.x + width, rowPos.y + height + bgPaddingY);

        ImVec2 mousePos = ImGui::GetMousePos();
        bool rectHovered = mousePos.x >= topLeft.x && mousePos.x <= bottomRight.x &&
            mousePos.y >= topLeft.y && mousePos.y <= bottomRight.y;

        drawList->AddRectFilled(topLeft, bottomRight,
            IM_COL32(18, 18, 18, 255),
            0.f);

        drawList->AddRect(topLeft, bottomRight,
            IM_COL32_WHITE,
            0.f, 0, 1.f);

        return rectHovered;
        */

        /* // Full colour
        const ImDrawFlags corners = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersBottomRight;

        ImVec2 rowPos = ImGui::GetCursorScreenPos();

        ImVec2 topLeft(rowPos.x, rowPos.y - bgPaddingY);
        ImVec2 bottomRight(rowPos.x + width, rowPos.y + height + bgPaddingY);

        ImVec2 mousePos = ImGui::GetMousePos();
        bool rectHovered = mousePos.x >= topLeft.x && mousePos.x <= bottomRight.x &&
            mousePos.y >= topLeft.y && mousePos.y <= bottomRight.y;

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
        */

        ImVec2 rowPos = ImGui::GetCursorScreenPos();

        ImVec2 topLeft(rowPos.x, rowPos.y - bgPaddingY);
        ImVec2 bottomRight(rowPos.x + width, rowPos.y + height + bgPaddingY);

        ImVec2 mousePos = ImGui::GetMousePos();
        bool rectHovered = mousePos.x >= topLeft.x && mousePos.x <= bottomRight.x &&
            mousePos.y >= topLeft.y && mousePos.y <= bottomRight.y;

        if (rectHovered)
        {
            ImU32 leftColour = ColourUtil::RgbToU32(colour, 64);
            ImU32 rightColour = ColourUtil::RgbToU32(colour, 0);

            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();

            float normalizedMousePos = (ImGui::GetMousePos().x - windowPos.x) / windowSize.x;
            normalizedMousePos = std::clamp(normalizedMousePos, 0.f, 1.f);

            float splitX = topLeft.x + (bottomRight.x - topLeft.x) * normalizedMousePos;

            drawList->AddRectFilledMultiColor(topLeft,ImVec2(splitX, bottomRight.y),
                rightColour, leftColour, leftColour, rightColour);

            drawList->AddRectFilledMultiColor(ImVec2(splitX, topLeft.y), bottomRight,
                leftColour, rightColour, rightColour, leftColour);
        }

        return rectHovered;
        
    }

    inline void DrawTableDividerLine(ImGuiTable* table, int colIndex, 
        ImDrawList* drawList, ImU32 colour, float rowHeight)
    {
        ImGuiTableColumn* column = &table->Columns[colIndex];
        ImVec2 rowPos = ImGui::GetCursorScreenPos();
        float length = rowHeight + bgPaddingY - 2;

        //drawList->AddLine(ImVec2(column->MaxX, rowPos.y), ImVec2(column->MaxX, rowPos.y - length), colour);
        //drawList->AddLine(ImVec2(column->MaxX, rowPos.y), ImVec2(column->MaxX, rowPos.y - length), IM_COL32(255 * 0.66f, 255 * 0.66f, 255 * 0.66f, 255));
        drawList->AddLine(
            ImVec2(column->MaxX, rowPos.y), 
            ImVec2(column->MaxX, rowPos.y - length), 
            IM_COL32(85, 85, 85, 128));
    }

    inline std::string PadTableText(const std::string& s, int frontSpaces = 1, int backSpaces = 1)
    {
        frontSpaces = (std::max)(frontSpaces, 0);
        backSpaces = (std::max)(backSpaces, 0);
        return std::string(frontSpaces, ' ') + s + std::string(backSpaces, ' ');
    }

    enum class TextMode
    {
        Default,
        ArtistAndTitle,
    };

    inline std::string FormatTableText(const std::string& text, float maxWidth)
    {
        float width = ImGui::CalcTextSize(text.c_str()).x;

        if (width <= maxWidth)
        {
            return PadTableText(text);
        }

        // Truncated
        std::string clipped = text;

        while (!clipped.empty() &&
            ImGui::CalcTextSize((clipped + "...").c_str()).x > maxWidth)
        {
            clipped.pop_back();
        }

        clipped += "...";

        return PadTableText(clipped);
    }

    inline bool DrawTableButton(const std::string& text, TextMode mode = TextMode::Default)
    {
        if (text.empty())
        {
            /*
            if (mode != TextMode::ArtistAndTitle)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.66f, 0.66f, 0.66f, 1.f));
                TextUtil::DrawCenterJustifyTableText(PadTableText("-", 8, 8));
                ImGui::PopStyleColor();
            }
            */
            ImGui::TextUnformatted(PadTableText("", 8, 8).c_str());
            return false;
        }

        float maxWidth = (mode == TextMode::ArtistAndTitle) ? 550.f : 400.f;

        std::string textToDraw = FormatTableText(text, maxWidth);

        // Select
        bool clicked = false;
        ImVec2 pos = ImGui::GetCursorPos();
        //ImVec2 textSize = ImGui::CalcTextSize(textToDraw.c_str());

        SetClearSelectableStyle();
        ImGui::PushID(text.c_str());

        if (ImGui::Selectable("##TableSelect", false))
            clicked = true;
       
        ImGui::PopID();
        ImGui::PopStyleColor(3);
       
        ImGui::SetCursorPos(pos);

        // Draw Text
        bool hovered = ImGui::IsItemHovered();
        bool requireColourPop = false;

        if (mode != TextMode::ArtistAndTitle)
        {
            SetTextColourOnHover(hovered);
            requireColourPop = true;
        }

        TextUtil::DrawCenterJustifyTableText(textToDraw);
        //ImGui::TextUnformatted(textToDraw.c_str());

        if (requireColourPop)
            ImGui::PopStyleColor();

        // Full text tooltip
        if (hovered && ImGui::CalcTextSize(text.c_str()).x > maxWidth)
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(600.0f);
            ImGui::TextUnformatted(text.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }

        return clicked;
    }
};