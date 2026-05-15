#include "..\include\mixMatchApp.hpp"

// TODO:
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

    // Main Search Bar //
    ImGui::Begin("##Main Search", &open,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse);

    float frameHeight = ImGui::GetFrameHeight();
    ImVec2 mainSearchBarButtonSize = ImVec2(frameHeight, frameHeight);

    // Add new track button
    if (ImGui::Button(Ui::Text::ICON_ADD, mainSearchBarButtonSize) && !showAddTrackWindow)
    {
        addData = {};
        showEditTrackWindow = false;
        showAddTrackWindow = true;
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

        TrackSearchTable2(mainLibrary, tableX, tableWidth);
    }

    ImGui::End();

    if (showSpringDiagram)
    {
        // Active track display
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

        //ImGui::End();

    }
    else
    {
        // Active track display
        ImGui::SetNextWindowPos(ImVec2(0, 75));
        ImGui::SetNextWindowSize(
            ImVec2(io.DisplaySize.x, io.DisplaySize.y - 75),
            ImGuiCond_Always
        );
        ImGui::Begin("##Active Track Display", &open,
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse);

        DrawActiveTrackDisplay(activeTrackId);

        ImGui::End();
    }

    if (showAddTrackWindow)
        InputTrackDataWindow(addData, TrackInputMode::ADD);

    if (showEditTrackWindow)
        InputTrackDataWindow(editData, TrackInputMode::EDIT);

    if (showDeleteTrackWindow)
        DeleteConformationWindow(deleteTrackData, DeleteConformationMode::TRACK);
    
    if (showDeleteMixWindow)
        DeleteConformationWindow(deleteMixData, DeleteConformationMode::MIX);
}


void MixMatchApp::DeleteConformationWindow(DeleteData& data, MixMatchApp::DeleteConformationMode mode)
{
    bool* windowOpen = mode == DeleteConformationMode::TRACK ? &showDeleteTrackWindow : &showDeleteMixWindow;
    const char* title = mode == DeleteConformationMode::TRACK ? "Permanently Delete Track" : " Permanently Remove Mix";

    // Close delete window if active track changes
    if (data.trackId != activeTrackId)
        *windowOpen = false;

    ImGui::Begin(title, windowOpen, ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Are you sure?");

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

        *windowOpen = false;
    }

    ImGui::SameLine();

    if (ImGui::Button("No"))
        *windowOpen = false;

    ImGui::End();
}


void MixMatchApp::InputTrackDataWindow(InputData& data, TrackInputMode mode)
{
    bool* windowOpen = mode == ADD ? &showAddTrackWindow : &showEditTrackWindow;
    const char* title = mode == ADD ? "Add New Track" : "Edit Track";

    // Close edit window if active track changes
    if (mode == EDIT && data.id != activeTrackId)
        *windowOpen = false;

    ImGui::PushStyleColor(ImGuiCol_TitleBg, ColourUtil::TintVec4(data.colour, Ui::Colour::TINT_BG));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ColourUtil::TintVec4(data.colour, Ui::Colour::TINT_HOVER));

    ImGui::Begin(
        title, 
        windowOpen, 
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize);

    ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

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
        ImGui::Text("BPM");
        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(fieldWidth);
        ImGui::InputFloat("##BPM", &data.bpm, 1.f, 10.f, "%.2f");
        data.bpm = std::max(0.f, data.bpm);

        // TODO: Use selectable stars?
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
            t.bpm = data.bpm;
            t.rating = data.rating;
            t.colour = ColourUtil::ImVec4ToRgb(data.colour);

            if (mode == ADD)
            {
                data.result = manager.addTrack(t);

                if (data.result == LibraryManager::ValidationResult::ValidTrack)
                {
                    if (!manager.getCatalogueForDisplay().empty())
                        activeTrackId = manager.getCatalogueForDisplay().back().id;
                }
            }
            else if (mode == EDIT)
            {
                t.id = activeTrackId;

                data.result = manager.editTrack(t);
            }

            if (data.result == LibraryManager::ValidationResult::ValidTrack)
            {
                data = {};
                *windowOpen = false;
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

    ImGui::PopStyleColor(2);
    ImGui::End();
}

void MixMatchApp::TrackSearchTable2(const std::vector<Track>& library, float x, float width)
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
                activeTrackId = track.id;
                mainSearchBuffer[0] = '\0';
                showMainLibary = false;
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

void MixMatchApp::TrackSearchTable(const std::vector<Track>& library, float x, float width)
{
    // Header
    ImGui::Separator();
    ImGui::SetCursorScreenPos(ImVec2(x, ImGui::GetCursorScreenPos().y));

    ImGui::PushFont(Ui::Text::SmallFont);
    ImGui::PushStyleColor(ImGuiCol_Text, Ui::Colour::TEXT_GRAY);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));

    ImGui::Text("Sort:");

    ImGui::SameLine();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::AlignTextToFramePadding();

    static int trackSortIndex = 0;
    const char* trackSortOptions[] = { "Artist", "Title", "Label", "BPM"};

    // TODO: Create custom combo pop up 

    if (ImGui::BeginCombo(
        "##TrackSortCombo", 
        trackSortOptions[trackSortIndex],
        ImGuiComboFlags_WidthFitPreview |
        ImGuiComboFlags_NoArrowButton))
    {
        for (int i = 0; i < IM_ARRAYSIZE(trackSortOptions); i++)
        {
            bool isSelected = (trackSortIndex == i);

            if (ImGui::Selectable(trackSortOptions[i], isSelected))
                trackSortIndex = i;

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
    ImGui::PopStyleVar(1);
    mainSearchSort = static_cast<LibraryManager::TrackSort>(trackSortIndex);

    ImGui::SameLine();
    ImGui::Text("| View: All");

    ImGui::PopFont();
    ImGui::PopStyleColor(4);

    ImGui::PushFont(Ui::Text::DefaultFont);

    // Table

    //ImGui::SetCursorScreenPos(ImVec2(x, ImGui::GetCursorScreenPos().y));

    ImGui::PushStyleVar(
        ImGuiStyleVar_CellPadding,
        ImVec2(0, 0) //ImVec2(24, 6)
    );

    if (ImGui::BeginTable(
        "TrackTable",
        7,
        ImGuiTableFlags_SizingFixedFit | 
        ImGuiTableFlags_BordersInnerV))    //ImVec2(width, 0))
    {
        ImGui::TableSetupColumn("##Edit", ImGuiTableColumnFlags_WidthFixed); //ImGui::GetFrameHeight());
        ImGui::TableSetupColumn("Artist", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("BPM", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Rating", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("##Remove", ImGuiTableColumnFlags_WidthFixed);

        for (const Track& track : library)
        {
            std::string rowId = std::to_string(track.id);
         

            // Invisible selector
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_NavHighlight, IM_COL32(0, 0, 0, 0));
                
            if (ImGui::Selectable(
                ("##TableSelect" + rowId).c_str(),
                false,
                ImGuiSelectableFlags_SpanAllColumns))
            {
                activeTrackId = track.id;
                showMainLibary = false;
            }

            bool hovered = ImGui::IsItemHovered();

            ImGui::PopStyleColor(3);
            ImGui::SameLine();


            // Draw box
            /*
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
                    (uint8_t)((hovered ? Ui::Colour::ALPHA_HOVER : Ui::Colour::ALPHA_BG) * 255)),
                6.0f);
            */

            // Edit button
            ImGui::TableSetColumnIndex(0);

            float rowHeight = ImGui::GetTextLineHeight();
            ImVec2 buttonSize(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());

            //ImVec2 buttonSize(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
            ImGui::Dummy(buttonSize);

            ImVec2 pos = ImGui::GetItemRectMin();
            pos.x -= 12.0f;

            if (hovered)
            {
                ImGui::SetCursorScreenPos(pos);

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));  

                if (ImGui::Button(Ui::Text::ICON_EDIT, buttonSize))
                    showEditTrackWindow = true;

                ImGui::PopStyleVar();
            }
            
            //ImVec2 cellMin = ImGui::GetCursorScreenPos();
            //ImVec2 buttonSize = ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());

            //cellMin.x -= ImGui::GetStyle().CellPadding.x * 0.5f;
            //cellMin.y -= ImGui::GetStyle().CellPadding.y;

            //ImGui::SetCursorScreenPos(cellMin);

            //if (hovered)
            //{
            //    // data = here?
            //    if (ImGui::Button(Ui::Text::ICON_ADD, buttonSize))
            //    {
            //        showEditTrackWindow = true;
            //    }
            //}
            //else
            //{
            //    ImGui::Dummy(buttonSize);
            //}
            

            // Artist
            ImGui::TableSetColumnIndex(1);
            TextUtil::DrawCenterJustifyTableText(track.artist);
            ImGui::TextUnformatted(track.artist.c_str());

            // Title
            ImGui::TableSetColumnIndex(2);
            TextUtil::DrawCenterJustifyTableText(track.title);
            ImGui::TextUnformatted(track.title.c_str());

            // Label
            ImGui::TableSetColumnIndex(3);
            TextUtil::DrawCenterJustifyTableText(track.label);
            ImGui::TextUnformatted(track.label.c_str());

            // BPM Sine
            ImGui::TableSetColumnIndex(4);
            if (track.bpm > 0.f)
            {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
                ImGui::PushStyleColor(
                    ImGuiCol_PlotLines,
                    IM_COL32_WHITE); //ColourUtil::RgbToU32(Ui::RGB_DEFAULT, 255)
                    
                static float sine[1024];
                static float beat[1024];

                float cycles = 4.f;
                float bpm = (float)track.bpm ;
                float speed = bpm / (60.f * cycles);
                float time = (float)ImGui::GetTime();

                float phase = time * speed * 2.f * IM_PI;

                for (int i = 0; i < IM_ARRAYSIZE(sine); ++i)
                {
                    float x = (float)i / (IM_ARRAYSIZE(sine) - 1);

                    float value = sinf(x * cycles * 2.f * IM_PI - phase);

                    sine[i] = value;
                    // TOOO: Add saw wave
                    beat[i] = (value > 0.95f) ? 1.f : -1.f;
                }

                ImGui::PlotLines(("##BpmPlot" + rowId).c_str(), sine, IM_ARRAYSIZE(sine), 0, nullptr, -1.0f, 1.0f);
                //ImGui::PlotLines(("##BpmPlot" + rowId).c_str(), beat, IM_ARRAYSIZE(beat), 0, nullptr, -1.0f, 1.0f);

                ImGui::PopStyleColor(2);
            }

            ImGui::TableSetColumnIndex(5);
            TextUtil::DrawCenterJustifyTableText(std::to_string(track.rating));
            ImGui::TextUnformatted(std::to_string(track.rating).c_str());
        }
        ImGui::EndTable();
    }
    ImGui::PopFont();
    ImGui::PopStyleVar();
}

void MixMatchApp::DrawActiveTrackDisplay(int id)
{
    if (id == -1)
        return;

    const Track* activeTrack = manager.getTrackForDisplay(id);

    if (activeTrack == nullptr)
        return;

    // Main Display //
    std::string artistAndTitle = activeTrack->artist + " - " + activeTrack->title;
    ImGui::Text(artistAndTitle.c_str());

    if (!activeTrack->label.empty())
        ImGui::Text(activeTrack->label.c_str());

    if (activeTrack->bpm > 0.f)
        ImGui::Text(std::to_string(activeTrack->bpm).c_str());
    
    DrawStarRating(activeTrack->rating);

    // Edit active track button
    if (ImGui::Button(Ui::Text::ICON_EDIT) && !showEditTrackWindow)
    {
        editData = {};

        editData.id = activeTrackId;
        strcpy_s(editData.artist, activeTrack->artist.c_str());
        strcpy_s(editData.title, activeTrack->title.c_str());
        strcpy_s(editData.label, activeTrack->label.c_str());
        editData.bpm = activeTrack->bpm;
        editData.rating = activeTrack->rating;
        editData.colour = ColourUtil::RgbToImVec4(activeTrack->colour);

        showAddTrackWindow = false;
        showEditTrackWindow = true;
    }

    // Remove active track button
    if (ImGui::Button(Ui::Text::ICON_DELETE))
    {
        deleteTrackData.trackId = activeTrackId;
        deleteTrackData.mixId = -1;
        showDeleteTrackWindow = true;
    }

    // Mixes //
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

                ImGui::TableNextRow();

                // Track Select
                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(mixTrack->id);
                if (ImGui::Selectable(
                    "##Mix Display Track Select",
                    false,
                    ImGuiSelectableFlags_SpanAllColumns |
                    ImGuiSelectableFlags_AllowOverlap))
                {
                    activeTrackId = mixTrack->id;
                }
                ImGui::PopID();
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
                ImGui::PushID(mixTrack->id);
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
                ImGui::PopID();

                // Mix rating
                ImGui::TableSetColumnIndex(1);
                int rating = mix.rating;
                if (ImGui::DragInt(("##Mix Rating" + rowId).c_str(), &rating, 0.25f, 0, 5))
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
                ImGui::PushID(mixTrack->id);
                if (ImGui::Button(Ui::Text::ICON_CLOSE))
                {
                    deleteMixData.trackId = activeTrackId;
                    deleteMixData.mixId = mix.id;
                    showDeleteMixWindow = true;                    
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleVar(1);
    }

    // Theads

    // Sets
}

