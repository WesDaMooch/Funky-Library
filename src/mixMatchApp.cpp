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
    style.GrabRounding = 6.f;
    style.ScrollbarRounding = 6.f;

    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 0));
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ImGui::Begin("##Main Search", &open,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse);

    float frameHeight = ImGui::GetFrameHeight();
    ImVec2 mainSearchBarButtonSize = ImVec2(frameHeight, frameHeight);

    // Show main libary button
    //if (ImGui::Button(UI::ICON_SEARCH, mainSearchBarButtonSize))
    //    showMainLibary = !showMainLibary;

    // Add new track button
    if (ImGui::Button(Ui::ICON_ADD, mainSearchBarButtonSize) && !showAddTrackWindow)
    {
        addData = {};
        showEditTrackWindow = false;
        showAddTrackWindow = true;
    }

    ImGui::SameLine();
        
    //ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::PushItemWidth(-frameHeight - style.ItemSpacing.x);
    ImGui::InputText("##MainSeachInput", mainSearchBuffer, sizeof(mainSearchBuffer));
    ImGui::PopItemWidth();
    

    ImGui::SameLine();

    // Refresh libary button
    if (ImGui::Button(Ui::ICON_REFRESH, mainSearchBarButtonSize))
        manager.refresh();

    // Main libary
    showMainLibary = true;
    if (showMainLibary)
    {
        ImGui::Separator();

        const std::vector<Track> l = manager.searchAndSort(mainSearchBuffer, LibraryManager::Sort::Artist);
        TrackCardTable(l);
    }

    ImGui::End();

    // Add new track window
    if (showAddTrackWindow)
        TrackInputWindow(addData, TrackInputMode::ADD);

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


void MixMatchApp::TrackInputWindow(InputData& data, TrackInputMode mode)
{
    bool* open = mode == ADD ? &showAddTrackWindow : &showEditTrackWindow;
    const char* title = mode == ADD ? "Add New Track" : "Edit Track";

    // Close edit window if active track changes
    if (mode == EDIT && data.id != activeTrackId)
        *open = false;

    ImGui::Begin(title, open, ImGuiWindowFlags_NoCollapse);

    if (ImGui::BeginTable("track_table", 3, ImGuiTableFlags_SizingFixedFit))
    {
        float fieldWidth = 500.0f;
        float fieldHeight = ImGui::GetFrameHeight();

        ImGui::TableSetupColumn("Text", ImGuiTableColumnFlags_WidthFixed);
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

        ImGui::EndTable();
    }

    if (ImGui::Button(Ui::ICON_SAVE))
    {
        Track t;
        t.artist = data.artist;
        t.title = data.title;
        t.label = data.label;
        t.bpm = data.bpm;
        t.colour = ColourUtil::ImVec4ToRgb(data.colour);

        bool success = false;

        if (mode == ADD)
        {
            success = manager.addTrack(t);

            if (success)
            {
                if (!manager.getCatalogueForDisplay().empty())
                    activeTrackId = manager.getCatalogueForDisplay().back().id;
            }
        }
        else if (mode == EDIT)
        {
            t.id = activeTrackId;

            success = manager.editTrack(t);
        }

        if (success)
        {
            data = {};
            *open = false;
        }
    }

    // TODO: Add warning text, put saveButtonPressCount in inputData??
    //ImGui::SameLine();
    //ImGui::Text(warningText.c_str());

    ImGui::End();
}

void MixMatchApp::TrackCardTable(const std::vector<Track>& searchLibrary)
{
    ImGui::PushStyleVar(
        ImGuiStyleVar_CellPadding,
        ImVec2(24, 6)
    );

    if (ImGui::BeginTable(
        "TrackTable",
        4,
        ImGuiTableFlags_SizingFixedFit | 
        //ImGuiTableFlags_Hideable |
        //TODO: Setup ImGuiTableFlags_Sortable |
        ImGuiTableFlags_BordersInnerV))
    {
        
        ImGui::TableSetupColumn(
            "Artist",
            ImGuiTableColumnFlags_WidthFixed
        );

        ImGui::TableSetupColumn(
            "Title",
            ImGuiTableColumnFlags_WidthFixed
        );
  
        ImGui::TableSetupColumn(
            "Label",
            ImGuiTableColumnFlags_WidthFixed
        );

        ImGui::TableSetupColumn(
            "BPM",
            ImGuiTableColumnFlags_WidthStretch
        );

        for (const Track& track : searchLibrary)
        {
            ImGui::TableNextRow();

            std::string rowId = std::to_string(track.id);

            ImGui::TableSetColumnIndex(0);;

            // Clear selector
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));

            if (ImGui::Selectable(
                ("##TableSelect" + rowId).c_str(),
                false,
                ImGuiSelectableFlags_SpanAllColumns))
            {
                activeTrackId = track.id;
                showMainLibary = false;
            }

            ImGui::PopStyleColor(2);

            ImGui::SameLine();

            //if (ImGui::IsItemHovered())
            //{
            //    float rectPaddingY = 6.f;
            //    ImVec2 rectMin = ImGui::GetItemRectMin();
            //    ImVec2 rectMax = ImGui::GetItemRectMax();

            //    rectMin.y -= rectPaddingY;
            //    rectMax.y += rectPaddingY;

            //    ImGui::GetWindowDrawList()->AddRectFilled(
            //        rectMin,
            //        rectMax,
            //        ColourUtil::RgbToU32(track.colour, (uint8_t)(Ui::ALPHA_BG * 255)),
            //        6.0f
            //    );
            //}

            bool hovered = ImGui::IsItemHovered();

            float rectPaddingY = 6.f;
            ImVec2 rectMin = ImGui::GetItemRectMin();
            ImVec2 rectMax = ImGui::GetItemRectMax();

            rectMin.y -= rectPaddingY;
            rectMax.y += rectPaddingY;

            ImGui::GetWindowDrawList()->AddRectFilled(
                rectMin,
                rectMax,
                ColourUtil::RgbToU32(
                    track.colour, 
                    (uint8_t)((hovered ? Ui::ALPHA_HOVER : Ui::ALPHA_BG) * 255)
                ),
                6.0f
            );

            // Artist
            ImGui::TableSetColumnIndex(0);
            TextUtil::centerJustifyTableText(track.artist);
            ImGui::TextUnformatted(track.artist.c_str());

            // Title
            ImGui::TableSetColumnIndex(1);
            TextUtil::centerJustifyTableText(track.title);
            ImGui::TextUnformatted(track.title.c_str());

            // Label
            ImGui::TableSetColumnIndex(2);
            TextUtil::centerJustifyTableText(track.label);
            ImGui::TextUnformatted(track.label.c_str());
            //if (!track.label.empty())
            //    ImGui::Text(track.label.c_str()); //TODO: use unformatedText?

            // BPM Sine
            ImGui::TableSetColumnIndex(3);
            if (track.bpm > 0.f)
            {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
                ImGui::PushStyleColor(
                    ImGuiCol_PlotLines,
                    ColourUtil::RgbToU32(Ui::RGB_DEFAULT, 255)
                );

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
                    beat[i] = (value > 0.95f);
                }

                ImGui::PlotLines(("##BpmPlot" + rowId).c_str(), sine, IM_ARRAYSIZE(sine), 0, nullptr, -1.0f, 1.0f);
                //ImGui::PlotLines(("##BpmPlot" + rowId).c_str(), beat, IM_ARRAYSIZE(beat), 0, nullptr, -1.0f, 1.0f);

                ImGui::PopStyleColor(2);
            }
        }
        ImGui::PopStyleVar();
        ImGui::EndTable();
    }
    //ImGui::PopStyleVar();

}



// TODO: remove
void MixMatchApp::TrackInfoCard(int id)
{
    const Track* track = manager.getTrackForDisplay(id);

    if (track == nullptr)
        return;

    float cardPaddingY = 0.f;
    ImVec2 cardSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight() + cardPaddingY);

    std::string buttonId = "##cardButton" + std::to_string(id);
    if (ImGui::InvisibleButton(buttonId.c_str(), cardSize))
    {
        activeTrackId = id;
        showMainLibary = false;
    }

    float rectRoundness = 6.f;
    ImVec2 rectMin = ImGui::GetItemRectMin();
    ImVec2 rectMax = ImGui::GetItemRectMax();

    uint8_t alpha = (uint8_t)(Ui::ALPHA_BG * 255.f);

    if (ImGui::IsItemHovered())
        alpha = (uint8_t)(Ui::ALPHA_HOVER * 255.f);

    // Background
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(
        rectMin,
        rectMax,
        ColourUtil::RgbToU32(track->colour, alpha),
        rectRoundness
    );

    // Border
    draw->AddRect(
        rectMin,
        rectMax,
        ColourUtil::RgbToU32(Ui::RGB_DEFAULT, alpha),
        rectRoundness,
        0,
        1.f
    );
   
    float titleTextX = rectMin.x + 24;
    float labelTextX = rectMin.x + 320;
    float textY = rectMin.y + (cardPaddingY * 0.5f) + Ui::GLYPH_OFFSET;

    std::string titleText = track->artist + " - " + track->title;
    draw->AddText(ImVec2(titleTextX, textY), IM_COL32_WHITE, titleText.c_str());

    if (track->label != "")
    {
        std::string labelText = " | " + track->label;
        draw->AddText(ImVec2(labelTextX, textY), IM_COL32_WHITE, labelText.c_str());
    }
    
            


    // TODO: plots with same should be alined regardless of plot size
    /*
    ImGui::SameLine();
    if (track->bpm > 0.f)
    {
        static float samples[100];

        float bpm = static_cast<float>(track->bpm);
        float beatsPerSec = bpm / 120.0f;

        float time = static_cast<float>(ImGui::GetTime());

        for (int i = 0; i < IM_ARRAYSIZE(samples); ++i)
        {
            float x = (float)i / (IM_ARRAYSIZE(samples) - 1);

            samples[i] = sinf((x * 6.0f * IM_PI) - (time * beatsPerSec * 2.0f * IM_PI));
        }

        std::string plotLabel = "##BpmSine" + std::to_string(id);
        ImGui::PlotLines(plotLabel.c_str(), samples, IM_ARRAYSIZE(samples), 0, nullptr, -1.0f, 1.0f);
    }
    */
}