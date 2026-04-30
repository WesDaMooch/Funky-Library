#include "..\include\mixMatchApp.hpp"

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

    // Refresh libary button
    if (ImGui::Button(Ui::Text::ICON_REFRESH, mainSearchBarButtonSize))
        manager.refresh();

    if (mainSearchBuffer[0] != '\0')
        showMainLibary = true;

    // Main libary
    //showMainLibary = true;
    if (showMainLibary)
    {
        const std::vector<Track> mainLibrary = 
            manager.searchAndSort(mainSearchBuffer, mainSearchSort);

        TrackSearchTable2(mainLibrary, tableX, tableWidth);
    }

    ImGui::End();


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


    // Add new track window
    if (showAddTrackWindow)
        InputTrackDataWindow(addData, TrackInputMode::ADD);

    // Edit track window
    //if (showEditTrackWindow)
    //    InputTrackDataWindow(editData, TrackInputMode::EDIT);


    // Remove track window
    if (showRemoveTrackWindow)
    {
        int testId = 0;
        RemoveTrackWindow(testId);
    }


    /*
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 0));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("Search", &open,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse);

    ImVec2 searchBarWindowSize = ImGui::GetWindowSize();

    // Search bar

    ImGui::PushItemWidth(-100);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(25, 5));

    ImGui::InputText("##MainSeachInput", mainSearchBuffer, sizeof(mainSearchBuffer));

    if (ImGui::IsItemActive())
    {
        showSearchResultWindow = true;
        // TODO: get this working
        
        //if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        //    showSearchResultWindow = false;
        
    }

    ImGui::PopStyleVar();
    ImGui::PopItemWidth();

    // Search icon
    // TODO: put to the right of search bar
    if (!ImGui::IsItemActive())
    {
        ImVec2 p = ImGui::GetItemRectMin();
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(p.x + 8, p.y + 8), //TODO: put in middle of bar
            IM_COL32(255, 255, 255, 255),
            UI::ICON_SEARCH
        );
    }

    ImGui::SameLine();

    // Add new track button
    if (ImGui::Button(UI::ICON_ADD) && !showAddTrackWindow)
    {
        addData = {};

        showEditTrackWindow = false;
        showAddTrackWindow = true;
    }
        
    ImGui::SameLine();

    // Refresh catalogue button
    if (ImGui::Button(UI::ICON_REFRESH))
        manager.refresh();

    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        !ImGui::IsAnyItemActive())
        showSearchResultWindow = false;

    ImGui::End();

    // Add new track window
    if (showAddTrackWindow)
        TrackInputWindow(addData, TrackInputMode::ADD);
    
    // Search result window
    // TODO: put this inside of the seach bar window
    if (showSearchResultWindow)
    {
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 0));
        ImGui::Begin("Search Result", &showSearchResultWindow,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoFocusOnAppearing);

        // Text search logic
        std::string mainSearchString = StringUtil::toLower(mainSearchBuffer);
        auto mainSearchWords = StringUtil::splitWords(mainSearchString);

        for (const Track& t : manager.getCatalogueForDisplay())
        {
            if (mainSearchBuffer[0] != '\0')
            {
                std::string artist = StringUtil::toLower(t.artist);
                std::string track = StringUtil::toLower(t.title);

                bool match = false;
                for (const auto& word : mainSearchWords)
                {
                    if (artist.find(word) != std::string::npos ||
                        track.find(word) != std::string::npos)
                    {
                        match = true;
                        break;
                    }
                }

                if (!match)
                    continue;
            }

            // Track select button
            std::string trackDisplayText = t.artist + " - " + t.title;

            if (ImGui::Button(trackDisplayText.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0)))
            {
                activeTrackId = t.id;
                mainSearchBuffer[0] = '\0';
                showSearchResultWindow = false;
            }
        }
        ImGui::End();
    }
    */

    /*              Viewer / Editor             */
    /*
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 0)); //io.DisplaySize.y - searchWindowHeight
    //ImGui::SetNextWindowPos(ImVec2(0, searchWindowHeight));
    ImGui::Begin("Track Viewer And Editor", &open,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    bool deletActiveTrack = false;

    const Track* activeTrack = manager.getTrackForDisplay(activeTrackId);
    if (activeTrack == nullptr)
        activeTrackId = -1;

    // Display active track
    if (activeTrackId > -1)
    {
        // Banner
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();

        ImVec2 bannerPos(
            cursorPos.x,
            cursorPos.y + style.WindowPadding.y);

        ImVec2 bannerSize(ImGui::GetContentRegionAvail().x, 200);
        float bannerPadding = style.FramePadding.x;

        ImDrawList* draw = ImGui::GetWindowDrawList();

        ImU32 bannerColour = IM_COL32(
            activeTrack->colour[0],
            activeTrack->colour[1],
            activeTrack->colour[2],
            (int)(UI::ALPHA_DEFAULT * 255.0f)
        );
     
        // Background
        draw->AddRectFilled(
            bannerPos,
            ImVec2(bannerPos.x + bannerSize.x, bannerPos.y + bannerSize.y),
            bannerColour,
            0.f
        );

        // Colour
        //float colourButtonSize = bannerSize.y * 0.75f;
        //float colourButtonPad = (bannerSize.y - colourButtonSize) * 0.5f;

        //ImVec2 colourButtonPos(
        //    bannerPos.x + colourButtonPad,
        //    bannerPos.y + colourButtonPad
        //);

        //// Shared text X start
        //float textX =
        //    colourButtonPos.x +
        //    colourButtonSize +
        //    colourButtonPad;

        //// Artist & track name
        //std::string trackLabel = activeTrack->artist + " - " + activeTrack->title;
        //ImVec2 trackLabelSize = ImGui::CalcTextSize(trackLabel.c_str());

        //draw->AddText(
        //    ImVec2(
        //        textX,
        //        bannerPos.y + bannerSize.y * 0.33f - trackLabelSize.y * 0.5f),
        //    IM_COL32(255, 255, 255, 255),
        //    trackLabel.c_str()
        //);


        //// Info
        //std::string infoLabel = "Label";
        //ImVec2 infoSize = ImGui::CalcTextSize(infoLabel.c_str());

        //draw->AddText(
        //    ImVec2(
        //        textX,
        //        bannerPos.y + bannerSize.y * 0.66f - infoSize.y * 0.5f),
        //    IM_COL32(255, 255, 255, 255),
        //    infoLabel.c_str()
        //);

        std::string trackLabel = activeTrack->artist + " - " + activeTrack->title;
        ImGui::Text(trackLabel.c_str());
        ImGui::Text(activeTrack->label.c_str());

        // Edit active track button
        if (ImGui::Button(UI::ICON_EDIT) && !showEditTrackWindow)
        {
            editData = {};
            
            // TODO: could get activeTrack here

            editData.id = activeTrackId;
            strcpy_s(editData.artist, activeTrack->artist.c_str());
            strcpy_s(editData.title, activeTrack->title.c_str());
            strcpy_s(editData.label , activeTrack->label.c_str());
            editData.bpm = activeTrack->bpm;
            editData.colour = RgbToImVec4(activeTrack->colour);

            showAddTrackWindow = false;
            showEditTrackWindow = true;
        }
            

        // Remove active track button
        ImVec2 buttonText = ImGui::CalcTextSize(UI::ICON_DELETE);
        ImVec2 buttonSize(
            buttonText.x + style.FramePadding.x * 2.f,
            buttonText.y + style.FramePadding.y * 2.f);

        ImGui::SetCursorScreenPos(
            ImVec2(
                bannerPos.x + bannerSize.x - buttonSize.x - bannerPadding,
                bannerPos.y + bannerSize.y - buttonSize.y - bannerPadding));

        if (ImGui::Button(UI::ICON_DELETE))
            deletActiveTrack = true;

        ImGui::SetCursorScreenPos(bannerPos);
        ImGui::Dummy(bannerSize);


        // Mixes

        // Add new mix button
        if (ImGui::Button(UI::ICON_ADD))
        {
            showAddMixWindow = true;
        }
            

        // Add new mix search window
        if (showAddMixWindow)
        {
            // Add new mix search
            ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 0));
            ImGui::SetNextWindowPos(ImVec2(0, 200));
            ImGui::Begin("Mix Search", &showAddMixWindow,
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoCollapse);

            ImGui::InputText("##MixSearch", mixSearchBuffer, sizeof(mixSearchBuffer));

            std::string search = StringUtil::toLower(mixSearchBuffer);
            auto words = StringUtil::splitWords(search);

            for (const auto& t : manager.getCatalogueForDisplay())
            {
                if (activeTrackId == t.id)
                    continue;

                bool mixFound = false;
                for (int mixId : activeTrack->mixIds)
                {
                    if (mixId == t.id)
                    {
                        mixFound = true;
                        break;
                    }
                }

                if (mixFound)
                    continue;

                if (mixSearchBuffer[0] != '\0')
                {
                    std::string artist = StringUtil::toLower(t.artist);
                    std::string track = StringUtil::toLower(t.title);

                    bool match = false;
                    for (const auto& word : words)
                    {
                        if (artist.find(word) != std::string::npos ||
                            track.find(word) != std::string::npos)
                        {
                            match = true;
                            break;
                        }
                    }

                    if (!match)
                        continue;
                }

                std::string trackDisplayText = t.artist + " - " + t.title;
                // Add mix select button
                if (ImGui::Button(trackDisplayText.c_str(), ImVec2(300, 0)))
                {
                    manager.addMix(activeTrackId, t.id);
                    showAddMixWindow = false;
                }

            }

            if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
                !ImGui::IsAnyItemActive())
                showAddMixWindow = false;

            ImGui::End();
        }

    }

    // Display mixes
    if (activeTrackId > -1 && activeTrack != nullptr)
    {
        int removeMixId = -1;

        ImGuiStyle& style = ImGui::GetStyle();

        for (int mixId : activeTrack->mixIds)
        {
            const Track* mixTrack = manager.getTrackForDisplay(mixId);

            if (mixTrack == nullptr)
                continue;

            std::string mixTrackLabel = mixTrack->artist + " - " + mixTrack->title;

            ImGui::PushID(mixTrack->id);

            // right button size
            float removeWidth = ImGui::GetFrameHeight();

            // remaining width for track button
            float trackWidth =
                ImGui::GetContentRegionAvail().x
                - removeWidth
                - style.ItemSpacing.x;

            // left align text inside buttons
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f));

            if (ImGui::Button(mixTrackLabel.c_str(), ImVec2(trackWidth, 0)))
                activeTrackId = mixTrack->id;

            ImGui::PopStyleVar();

            ImGui::SameLine();

            if (ImGui::Button(UI::ICON_REMOVE, ImVec2(removeWidth, 0)))
                removeMixId = mixId;

            ImGui::PopID();
        }

        if (removeMixId != -1)
            manager.removeMix(activeTrackId, removeMixId);
    }

    if (showEditTrackWindow)
        TrackInputWindow(editData, TrackInputMode::EDIT);
    

    if (deletActiveTrack)
    {
        // TODO: Open Yes / No window
        manager.removeTrack(activeTrackId);
        activeTrack = nullptr;
        activeTrackId = -1;
    }
    ImGui::End();
    */
}


void MixMatchApp::RemoveTrackWindow(int id)
{
    const Track* track = manager.getTrackForDisplay(id);

    if (track == nullptr)
        return;

    // tint
    ImGui::PushStyleColor(
        ImGuiCol_TitleBg,
        ImVec4(
            track->colour[0] * 0.15f,
            track->colour[1] * 0.15f,
            track->colour[2] * 0.15f,
            1.f));

    ImGui::PushStyleColor(
        ImGuiCol_TitleBgActive,
        ImVec4(
            track->colour[0] * 0.33f,
            track->colour[1] * 0.33f,
            track->colour[2] * 0.33f,
            1.f));

    ImGui::Begin("Remove Track", &showRemoveTrackWindow, ImGuiWindowFlags_NoCollapse);

    ImGui::PopStyleColor(2);

    std::string s = "Perminatly remove " + track->artist + " - " + track->title;
    ImGui::Text(s.c_str());

    // TODO: Push some button styles

    if (ImGui::Button("Yes"))
    {
        //manager.removeTrack(id);
        showRemoveTrackWindow = false;
    }

    ImGui::SameLine();

    if (ImGui::Button("No"))
        showRemoveTrackWindow = false;

    ImGui::End();
}

void MixMatchApp::InputTrackDataWindow(InputData& data, TrackInputMode mode)
{
    bool* open = mode == ADD ? &showAddTrackWindow : &showEditTrackWindow;
    const char* title = mode == ADD ? "Add New Track" : "Edit Track";

    // Close edit window if active track changes
    if (mode == EDIT && data.id != activeTrackId)
        *open = false;

    ImGui::PushStyleColor(ImGuiCol_TitleBg, ColourUtil::TintVec4(data.colour, Ui::Colour::TINT_BG));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ColourUtil::TintVec4(data.colour, Ui::Colour::TINT_HOVER));

    //ImGui::SetNextWindowBgAlpha(0.f);
    ImGui::Begin(
        title, 
        open, 
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize);

    // Glass
    //DrawBetterGlass(ImGui::GetCurrentWindow());

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
        data.rating = std::clamp(data.rating, 0, 10);

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

                if (data.result == LibraryManager::TrackValidationResult::Valid)
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

            if (data.result == LibraryManager::TrackValidationResult::Valid)
            {
                data = {};
                *open = false;
            }
        }
        ImGui::PopStyleColor(3);

        ImGui::TableSetColumnIndex(1);
        if (data.result !=
            LibraryManager::TrackValidationResult::None ||
            LibraryManager::TrackValidationResult::Valid)
        {
            std::string waringText = "";

            switch (data.result)
            {
            case LibraryManager::TrackValidationResult::MissingArtist:
                waringText = "Missing Artist Entry";
                break;
            case LibraryManager::TrackValidationResult::MissingTitle:
                waringText = "Missing Title Entry";
                break;
            case LibraryManager::TrackValidationResult::TrackNotFound:
                waringText = "Track Not In Library";
                break;
            case LibraryManager::TrackValidationResult::DuplicateTrack:
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

            TextUtil::centerJustifyTableText(trackText);
            ImGui::TextUnformatted(trackText.c_str());
            
    
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
            TextUtil::centerJustifyTableText(track.artist);
            ImGui::TextUnformatted(track.artist.c_str());

            // Title
            ImGui::TableSetColumnIndex(2);
            TextUtil::centerJustifyTableText(track.title);
            ImGui::TextUnformatted(track.title.c_str());

            // Label
            ImGui::TableSetColumnIndex(3);
            TextUtil::centerJustifyTableText(track.label);
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
            TextUtil::centerJustifyTableText(std::to_string(track.rating));
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

    const Track* track = manager.getTrackForDisplay(id);

    if (track == nullptr)
        return;

    // Main display
    std::string artistAndTitle = track->artist + " - " + track->title;
    ImGui::Text(artistAndTitle.c_str());

    if (!track->label.empty())
        ImGui::Text(track->label.c_str());

    if (track->bpm > 0.f)
        ImGui::Text(std::to_string(track->bpm).c_str());
    
    ImGui::Text(std::to_string(track->rating).c_str());

    // Mixes with
    ImGui::SeparatorText("Mixes");

    // Add new mix button
    if (ImGui::Button(showAddMixWindow ? Ui::Text::ICON_REMOVE : Ui::Text::ICON_ADD))
        showAddMixWindow = !showAddMixWindow;

    if (showAddMixWindow)
    {
        ImGui::SameLine();
        ImGui::InputText("##Mix Search Input", mixSearchBuffer, sizeof(mixSearchBuffer));

        const std::vector<Track> mixSearchLibrary =
            manager.searchAndSort(mixSearchBuffer, LibraryManager::TrackSort::Artist);

        for (const Track& mixSearch : mixSearchLibrary)
        {
            if (mixSearch.id == activeTrackId)
                continue;

            std::string mixSearchArtistAndTitle = mixSearch.artist + " - " + mixSearch.title;
            ImGui::Text(mixSearchArtistAndTitle.c_str());
        }

    }
    else {
        for (const int mixId : track->mixIds)
        {
            const Track* mixTrack = manager.getTrackForDisplay(mixId);

            if (mixTrack == nullptr)
                continue;

            std::string mixArtistAndTitle = mixTrack->artist + " - " + mixTrack->title;
            ImGui::Text(mixArtistAndTitle.c_str());
        }
    }




    /*
    if (ImGui::BeginTable(
        "Mix Table",
        1))    //ImVec2(width, 0))
    {

        ImGui::TableSetupColumn("Mix Name");

        for (const int mixId : track->mixIds)
        {
            const Track* mixTrack = manager.getTrackForDisplay(mixId);

            if (mixTrack == nullptr)
                continue;

            ImGui::TableNextRow();

            std::string mixArtistAndTitle = mixTrack->artist + " - " + mixTrack->title;
            ImGui::Text(mixArtistAndTitle.c_str());

        }
        ImGui::EndTable();
    }
    */

    // Theads

    // Sets
}