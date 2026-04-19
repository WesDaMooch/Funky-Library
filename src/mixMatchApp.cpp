#include "..\include\mixMatchApp.hpp"

MixMatchApp::MixMatchApp()
{
	manager.load();
}

void MixMatchApp::TrackInputWindow(TrackInputMode mode, Track* track)
{
    bool* open = mode == ADD ? &showAddTrackWindow : &showEditTrackWindow;
    const char* title = mode == ADD ? "Add New Track" : "Edit Track";

    ImGui::Begin(title, open, ImGuiWindowFlags_NoCollapse);
    
    ImGui::InputText("Artist", inputArtistBuffer, sizeof(inputArtistBuffer));
    ImGui::InputText("Track", inputTrackBuffer, sizeof(inputTrackBuffer));
    
    if (ImGui::Button(ICON_SAVE))
    {
        Track t;
        t.artistName = inputArtistBuffer;
        t.trackName = inputTrackBuffer;

        bool success = false;

        if (mode == ADD)
        {
            success = manager.addTrack(t);

            if (success)
            {
                if (!manager.getCatalogue().empty())
                    activeTrackId = manager.getCatalogue().back().id;

                //showAddTrackWindow = false;
            }
        }
        else if (mode == EDIT && track != nullptr)
        {
            t.id = track->id;

            //trackEdited = manager.editTrack(t);

            //if (success)
            //{
            //    showEditTrackWindow = false;
            //}
        }

        if (success)
        {
            inputArtistBuffer[0] = '\0';
            inputTrackBuffer[0] = '\0';
            *open = false;
        }
    }
    
    //ImGui::SameLine();
    
    //ImGui::Text(warningText.c_str());

    ImGui::End();
}

void MixMatchApp::RunFrame()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	

    // When edit track pressed
    //strcpy_s(inputArtistBuffer, track->artistName.c_str());
    //strcpy_s(inputTrackBuffer, track->trackName.c_str());
    //showEditTrackWindow = true;

    /*              Main Search             */

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
        /*
        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            showSearchResultWindow = false;
        */
    }

    ImGui::PopStyleVar();
    ImGui::PopItemWidth();

    // Search icon
    if (!ImGui::IsItemActive())
    {
        ImVec2 p = ImGui::GetItemRectMin();
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(p.x + 8, p.y + 8), //TODO: put in middle of bar
            IM_COL32(255, 255, 255, 255),
            ICON_SEARCH
        );
    }

    ImGui::SameLine();

    // Add new track button
    if (ImGui::Button(ICON_ADD))
        showAddTrackWindow = true;

    ImGui::SameLine();

    // Refresh catalogue button
    if (ImGui::Button(ICON_REFRESH))
        manager.refresh();

    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        !ImGui::IsAnyItemActive())
        showSearchResultWindow = false;

    ImGui::End();

    // Add new track window
    if (showAddTrackWindow)
    {
        inputArtistBuffer[0] = '\0';
        inputTrackBuffer[0] = '\0';
        TrackInputWindow(TrackInputMode::ADD);
    }
        

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
        std::string mainSearchString = manager.toLower(mainSearchBuffer);
        auto mainSearchWords = manager.splitWords(mainSearchString);

        for (const Track& t : manager.getCatalogue())
        {
            if (mainSearchBuffer[0] != '\0')
            {
                std::string artist = manager.toLower(t.artistName);
                std::string track = manager.toLower(t.trackName);

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
            std::string trackDisplayText = t.artistName + " - " + t.trackName;

            if (ImGui::Button(trackDisplayText.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0)))
            {
                activeTrackId = t.id;
                mainSearchBuffer[0] = '\0';
                showSearchResultWindow = false;
            }
        }
        ImGui::End();
    }


    /*              Viewer / Editor             */

    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 0)); //io.DisplaySize.y - searchWindowHeight
    //ImGui::SetNextWindowPos(ImVec2(0, searchWindowHeight));
    ImGui::Begin("Track Viewer And Editor", &open,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    bool removeActiveTrack = false;

    const Track* activeTrack = manager.getTrack(activeTrackId);
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

        //ImVec4 colour = ImVec4(activeTrack->colour);

        ImVec4 color = ImVec4(
            DEFAULT_COL_RGB[0] / 255.f,
            DEFAULT_COL_RGB[1] / 255.f,
            DEFAULT_COL_RGB[2] / 255.f,
            1.f
        );

        // Background
        draw->AddRectFilled(
            bannerPos,
            ImVec2(bannerPos.x + bannerSize.x, bannerPos.y + bannerSize.y),
            ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, ALPHA_DEFAULT)),
            0.f
        );

        // Colour
        float colourButtonSize = bannerSize.y * 0.75f;
        float colourButtonPad = (bannerSize.y - colourButtonSize) * 0.5f;

        ImVec2 colourButtonPos(
            bannerPos.x + colourButtonPad,
            bannerPos.y + colourButtonPad
        );

        ImGui::SetCursorScreenPos(colourButtonPos);

        if (ImGui::ColorButton(
            "##BannerColor",
            color,
            ImGuiColorEditFlags_NoTooltip |
            ImGuiColorEditFlags_NoBorder,
            ImVec2(colourButtonSize, colourButtonSize)))
        {
            ImGui::OpenPopup("BannerColorPicker");
        }

        if (ImGui::BeginPopup("BannerColorPicker"))
        {
            ImGui::ColorPicker3("##ColourPicker",
                (float*)&color,
                ImGuiColorEditFlags_NoSmallPreview |
                ImGuiColorEditFlags_NoLabel |
                ImGuiColorEditFlags_NoSidePreview
            );
            ImGui::EndPopup();
        }



        // Shared text X start
        float textX =
            colourButtonPos.x +
            colourButtonSize +
            colourButtonPad;

        // Artist & track name
        std::string trackLabel = activeTrack->artistName + " - " + activeTrack->trackName;
        ImVec2 trackLabelSize = ImGui::CalcTextSize(trackLabel.c_str());

        draw->AddText(
            ImVec2(
                textX,
                bannerPos.y + bannerSize.y * 0.33f - trackLabelSize.y * 0.5f),
            IM_COL32(255, 255, 255, 255),
            trackLabel.c_str()
        );


        // Info
        std::string infoLabel = "Label";
        ImVec2 infoSize = ImGui::CalcTextSize(infoLabel.c_str());

        draw->AddText(
            ImVec2(
                textX,
                bannerPos.y + bannerSize.y * 0.66f - infoSize.y * 0.5f),
            IM_COL32(255, 255, 255, 255),
            infoLabel.c_str()
        );

        // Remove active track button
        ImVec2 buttonText = ImGui::CalcTextSize(ICON_DELETE);
        ImVec2 buttonSize(
            buttonText.x + style.FramePadding.x * 2.f,
            buttonText.y + style.FramePadding.y * 2.f);

        ImGui::SetCursorScreenPos(
            ImVec2(
                bannerPos.x + bannerSize.x - buttonSize.x - bannerPadding,
                bannerPos.y + bannerSize.y - buttonSize.y - bannerPadding));

        if (ImGui::Button(ICON_DELETE))
            removeActiveTrack = true;

        ImGui::SetCursorScreenPos(bannerPos);
        ImGui::Dummy(bannerSize);


        // Mixes

        // Add new mix button
        if (ImGui::Button(ICON_ADD))
            showAddMixWindow = true;

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

            std::string search = manager.toLower(mixSearchBuffer);
            auto words = manager.splitWords(search);

            for (const auto& t : manager.getCatalogue())
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
                    std::string artist = manager.toLower(t.artistName);
                    std::string track = manager.toLower(t.trackName);

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

                std::string trackDisplayText = t.artistName + " - " + t.trackName;
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
            const Track* mixTrack = manager.getTrack(mixId);

            if (mixTrack == nullptr)
                continue;

            std::string mixTrackLabel = mixTrack->artistName + " - " + mixTrack->trackName;

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

            if (ImGui::Button(ICON_REMOVE, ImVec2(removeWidth, 0)))
                removeMixId = mixId;

            ImGui::PopID();
        }

        if (removeMixId != -1)
            manager.removeMix(activeTrackId, removeMixId);
    }

    if (removeActiveTrack == true)
    {
        // TODO: Open Yes / No window
        manager.removeTrack(activeTrackId);
        activeTrack = nullptr;
        activeTrackId = -1;
    }
    ImGui::End();



}