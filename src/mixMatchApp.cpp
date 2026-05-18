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

    style.FrameRounding = 6.f;
    style.WindowRounding = 8.f;
    //style.GrabRounding = 6.f;
    //style.ScrollbarRounding = 6.f;

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
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            float headerRectHeight = 40.f;

            draw_list->AddRectFilled(ImVec2(0, mainSearchBarSize.y),
                ImVec2(io.DisplaySize.x, mainSearchBarSize.y + headerRectHeight),
                ColourUtil::RgbToU32(activeTrack->colour, 64));

            draw_list->AddRectFilled(ImVec2(io.DisplaySize.x * 0.75f, mainSearchBarSize.y + headerRectHeight),
                ImVec2(io.DisplaySize.x, mainSearchBarSize.y + (headerRectHeight * 2.f)),
                ColourUtil::RgbToU32(activeTrack->colour, 64));

            ImGui::Dummy(ImVec2(io.DisplaySize.x - (style.WindowPadding.x * 2.f), headerRectHeight));
        }

        //
        std::string artistAndTitle = activeTrack->artist + " - " + activeTrack->title;
        ImGui::Text(artistAndTitle.c_str());

        if (!activeTrack->label.empty())
        {
            if (ImGui::Button(activeTrack->label.c_str()))
            {
                activeLabel = activeTrack->label;
                mainPage = MainPage::LABEL;
            }
        }

        if (!activeTrack->release.empty())
        {
            if (ImGui::Button(activeTrack->release.c_str()))
            {
                activeRelease = activeTrack->release;
                mainPage = MainPage::RELEASE;
            }
        }
            
        if (!activeTrack->position.empty())
            ImGui::Text(("Track: " + activeTrack->position).c_str());

        if (activeTrack->bpm > 0.f)
            ImGui::Text(std::to_string(activeTrack->bpm).c_str());

        DrawStarRating(activeTrack->rating);

        // Edit active track button
        if (ImGui::Button(Ui::Text::ICON_EDIT))
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

        // Delete track button
        if (ImGui::Button(Ui::Text::ICON_DELETE))
        {
            deleteTrackData.trackId = activeTrackId;
            deleteTrackData.mixId = -1;
            deleteTrackData.label = activeTrack->artist + " - " + activeTrack->title;

            showDeleteTrackPopup = true;
            ImGui::OpenPopup(deleteTrackData.label.c_str());
        }

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
        else {
            // Display Mixes //
            bool deleteMixButtonPressed = false;

            ImGui::PushStyleVar(
                ImGuiStyleVar_CellPadding,
                ImVec2(0, 0)
            );

            if (ImGui::BeginTable(
                "Mix Display Table",
                6,
                ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableSetupColumn("Direction", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFrameHeight());
                ImGui::TableSetupColumn("Mix Rating", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFrameHeight());
                ImGui::TableSetupColumn("Artist & Title", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
                //ImGui::TableSetupColumn("BPM", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Rating", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Remove", ImGuiTableColumnFlags_WidthFixed, ImGui::GetFrameHeight());

                for (const Mix& mix : activeTrack->mix)
                {
                    const Track* mixTrack = manager.getTrackForDisplay(mix.id);

                    if (mixTrack == nullptr)
                        continue;

                    std::string rowId = std::to_string(mixTrack->id);

                    // TODO: push id once per row

                    ImGui::TableNextRow();
                    ImGui::PushID(mixTrack->id);

                    // Track Select
                    ImGui::TableSetColumnIndex(0);

                    if (ImGui::Selectable(
                        "##Mix Track Select",
                        false,
                        ImGuiSelectableFlags_SpanAllColumns |
                        ImGuiSelectableFlags_AllowOverlap))
                    {
                        activeTrackId = mixTrack->id;
                        mainPage = MainPage::ACTIVE_TRACK;
                    }
                    ImGui::SameLine();

                    // Direction 
                    ImGui::TableSetColumnIndex(0);

                    std::string directionIcon = "";

                    switch (mix.direction)
                    {
                    case MixDirection::In:
                        directionIcon = Ui::Text::ICON_ARROWFORWARD;
                        break;
                    case MixDirection::Out:
                        directionIcon = Ui::Text::ICON_ARROWBACK;
                        break;
                    case MixDirection::InAndOut:
                        directionIcon = Ui::Text::ICON_SYNCALT;
                        break;
                    }
                    // TODO: Draw two icons on top of each other

                    if (ImGui::Button(directionIcon.c_str()))
                    {
                        int direction = static_cast<int>(mix.direction);

                        direction++;

                        if (direction >= static_cast<int>(MixDirection::NumDirections))
                            direction = 0;

                        Mix editedMix = mix;
                        editedMix.direction = static_cast<MixDirection>(direction);

                        manager.editMix(editedMix, activeTrackId);
                    }

                    // Mix rating
                    ImGui::TableSetColumnIndex(1);
                    int rating = mix.rating;
                    if (ImGui::DragInt("##Mix Rating", &rating, 0.25f, 0, 5))
                    {
                        Mix editedMix = mix;
                        editedMix.rating = rating;
                        manager.editMix(editedMix, activeTrackId);
                    }

                    // Artist & title
                    ImGui::TableSetColumnIndex(2);
                    std::string artistAndTitle = mixTrack->artist + " - " + mixTrack->title;
                    TextUtil::DrawCenterJustifyTableText(artistAndTitle);

                    // Label
                    ImGui::TableSetColumnIndex(3);
                    TextUtil::DrawCenterJustifyTableText(mixTrack->label);

                    // Rating
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text(std::to_string(mixTrack->rating).c_str());

                    // Remove
                    ImGui::TableSetColumnIndex(5);

                    if (ImGui::Button(Ui::Text::ICON_CLOSE))
                    {
                        deleteMixData.trackId = activeTrackId;
                        deleteMixData.mixId = mixTrack->id;
                        deleteMixData.label = artistAndTitle;

                        deleteMixButtonPressed = true;
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            ImGui::PopStyleVar(1);

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
        
        DisplayTrackList(labelLibrary, TrackListMode::LABEL);
        break;
    }
    case MainPage::RELEASE:
    {
        const std::vector<const Track*> releaseLibrary = manager.getRelease(activeRelease);

        if (releaseLibrary.empty())
            break;

        ImGui::Text(activeRelease.c_str());

        DisplayTrackList(releaseLibrary, TrackListMode::RELEASE);
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

void MixMatchApp::DisplayTrackList(std::vector<const Track*> trackList, TrackListMode mode)
{
    if (trackList.empty())
        return;

    int cols = 0;

    switch (mode)
    {
    case TrackListMode::LABEL:
        cols = 3;
        break;
    case TrackListMode::RELEASE:
        cols = 4;
        break;
    default: return;
    }

    ImGui::PushStyleVar(
        ImGuiStyleVar_CellPadding,
        ImVec2(24, 0)
    );

    if (ImGui::BeginTable(
        "Track List Tabel",
        cols,
        ImGuiTableFlags_SizingFixedFit))
    {

        if (mode == TrackListMode::RELEASE)
            ImGui::TableSetupColumn("Track", ImGuiTableColumnFlags_WidthFixed);

        ImGui::TableSetupColumn("Artist & Title", ImGuiTableColumnFlags_WidthFixed);

        if (mode == TrackListMode::RELEASE)
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
        
        if (mode == TrackListMode::LABEL)
            ImGui::TableSetupColumn("Release", ImGuiTableColumnFlags_WidthFixed);

        ImGui::TableSetupColumn("Rating", ImGuiTableColumnFlags_WidthFixed);

        for (const Track* track : trackList)
        {
            if (track == nullptr)
                continue;

            ImGui::TableNextRow();
            ImGui::PushID(track->id);

            int colIndex = 0;

            // Track Select
            ImGui::TableSetColumnIndex(colIndex);

            if (ImGui::Selectable(
                "##Track Select",
                false,
                ImGuiSelectableFlags_SpanAllColumns |
                ImGuiSelectableFlags_AllowOverlap))
            {
                activeTrackId = track->id;
                mainPage = MainPage::ACTIVE_TRACK;
            }
            ImGui::SameLine();
         
            if (mode == TrackListMode::RELEASE)
            {
                ImGui::TableSetColumnIndex(colIndex);
                colIndex++;
                ImGui::Text(track->position.c_str());
            }

            // Artist & title
            ImGui::TableSetColumnIndex(colIndex);
            colIndex++;
            ImGui::Text(TrackUtil::FormartTitle(track->artist, track->title).c_str());

            // Label
            if (mode == TrackListMode::RELEASE)
            {
                ImGui::TableSetColumnIndex(colIndex);
                colIndex++;
                ImGui::Text(track->label.c_str());
            }

            // Release
            if (mode == TrackListMode::LABEL) 
            {
                ImGui::TableSetColumnIndex(colIndex);
                colIndex++;
                ImGui::Text(track->release.c_str());
            }

            // Rating
            ImGui::TableSetColumnIndex(colIndex);
            DrawStarRating(track->rating);

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar(1);
}