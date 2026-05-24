#include "..\include\mixMatchApp.hpp"

// TODO:
// Use context menus (right click) to view mix note?
// Add back button on the right of the main search bar
// Mix rating drag box feels weird, need to pass it a new int and then update the track
// Add remove track button
// Add 'Are you sure' windows for remove track and mix

// Add tags
// Add releases / sides / track number or letter (string)

// Add spring graph (maybe called constallation map or something)

// Add tap tempo 

MixMatchApp::MixMatchApp()
{
	manager.load();
}

void MixMatchApp::RunFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    // Main Search Bar //   
    //style.FrameRounding = ImGui::GetFrameHeight() * 0.5f;


    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 0));
    ImGui::SetNextWindowPos(ImVec2(0, 0));

#pragma region MainSearchBar

    // Main Search Bar //
    ImGui::Begin("##Main Search", &open,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse);

    ImVec2 mainSearchBarSize = ImGui::GetWindowSize();

    float frameHeight = ImGui::GetFrameHeight();
    ImVec2 mainSearchBarButtonSize = ImVec2(frameHeight, frameHeight);

    // Add new track button
    if (ImGui::Button(Ui::Text::ICON_ADD, mainSearchBarButtonSize))
    {
        addTrackData = {};
        showAddTrackPopup = true;
        ImGui::OpenPopup("Add Track");
    }

    ImGui::SameLine();

    float tableX = ImGui::GetCursorScreenPos().x;
    float tableWidth = ImGui::GetContentRegionAvail().x - frameHeight - style.ItemSpacing.x;

    ImGui::SetNextItemWidth(tableWidth);
    ImGui::InputText("##MainSeachInput", mainSearchBuffer, sizeof(mainSearchBuffer));

    ImGui::SameLine();

    // Spring graph button 
    if (ImGui::Button(Ui::Text::ICON_HALFSTAR, mainSearchBarButtonSize))
        showSpringDiagram = !showSpringDiagram;

    if (mainSearchBuffer[0] != '\0')
        showMainLibary = true;

    // Main libary (search results)
    if (showMainLibary)
    {
        const std::vector<Track> mainLibrary =
            manager.searchAndSort(mainSearchBuffer, mainSearchSort);

        TrackSearchTable(mainLibrary, tableX, tableWidth);
    }

    // Add track popup window
    SetModalPopupPosAndSize("Add Track");

    ImGui::PushStyleColor(ImGuiCol_TitleBg, ColourUtil::TintVec4(addTrackData.colour, Ui::Colour::TINT_BG));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ColourUtil::TintVec4(addTrackData.colour, Ui::Colour::TINT_HOVER));

    if (ImGui::BeginPopupModal("Add Track", &showAddTrackPopup, ImGuiWindowFlags_AlwaysAutoResize))
    {
        InputTrackDataPopup(addTrackData, TrackInputMode::ADD);
        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);

    ImGui::End();

#pragma endregion MainSearchBar


#pragma region TrackMap
    /*
    if (showSpringDiagram)
    {
        
        ImGui::SetNextWindowPos(ImVec2(0, 75));
        ImGui::SetNextWindowSize(
            ImVec2(io.DisplaySize.x, io.DisplaySize.y - 75),
            ImGuiCond_Always
        );
        ImGui::Begin("##Spring", &open,
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse);


        //springGraph.simulate(trackNodes, trackEdges, ImGui::GetIO().DeltaTime);

        // Draw
        //auto* draw = ImGui::GetWindowDrawList();

        //// draw edges
        //for (const auto& e : trackEdges) {
        //    const auto& a = trackNodes[e.a];
        //    const auto& b = trackNodes[e.b];

        //    draw->AddLine(a.pos, b.pos, IM_COL32(100, 100, 100, 255));
        //}

        //// draw nodes
        //for (const auto& n : trackNodes) {
        //    draw->AddCircleFilled(n.pos, 4.0f, IM_COL32(255, 200, 100, 255));
        //}

        ImGui::End();
    }
    */

#pragma endregion TrackMap

    // Main Window //
    ImGui::SetNextWindowPos(ImVec2(0, 75));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y - 75), ImGuiCond_Always);

    ImGui::Begin("##Main Window", &open,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse);

    switch (mainPage)
    {
    case MainPage::NONE: break;
    case MainPage::ACTIVE_TRACK:
    {
#pragma region ActiveTrackHeader

        const Track* activeTrack = manager.getTrackForDisplay(activeTrackId);

        if (activeTrack == nullptr)
            break;

        // Track header graphic
        
        ImVec2 rectSize(ImGui::GetWindowSize().x, 300); //ImGui::GetContentRegionAvail().x

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 scroll(ImGui::GetScrollX(), ImGui::GetScrollY());

        ImVec2 start(windowPos.x - scroll.x, windowPos.y - scroll.y);
        ImVec2 end(start.x + rectSize.x, start.y + rectSize.y);

        ImU32 topColour = ColourUtil::RgbToU32(activeTrack->colour, 64);
        ImU32 bottomColour = ColourUtil::RgbToU32(activeTrack->colour, 0);

        const ImDrawFlags corners = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight;

        drawList->AddRectFilledMultiColor(start, end, topColour, topColour, bottomColour, bottomColour);

        // Artist & title
        ImGui::PushFont(Ui::Text::BigFont);
        ImGui::Text(TextUtil::FormartTitle(activeTrack->artist, activeTrack->title).c_str());
        ImGui::PopFont();

        ImGui::SameLine();

        // Edit active track button
        ImVec2 editSelSize = ImGui::CalcTextSize(Ui::Text::ICON_EDIT);
        ImVec2 editSelPos = ImGui::GetCursorPos();

        SetClearSelectableStyle();

        if (ImGui::Selectable("##Edit Select", false, NULL, editSelSize))
        {
            editTrackData = {};
            editTrackData.id = activeTrackId;
            strcpy_s(editTrackData.artist, activeTrack->artist.c_str());
            strcpy_s(editTrackData.title, activeTrack->title.c_str());
            strcpy_s(editTrackData.label, activeTrack->label.c_str());
            strcpy_s(editTrackData.release, activeTrack->release.c_str());
            strcpy_s(editTrackData.position, activeTrack->position.c_str());
            editTrackData.bpm = activeTrack->bpm;
            editTrackData.rating = activeTrack->rating;
            editTrackData.colour = ColourUtil::RgbToImVec4(activeTrack->colour);

            showEditTrackPopup = true;
            ImGui::OpenPopup("Edit Track");
        }

        ImGui::PopStyleColor(3);
        ImGui::SameLine();

        ImGui::SetCursorPos(editSelPos);
        SetTextColourOnHover(ImGui::IsItemHovered());
        ImGui::Text(Ui::Text::ICON_EDIT);
        ImGui::PopStyleColor();

        ImGui::SameLine();

        // Delete track button
        ImVec2 deleteSelSize = ImGui::CalcTextSize(Ui::Text::ICON_DELETE);
        ImVec2 deleteSelPos = ImGui::GetCursorPos();

        SetClearSelectableStyle();

        if (ImGui::Selectable("##Delet Select", false, NULL, deleteSelSize))
        {
            deleteTrackData.trackId = activeTrackId;
            deleteTrackData.mixId = -1;
            deleteTrackData.label = activeTrack->artist + " - " + activeTrack->title;

            showDeleteTrackPopup = true;
            ImGui::OpenPopup(deleteTrackData.label.c_str());
        }

        ImGui::PopStyleColor(3);
        ImGui::SameLine();

        ImGui::SetCursorPos(deleteSelPos);
        SetTextColourOnHover(ImGui::IsItemHovered());
        ImGui::Text(Ui::Text::ICON_DELETE);
        ImGui::PopStyleColor();

        // Label
        if (!activeTrack->label.empty())
        {   
            std::string label = activeTrack->label;
            ImVec2 selectSize = ImGui::CalcTextSize(label.c_str());
            ImVec2 selectPos = ImGui::GetCursorPos();

            SetClearSelectableStyle();

            if (ImGui::Selectable("##Label Select", false, NULL, selectSize))
            {
                activeLabel = label;
                mainPage = MainPage::LABEL;
            }

            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            ImGui::SetCursorPos(selectPos);
            SetTextColourOnHover(ImGui::IsItemHovered());
            ImGui::Text(label.c_str());
            ImGui::PopStyleColor();
        }

        // Release
        if (!activeTrack->release.empty())
        {
            std::string release = activeTrack->release;
            ImVec2 selectSize = ImGui::CalcTextSize(release.c_str());
            ImVec2 selectPos = ImGui::GetCursorPos();

            SetClearSelectableStyle();

            if (ImGui::Selectable("##Release Select", false, NULL, selectSize))
            {
                activeRelease = release;
                mainPage = MainPage::RELEASE;
            }

            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            ImGui::SetCursorPos(selectPos);
            SetTextColourOnHover(ImGui::IsItemHovered());

            ImGui::Text(release.c_str());

            if (!activeTrack->position.empty())
                ImGui::Text(("Track: " + activeTrack->position).c_str());

            ImGui::PopStyleColor();
        }
            

        StarRating(activeTrack);

        // Track edit popup
        SetModalPopupPosAndSize("Edit Track");

        ImGui::PushStyleColor(ImGuiCol_TitleBg, ColourUtil::TintVec4(editTrackData.colour, Ui::Colour::TINT_BG));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ColourUtil::TintVec4(editTrackData.colour, Ui::Colour::TINT_HOVER));

        if (ImGui::BeginPopupModal("Edit Track", &showEditTrackPopup, ImGuiWindowFlags_AlwaysAutoResize))
        {
            InputTrackDataPopup(editTrackData, TrackInputMode::EDIT);

            if (!showEditTrackPopup)
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
        ImGui::PopStyleColor(2);


        // Delete track popup
        SetModalPopupPosAndSize(deleteTrackData.label.c_str());
        if (ImGui::BeginPopupModal(deleteTrackData.label.c_str(), &showDeleteTrackPopup, ImGuiWindowFlags_AlwaysAutoResize))
        {
            DeleteConformationPopup(deleteTrackData, DeleteConformationMode::TRACK);

            if (!showDeleteTrackPopup)
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

#pragma endregion ActiveTrackHeader


#pragma region ActiveTrackMixes

        ImGui::SeparatorText("Mixes");

        // Add new mix button
        if (ImGui::Button(showAddMixWindow ? Ui::Text::ICON_REMOVE : Ui::Text::ICON_ADD))
            showAddMixWindow = !showAddMixWindow;

        if (showAddMixWindow)
        {
            int mixToAddId = -1;

            // Search for new mix to add
            ImGui::SameLine();
            ImGui::InputText("##Mix Search Input", mixSearchBuffer, sizeof(mixSearchBuffer));

            const std::vector<Track> mixSearchLibrary =
                manager.searchAndSort(mixSearchBuffer, LibraryManager::TrackSort::Artist);

            ImGui::PushStyleVar(
                ImGuiStyleVar_CellPadding,
                ImVec2(24, 0)
            );

            if (ImGui::BeginTable(
                "Mix Search Table",
                3,
                ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableSetupColumn("Artist & Title", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
                //ImGui::TableSetupColumn("BPM", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Rating", ImGuiTableColumnFlags_WidthStretch);

                for (const Track& mixSearch : mixSearchLibrary)
                {
                    if (mixSearch.id == activeTrackId)
                        continue;

                    bool matchingMix = false;
                    for (const Mix& parentTrackMix : activeTrack->mix)
                    {
                        if (mixSearch.id == parentTrackMix.id)
                        {
                            matchingMix = true;
                            break;
                        }
                    }

                    if (matchingMix)
                        continue;

                    ImGui::TableNextRow();

                    // Track Select
                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushID(mixSearch.id);
                    if (ImGui::Selectable(
                        "##MixSearchTrackSelect",
                        false,
                        ImGuiSelectableFlags_SpanAllColumns))
                    {
                        mixToAddId = mixSearch.id;
                    }
                    ImGui::PopID();
                    ImGui::SameLine();

                    // Artist & title
                    ImGui::TableSetColumnIndex(0);
                    std::string artistAndTitle = mixSearch.artist + " - " + mixSearch.title;
                    TextUtil::DrawCenterJustifyTableText(artistAndTitle);

                    // Label
                    ImGui::TableSetColumnIndex(1);
                    TextUtil::DrawCenterJustifyTableText(mixSearch.label);

                    // Rating
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text(std::to_string(mixSearch.rating).c_str());
                }
                ImGui::EndTable();
            }
            ImGui::PopStyleVar(1);

            if (mixToAddId > -1)
            {
                manager.addMix(mixToAddId, activeTrackId);
                showAddMixWindow = false;
            }
        }
        else 
        {
            // Display Mixes //
            DisplayMixTable(activeTrack->mix);            
        }

#pragma endregion ActiveTrackMixes

        // Theads

        // Sets

        break;
    }
    case MainPage::LABEL:
    {
        const std::vector<const Track*> labelLibrary = manager.getLabel(activeLabel);

        if (labelLibrary.empty())
            break;

        ImGui::Text(activeLabel.c_str());
        DisplayLabelTable(labelLibrary);
        break;
    }
    case MainPage::RELEASE:
    {
        const std::vector<const Track*> releaseLibrary = manager.getRelease(activeRelease);

        if (releaseLibrary.empty())
            break;

        ImGui::Text(activeRelease.c_str());
        DisplayReleaseTable(releaseLibrary);
        break;
    }
    default: break;
    }

    ImGui::End();
}

void MixMatchApp::DeleteConformationPopup(DeleteData& data, DeleteConformationMode mode)
{
    // TODO: Use track colour for window header

    const char* text = mode == DeleteConformationMode::TRACK ?
        "Delete track?" :
        "Remove mix?";

    float windowWidth = ImGui::GetWindowSize().x;
    float textWidth = ImGui::CalcTextSize(text).x;

    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text("%s", text);

    // Yes / No buttons
    float buttonWidth = 80.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;

    float totalWidth = buttonWidth * 2.0f + spacing;

    float availableSpace = ImGui::GetContentRegionAvail().x;

    ImGui::SetCursorPosX(
        ImGui::GetCursorPosX() + (availableSpace - totalWidth) * 0.5f
    );

    if (ImGui::Button("Yes"))
    {
        switch (mode)
        {
        case DeleteConformationMode::TRACK:
            manager.removeTrack(data.trackId);
            break;
        case DeleteConformationMode::MIX:
            manager.removeMix(data.trackId, data.mixId);
            break;
        }

        ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();

    if (ImGui::Button("No"))
        ImGui::CloseCurrentPopup();
}

void MixMatchApp::InputTrackDataPopup(InputData& data, TrackInputMode mode)
{
    // TODO: Use tabs
    if (ImGui::BeginTable("InputTrackTable", 2, ImGuiTableFlags_SizingFixedFit))
    {
        float fieldWidth = 500.0f;
        float fieldHeight = ImGui::GetFrameHeight();

        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Artist");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(fieldWidth);
        ImGui::InputText("##Artist", data.artist, sizeof(data.artist));

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Title");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(fieldWidth);
        ImGui::InputText("##Title", data.title, sizeof(data.title));

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Label");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(fieldWidth);
        ImGui::InputText("##Label", data.label, sizeof(data.label));

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Release");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(fieldWidth);
        ImGui::InputText("##Release", data.release, sizeof(data.release));

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Side / Number");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(fieldWidth);
        ImGui::InputText("##Position", data.position, sizeof(data.position));

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("BPM");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(fieldWidth);
        ImGui::InputFloat("##BPM", &data.bpm, 1.f, 10.f, "%.2f");
        data.bpm = std::max(0.f, data.bpm);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Rating");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(fieldWidth);
        ImGui::InputInt("##Rating", &data.rating, 1, 1);
        data.rating = std::clamp(data.rating, 0, 5);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Colour");
        ImGui::TableSetColumnIndex(1);
        if (ImGui::ColorButton(
            "##ColourButton",
            data.colour,
            ImGuiColorEditFlags_NoTooltip |
            ImGuiColorEditFlags_NoBorder,
            ImVec2(fieldWidth, fieldHeight)))
        {
            ImGui::OpenPopup("##ColorPickerPopup");
        }
        if (ImGui::BeginPopup("##ColorPickerPopup"))
        {
            ImGui::ColorPicker3("##ColourPicker",
                (float*)&data.colour,
                ImGuiColorEditFlags_NoSmallPreview |
                ImGuiColorEditFlags_NoLabel |
                ImGuiColorEditFlags_NoSidePreview
            );
            ImGui::EndPopup();
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);

        ImVec2 saveButtonSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 1.f, 0.f, Ui::Colour::ALPHA_BG));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 1.f, 0.f, Ui::Colour::ALPHA_HOVER));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 1.f, 0.f, Ui::Colour::ALPHA_ACTIVE));

        if (ImGui::Button(Ui::Text::ICON_SAVE, saveButtonSize))
        {
            Track t;
            t.artist = data.artist;
            t.title = data.title;
            t.label = data.label;
            t.release = data.release;
            t.position = data.position;
            t.bpm = data.bpm;
            t.rating = data.rating;
            t.colour = ColourUtil::ImVec4ToRgb(data.colour);

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
        ImGui::PopStyleColor(3);

        ImGui::TableSetColumnIndex(1);
        if (data.result !=
            LibraryManager::ValidationResult::None ||
            LibraryManager::ValidationResult::ValidTrack)
        {
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
        ImGui::EndTable();
    }
}

void MixMatchApp::TrackSearchTable(const std::vector<Track>& library, float x, float width)
{
    if (library.empty())
        return;

    ImGui::Separator();

    ImGui::PushStyleVar(
        ImGuiStyleVar_CellPadding,
        ImVec2(24, 12)
    );

    if (ImGui::BeginTable(
        "TrackTable",
        1))    //ImVec2(width, 0))
    {
        ImGui::TableSetupColumn("TrackText");


        for (const Track& track : library)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            std::string rowId = std::to_string(track.id);

            //ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0));
            //ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));
            //ImGui::PushStyleColor(ImGuiCol_NavHighlight, IM_COL32(0, 0, 0, 0));

            if (ImGui::Selectable(
                ("##TableSelect" + rowId).c_str(),
                false,
                ImGuiSelectableFlags_SpanAllColumns))
            {
                mainSearchBuffer[0] = '\0';
                showMainLibary = false;
                activeTrackId = track.id;
                mainPage = MainPage::ACTIVE_TRACK;
            }
            //ImGui::PopStyleColor(3);
            bool hovered = ImGui::IsItemHovered();
            ImGui::SameLine();

            /*
            if (hovered)
            {
                float rectPaddingY = 7.f;
                ImVec2 rectMin = ImGui::GetItemRectMin();
                ImVec2 rectMax = ImGui::GetItemRectMax();

                rectMin.y -= rectPaddingY;
                rectMax.y += rectPaddingY;

                ImGui::GetWindowDrawList()->AddRectFilled(
                    rectMin,
                    rectMax,
                    ColourUtil::RgbToU32(
                        track.colour,
                        (uint8_t)(Ui::Colour::ALPHA_HOVER * 255)),
                    6.0f);
            }
            */
 

            ImGui::TableSetColumnIndex(0);
            std::string trackText = track.artist + " - " + track.title;

            if (!track.label.empty())
                trackText += " | " + track.label;

            TextUtil::DrawCenterJustifyTableText(trackText);
    
            //ImGui::TableSetColumnIndex(2);

        }
        ImGui::EndTable();
    }

    ImGui::PopStyleVar(1);
}


void MixMatchApp::DisplayLabelTable(std::vector<const Track*> trackList)
{
    if (trackList.empty())
        return;
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGui::PushStyleVar(
        ImGuiStyleVar_CellPadding,
        ImVec2(tablePaddingX, tablePaddingY));

    if (ImGui::BeginTable(
        "Label Tabel",
        3,
        ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Artist & Title", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Release", ImGuiTableColumnFlags_WidthStretch);
        // Tags
        //ImGui::TableSetupColumn("BPM", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Rating", ImGuiTableColumnFlags_WidthStretch);

        ImGuiTable* table = ImGui::GetCurrentTable();
        float rowWidth = table->WorkRect.Max.x - table->WorkRect.Min.x;
        float rowHeight = ImGui::GetTextLineHeight();

        float ratingSize = ImGui::CalcTextSize(Ui::Text::ICON_STAR).x * 5;

        for (const Track* track : trackList)
        {
            if (track == nullptr)
                continue;

            ImGui::TableNextRow();
            ImGui::PushID(track->id);

            // Background
            ImGui::TableSetColumnIndex(0);
            bool hovered = DrawTableBg(drawList, rowWidth, rowHeight, track->colour, track->bpm);

            ImU32 lineColour = ColourUtil::RgbToU32(track->colour, hovered ? 88 : 64);

            // Artist & title
            ImGui::TableSetColumnIndex(0);

            SetClearSelectableStyle();
            if (ImGui::Selectable("##Track Select", false))
            {
                activeTrackId = track->id;
                mainPage = MainPage::ACTIVE_TRACK;
            }
            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            ImGui::TableSetColumnIndex(0);
            std::string artistAndTitle = TextUtil::FormartTitle(track->artist, track->title);
            TextUtil::DrawCenterJustifyTableText(PadTableText(artistAndTitle));

            DrawTableDividerLine(table, 0, drawList, lineColour, rowHeight);

            // Release
            std::string release = track->release;
            if (!release.empty())
            {
                ImGui::TableSetColumnIndex(1);

                SetClearSelectableStyle();
                if (ImGui::Selectable("##Release Select", false))
                {
                    activeRelease = release;
                    mainPage = MainPage::RELEASE;
                }
                ImGui::PopStyleColor(3);
                ImGui::SameLine();

                ImGui::TableSetColumnIndex(1);
                SetTextColourOnHover(ImGui::IsItemHovered());
                TextUtil::DrawCenterJustifyTableText(PadTableText(release));
                ImGui::PopStyleColor();
            }
            else
            {
                ImGui::TableSetColumnIndex(1);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.66f, 0.66f, 0.66f, 1.f));
                TextUtil::DrawCenterJustifyTableText(PadTableText("-", 8, 8));
                ImGui::PopStyleColor();
            }

            DrawTableDividerLine(table, 1, drawList, lineColour, rowHeight);

            // Rating
            ImGui::TableSetColumnIndex(2);
            float colWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + colWidth - ratingSize - tablePaddingX);
            StarRating(track);

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
}


void MixMatchApp::DisplayReleaseTable(std::vector<const Track*> trackList)
{
    if (trackList.empty())
        return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGui::PushStyleVar(
        ImGuiStyleVar_CellPadding,
        ImVec2(tablePaddingX, tablePaddingY));

    if (ImGui::BeginTable(
        "Release Tabel",
        4,
        ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Artist & Title", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
        // Tags
        //ImGui::TableSetupColumn("BPM", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Rating", ImGuiTableColumnFlags_WidthStretch);

        ImGuiTable* table = ImGui::GetCurrentTable();
        float rowWidth = table->WorkRect.Max.x - table->WorkRect.Min.x;
        float rowHeight = ImGui::GetTextLineHeight();

        float ratingSize = ImGui::CalcTextSize(Ui::Text::ICON_STAR).x * 5;

        for (const Track* track : trackList)
        {
            if (track == nullptr)
                continue;

            ImGui::TableNextRow();
            ImGui::PushID(track->id);

            // Background
            ImGui::TableSetColumnIndex(0);
            bool hovered = DrawTableBg(drawList, rowWidth, rowHeight, track->colour, track->bpm);

            ImU32 lineColour = ColourUtil::RgbToU32(track->colour, hovered ? 88 : 64);

            // Position
            ImGui::TableSetColumnIndex(0);
            std::string position = track->position;

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.66f, 0.66f, 0.66f, 1.f));
            ImGui::Text(PadTableText(position.empty() ? "-" : position, 2, 2).c_str());
            ImGui::PopStyleColor();

            DrawTableDividerLine(table, 0, drawList, lineColour, rowHeight);

            // Artist & title
            ImGui::TableSetColumnIndex(1);

            SetClearSelectableStyle();
            if (ImGui::Selectable("##Track Select", false))
            {
                activeTrackId = track->id;
                mainPage = MainPage::ACTIVE_TRACK;
            }
            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            ImGui::TableSetColumnIndex(1);
            std::string artistAndTitle = TextUtil::FormartTitle(track->artist, track->title);
            TextUtil::DrawCenterJustifyTableText(PadTableText(artistAndTitle));

            DrawTableDividerLine(table, 1, drawList, lineColour, rowHeight);

            // Label
            std::string label = track->label;
            if (!label.empty())
            {
                ImGui::TableSetColumnIndex(2);

                SetClearSelectableStyle();
                if (ImGui::Selectable("##Label Select", false))
                {
                    activeLabel = label;
                    mainPage = MainPage::LABEL;
                }
                ImGui::PopStyleColor(3);
                ImGui::SameLine();

                ImGui::TableSetColumnIndex(2);
                SetTextColourOnHover(ImGui::IsItemHovered());
                TextUtil::DrawCenterJustifyTableText(PadTableText(label));
                ImGui::PopStyleColor();
            }
            else
            {
                ImGui::TableSetColumnIndex(2);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.66f, 0.66f, 0.66f, 1.f));
                TextUtil::DrawCenterJustifyTableText(PadTableText("-", 8, 8));
                ImGui::PopStyleColor();
            }

            DrawTableDividerLine(table, 2, drawList, lineColour, rowHeight);

            // Rating
            ImGui::TableSetColumnIndex(3);
            float colWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + colWidth - ratingSize - tablePaddingX);
            StarRating(track);

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
}


void MixMatchApp::DisplayMixTable(const std::vector<Mix>& mixList)
{
    if (mixList.empty())
        return;

    bool deleteMixButtonPressed = false;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGui::PushStyleVar(
        ImGuiStyleVar_CellPadding,
        ImVec2(tablePaddingX, tablePaddingY));

    if (ImGui::BeginTable(
        "Mix Table",
        7,
        ImGuiTableFlags_SizingStretchProp))
    {
        float frameHeight = ImGui::GetFrameHeight();
        ImGui::TableSetupColumn("Direction", ImGuiTableColumnFlags_WidthFixed, frameHeight);
        ImGui::TableSetupColumn("Mix Rating", ImGuiTableColumnFlags_WidthFixed, frameHeight);
        ImGui::TableSetupColumn("Artist & Title", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Release", ImGuiTableColumnFlags_WidthStretch);
        //ImGui::TableSetupColumn("BPM", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Track Rating", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Remove", ImGuiTableColumnFlags_WidthFixed, frameHeight);

        ImGuiTable* table = ImGui::GetCurrentTable();
        float rowWidth = table->WorkRect.Max.x - table->WorkRect.Min.x;
        float rowHeight = ImGui::GetTextLineHeight();

        float ratingSize = ImGui::CalcTextSize(Ui::Text::ICON_STAR).x * 5;

        for (const Mix& mix : mixList)
        {
            const Track* mixTrack = manager.getTrackForDisplay(mix.id);

            if (mixTrack == nullptr)
                continue;

            ImGui::TableNextRow();
            ImGui::PushID(mixTrack->id);

            // Background
            ImGui::TableSetColumnIndex(0);
            bool hovered = DrawTableBg(drawList, rowWidth, rowHeight, mixTrack->colour, mixTrack->bpm);

            ImU32 lineColour = ColourUtil::RgbToU32(mixTrack->colour, hovered ? 88 : 64);

            // Direction 
            ImGui::TableSetColumnIndex(0);

            std::string directionIcon = "";

            if(mix.direction == MixDirection::In)
                directionIcon = Ui::Text::ICON_ARROWFORWARD;
            else if (mix.direction == MixDirection::Out)
                directionIcon = Ui::Text::ICON_ARROWBACK;
            else
                directionIcon = Ui::Text::ICON_SYNCALT;

            SetClearSelectableStyle();
            if (ImGui::Selectable("##Direction", false))
            {
                int direction = static_cast<int>(mix.direction);

                direction++;

                if (direction >= static_cast<int>(MixDirection::NumDirections))
                    direction = 0;

                Mix editedMix = mix;
                editedMix.direction = static_cast<MixDirection>(direction);

                manager.editMix(editedMix, activeTrackId);
            }
            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            ImGui::TableSetColumnIndex(0);
            SetTextColourOnHover(ImGui::IsItemHovered());
            TextUtil::DrawCenterJustifyTableText(directionIcon.c_str());
            ImGui::PopStyleColor();

            // Mix rating
            ImGui::TableSetColumnIndex(1);
            int rating = mix.rating;
            
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));

            if (ImGui::InputInt("##Mix Rating", &rating, 0.25f, 0, 5))
            {
                Mix editedMix = mix;
                editedMix.rating = rating;
                manager.editMix(editedMix, activeTrackId);
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(3);

            DrawTableDividerLine(table, 1, drawList, lineColour, rowHeight);
      
            // Artist & title
            ImGui::TableSetColumnIndex(2);

            std::string artistAndTitle = TextUtil::FormartTitle(mixTrack->artist, mixTrack->title);

            SetClearSelectableStyle();

            if (ImGui::Selectable("##Track Select", false))
            {
                activeTrackId = mixTrack->id;
                mainPage = MainPage::ACTIVE_TRACK;
            }

            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            ImGui::TableSetColumnIndex(2);
            TextUtil::DrawCenterJustifyTableText(PadTableText(artistAndTitle));

            DrawTableDividerLine(table, 2, drawList, lineColour, rowHeight);

            // Label
            std::string label = mixTrack->label;
            if (!label.empty())
            {
                ImGui::TableSetColumnIndex(3);

                SetClearSelectableStyle();

                if (ImGui::Selectable("##Label Select", false))
                {
                    activeLabel = label;
                    mainPage = MainPage::LABEL;
                }

                ImGui::PopStyleColor(3);
                ImGui::SameLine();

                ImGui::TableSetColumnIndex(3);
                SetTextColourOnHover(ImGui::IsItemHovered());
                TextUtil::DrawCenterJustifyTableText(PadTableText(label));
                ImGui::PopStyleColor();
            }
            else 
            {
                ImGui::TableSetColumnIndex(3);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.66f, 0.66f, 0.66f, 1.f));
                TextUtil::DrawCenterJustifyTableText(PadTableText("-", 8, 8));
                ImGui::PopStyleColor();
            }

            DrawTableDividerLine(table, 3, drawList, lineColour, rowHeight);

            // Release
            std::string release = mixTrack->release;
            if (!release.empty())
            {
                ImGui::TableSetColumnIndex(4);

                SetClearSelectableStyle();

                if (ImGui::Selectable("##Release Select", false))
                {
                    activeRelease = release;
                    mainPage = MainPage::RELEASE;
                }

                ImGui::PopStyleColor(3);
                ImGui::SameLine();

                ImGui::TableSetColumnIndex(4);
                SetTextColourOnHover(ImGui::IsItemHovered());
                TextUtil::DrawCenterJustifyTableText(PadTableText(release));
                ImGui::PopStyleColor();
            }
            else
            {
                ImGui::TableSetColumnIndex(4);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.66f, 0.66f, 0.66f, 1.f));
                TextUtil::DrawCenterJustifyTableText(PadTableText("-", 8, 8));
                ImGui::PopStyleColor();
            }

            DrawTableDividerLine(table, 4, drawList, lineColour, rowHeight);

            // Rating
            ImGui::TableSetColumnIndex(5);
            float colWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + colWidth - ratingSize);
            StarRating(mixTrack);

            DrawTableDividerLine(table, 5, drawList, lineColour, rowHeight);

            // Remove
            ImGui::TableSetColumnIndex(6);

            SetClearSelectableStyle();

            if (ImGui::Selectable("##Remove", false))
            {
                deleteMixData.trackId = activeTrackId;
                deleteMixData.mixId = mixTrack->id;
                deleteMixData.label = artistAndTitle;

                deleteMixButtonPressed = true;
            }

            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            ImGui::TableSetColumnIndex(6);
            SetTextColourOnHover(ImGui::IsItemHovered());
            TextUtil::DrawCenterJustifyTableText(PadTableText(Ui::Text::ICON_CLOSE));
            ImGui::PopStyleColor();
            
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();

    // Delete mix popup
    if (deleteMixButtonPressed)
    {
        showDeleteMixPopup = true;
        ImGui::OpenPopup(deleteMixData.label.c_str());
    }

    SetModalPopupPosAndSize(deleteMixData.label.c_str());
    if (ImGui::BeginPopupModal(deleteMixData.label.c_str(), &showDeleteMixPopup, ImGuiWindowFlags_AlwaysAutoResize))
    {
        DeleteConformationPopup(deleteMixData, DeleteConformationMode::MIX);

        if (!showDeleteMixPopup)
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}

     