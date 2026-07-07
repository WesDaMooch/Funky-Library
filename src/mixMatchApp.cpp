#include "..\include\mixMatchApp.hpp"

// Make it a thing, then make it look good!


// TODO:
// Add back button on the right of the main search bar

// Use context menus (right click) to view mix note?
// Add format
// Add mix & track note
// Add pitch adjust

// Add spring graph (maybe called constallation map or something)
// Add tap tempo ??

MixMatchApp::MixMatchApp()
{
	manager.load();
}

void MixMatchApp::runFrame2()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags windowFlags =
        //ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    if (ImGui::Begin("Funky Libaray", &open, windowFlags))
    {

        // Left panel
        {
            ImGuiChildFlags childFlags =
                ImGuiChildFlags_ResizeX;

            ImGui::BeginChild("Left Panel", ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, ImGui::GetContentRegionAvail().y), childFlags, ImGuiWindowFlags_None);

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::InputText("##MainSeachInput", mainSearchBuffer, sizeof(mainSearchBuffer));

            const auto searchResultLibrary = manager.searchAndSort(mainSearchBuffer, mainSearchSort);

            ImGui::BeginChild("Library List", ImVec2(0, 0));

            for (const auto& track : searchResultLibrary)
            {
                ImGui::PushID(track.id);
                ImGui::Selectable(TextUtil::FormartTitle(track.artist, track.title).c_str());
                ImGui::PopID();
            }

            ImGui::EndChild();
            ImGui::EndChild();
        }

        ImGui::SameLine();

        // Right panel
        {
            ImGuiChildFlags childFlags =
                ImGuiChildFlags_Borders;


            
            ImGui::BeginChild("ChildR", ImVec2(0, ImGui::GetContentRegionAvail().y), childFlags, ImGuiWindowFlags_None);

            ImGui::Text("Yo");

            ImGui::EndChild();
        }

        ImGui::End();
    }
}



void MixMatchApp::runFrame()
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

    // Map button 
    if (ImGui::Button(Ui::Text::ICON_HALFSTAR, mainSearchBarButtonSize))
    {
        showTrackMap = !showTrackMap;

        if (showTrackMap)
            rebakeTrackMap(manager.getCatalogueForDisplay());
    }

    if (mainSearchBuffer[0] != '\0')
        showMainLibary = true;

    // Main library (search results)
    if (showMainLibary)
    {
        const std::vector<Track> mainLibrary = manager.searchAndSort(mainSearchBuffer, mainSearchSort);
        trackSearchTable(mainLibrary, tableX, tableWidth);
    }

    // Add track popup window
    setModalPopupPosAndSize("Add Track");

    //ImGui::PushStyleColor(ImGuiCol_TitleBg, ColourUtil::TintVec4(addTrackData.colour, Ui::Colour::TINT_BG));
    //ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ColourUtil::TintVec4(addTrackData.colour, Ui::Colour::TINT_HOVER));

    if (ImGui::BeginPopupModal("Add Track", &showAddTrackPopup, ImGuiWindowFlags_AlwaysAutoResize))
    {
        inputTrackDataPopup(addTrackData, TrackInputMode::ADD);
        ImGui::EndPopup();
    }

    //ImGui::PopStyleColor(2);

    ImGui::End();

#pragma endregion MainSearchBar


#pragma region TrackMap
    
    if (showTrackMap)
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

        // Draw map
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mousePos = { io.MousePos.x, io.MousePos.y };

        int hoveredNodeId = -1;
        ImVec2 hoveredNodePos;

        // Mouse zoom
        float wheel = io.MouseWheel;

        float oldZoom = camera.zoom;
        ImVec2 worldUnderMouse = { (mousePos.x - camera.pos.x) / oldZoom, (mousePos.y - camera.pos.y) / oldZoom };

        camera.setZoom(wheel, io.DeltaTime);

        camera.pos.x = mousePos.x - worldUnderMouse.x * camera.zoom;
        camera.pos.y = mousePos.y - worldUnderMouse.y * camera.zoom;


        // Mouse drag
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            // Save mouse position
            mousePosOnLeftClick = mousePos;
            cameraPosOnLeftClick = camera.pos;
        }

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            float dx = mousePos.x - mousePosOnLeftClick.x;
            float dy = mousePos.y - mousePosOnLeftClick.y;

            camera.pos = ImVec2(cameraPosOnLeftClick.x + dx, cameraPosOnLeftClick.y + dy);
        }

        // Draw edges
        float radius = nodeBaseRadius * camera.zoom;
        float lineWidth = 0.5f * camera.zoom;
        float arrowLength = 4.0f * camera.zoom;
        float arrowWidth = 4.0f * camera.zoom;

        for (const TrackMap::Edge& e : map.edges)
        {
            const TrackMap::Node& a = map.nodes[map.nodeIndex.at(e.A_id)];
            const TrackMap::Node& b = map.nodes[map.nodeIndex.at(e.B_id)];

            ImVec2 start = camera.toScreenPos(a.pos.x, a.pos.y);
            ImVec2 end = camera.toScreenPos(b.pos.x, b.pos.y);
            ImVec2 dir = { end.x - start.x,end.y - start.y };
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

            if (len > 0.0f)
            {
                // Gap between node and endge
                dir.x /= len;
                dir.y /= len;

                float gap = (4.0f * camera.zoom) + radius;

                start.x += dir.x * gap;
                start.y += dir.y * gap;

                end.x -= dir.x * gap;
                end.y -= dir.y * gap;

                // Draw arrow
                ImVec2 perp = { -dir.y,dir.x };

                if (e.A_direction)
                {
                    ImVec2 arrowLine1 = {
                        start.x + dir.x * arrowLength + perp.x * arrowWidth,
                        start.y + dir.y * arrowLength + perp.y * arrowWidth
                    };

                    ImVec2 arrowA2 = {
                        start.x + dir.x * arrowLength - perp.x * arrowWidth,
                        start.y + dir.y * arrowLength - perp.y * arrowWidth
                    };

                    drawList->AddLine(start, arrowLine1, IM_COL32_WHITE, lineWidth);
                    drawList->AddLine(start, arrowA2, IM_COL32_WHITE, lineWidth);
                }

                if (e.B_direction)
                {
                    ImVec2 arrowLine1 = {
                        end.x - dir.x * arrowLength + perp.x * arrowWidth,
                        end.y - dir.y * arrowLength + perp.y * arrowWidth
                    };

                    ImVec2 arrowLine2 = {
                        end.x - dir.x * arrowLength - perp.x * arrowWidth,
                        end.y - dir.y * arrowLength - perp.y * arrowWidth
                    };

                    drawList->AddLine(end, arrowLine1, IM_COL32_WHITE, lineWidth);
                    drawList->AddLine(end, arrowLine2, IM_COL32_WHITE, lineWidth);
                }

            }

            // Edge line
            drawList->AddLine(
                start,
                end,
                IM_COL32_WHITE,
                lineWidth
            );
        }

        // Draw nodes
        for (const TrackMap::Node& n : map.nodes)
        {
            ImVec2 nodePos = camera.toScreenPos(n.pos.x, n.pos.y);

            // Node hovered
            float dx = mousePos.x - nodePos.x;
            float dy = mousePos.y - nodePos.y;

            if ((dx * dx + dy * dy) <= (radius * radius))
            {
                hoveredNodeId = n.id;
                hoveredNodePos.x = (float)n.pos.x;
                hoveredNodePos.y = (float)n.pos.y;
            }
                

            drawList->AddCircleFilled(
                nodePos,
                radius,
                IM_COL32_WHITE, //n.col
                16
            );


            // Outline
            drawList->AddCircle(
                nodePos,
                radius,
                IM_COL32_WHITE,
                16,
                0.5f * camera.zoom
            );

        }

        for (const auto& group : map.groups)
        {
            continue;

            ImVec2 pos = camera.toScreenPos((float)group.pos.x, (float)group.pos.y);
            drawList->AddCircle(
                pos,
                (float)group.radius * camera.zoom,
                IM_COL32_WHITE,
                16,
                1.0f
            );
        }


        // Debug
        // Centre
        //drawList->AddCircleFilled(camera.ToScreenPos(0, 0), 10.0f, IM_COL32_WHITE, 8);
        //ImGui::Text(std::to_string(map.nodes.size()).c_str());


        // Hovered node text
        if (hoveredNodeId > -1)
        {
            const Track* t = manager.getTrackForDisplay(hoveredNodeId);

            if (t != nullptr)
            {             
                drawList->AddText(
                    camera.toScreenPos(hoveredNodePos.x, hoveredNodePos.y),
                    IM_COL32_WHITE,
                    t->artist.c_str()
                );

                float textHeight = ImGui::GetTextLineHeight() / camera.zoom;

                drawList->AddText(
                    camera.toScreenPos(hoveredNodePos.x, hoveredNodePos.y + textHeight),
                    IM_COL32_WHITE,
                    t->title.c_str()
                );
            }
        }
        
        ImGui::End();
    }
    

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

        setClearSelectableStyle();

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
            editTrackData.colour = ColourUtil::RgbToVec4(activeTrack->colour);

            showEditTrackPopup = true;
            ImGui::OpenPopup("Edit Track");
        }

        ImGui::PopStyleColor(3);
        ImGui::SameLine();

        ImGui::SetCursorPos(editSelPos);
        setTextColourOnHover(ImGui::IsItemHovered());
        ImGui::Text(Ui::Text::ICON_EDIT);
        ImGui::PopStyleColor();

        // Label
        if (!activeTrack->label.empty())
        {   
            std::string label = activeTrack->label;
            ImVec2 selectSize = ImGui::CalcTextSize(label.c_str());
            ImVec2 selectPos = ImGui::GetCursorPos();

            setClearSelectableStyle();

            if (ImGui::Selectable("##Label Select", false, NULL, selectSize))
            {
                activeLabel = label;
                mainPage = MainPage::LABEL;
            }

            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            ImGui::SetCursorPos(selectPos);
            setTextColourOnHover(ImGui::IsItemHovered());
            ImGui::Text(label.c_str());
            ImGui::PopStyleColor();
        }

        // Release
        if (!activeTrack->release.empty())
        {
            std::string release = activeTrack->release;
            ImVec2 selectSize = ImGui::CalcTextSize(release.c_str());
            ImVec2 selectPos = ImGui::GetCursorPos();

            setClearSelectableStyle();

            if (ImGui::Selectable("##Release Select", false, NULL, selectSize))
            {
                activeRelease = release;
                mainPage = MainPage::RELEASE;
            }

            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            ImGui::SetCursorPos(selectPos);
            setTextColourOnHover(ImGui::IsItemHovered());

            ImGui::Text(release.c_str());

            if (!activeTrack->position.empty())
                ImGui::Text(("Track: " + activeTrack->position).c_str());

            ImGui::PopStyleColor();
        }
            
        int newRating = starRating(activeTrack->rating);
        if (newRating > -1)
        {
            Track editedTrack = *activeTrack;
            editedTrack.rating = newRating;
            manager.editTrack(editedTrack);
        }

        // Track edit popup
        setModalPopupPosAndSize("Edit Track");

        if (ImGui::BeginPopupModal("Edit Track", &showEditTrackPopup, ImGuiWindowFlags_AlwaysAutoResize))
        {
            inputTrackDataPopup(editTrackData, TrackInputMode::EDIT);

            if (!showEditTrackPopup)
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
                    drawCenterJustifyTableText(artistAndTitle);

                    // Label
                    ImGui::TableSetColumnIndex(1);
                    drawCenterJustifyTableText(mixSearch.label);

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
            displayMixTable(activeTrack->mix);            
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

        ImGui::PushFont(Ui::Text::BigFont);
        ImGui::Text(activeLabel.c_str());
        ImGui::PopFont();

        ImGui::Separator();

        displayLabelTable(labelLibrary);
        break;
    }
    case MainPage::RELEASE:
    {
        const std::vector<const Track*> releaseLibrary = manager.getRelease(activeRelease);

        if (releaseLibrary.empty())
            break;

        ImGui::PushFont(Ui::Text::BigFont);
        ImGui::Text(activeRelease.c_str());
        ImGui::PopFont();

        ImGui::Separator();

        displayReleaseTable(releaseLibrary);
        break;
    }
    default: break;
    }

    ImGui::End();
}

void MixMatchApp::deleteConformationPopup(DeleteData& data, DeleteConformationMode mode)
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

void MixMatchApp::inputTrackDataPopup(InputData& data, TrackInputMode mode)
{
    bool saveButtonPressed = false;

    const float fieldWidth = 500.0f;
    float fieldHeight = ImGui::GetFrameHeight();

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

    ImGui::PushStyleColor(ImGuiCol_Tab, ColourUtil::TintVec4(data.colour, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_TabHovered, ColourUtil::TintVec4(data.colour, 0.66f));
    ImGui::PushStyleColor(ImGuiCol_TabSelected, ColourUtil::TintVec4(data.colour, 0.8f));

    if (ImGui::BeginTabBar("Input Data Tabs", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("Track"))
        {
            if (ImGui::BeginTable("InputTrackTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
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
                ImGui::Text("Rating");
                ImGui::TableSetColumnIndex(1);
                int newRating = starRating(data.rating);

                if (newRating > -1)
                    data.rating = newRating;

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                ImGui::EndTable();
            }

            inputDataSaveButton(data, mode);

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Release"))
        {
            if (ImGui::BeginTable("InputReleaseTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableSetupColumn("Text", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthFixed, 362.f); // Find the correct size from first tab table

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Label");
                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(fieldWidth);
                ImGui::InputText("##Label", data.label, sizeof(data.label));

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("LP / EP");
                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(fieldWidth);
                ImGui::InputText("##Release", data.release, sizeof(data.release));

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Position");
                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(fieldWidth);
                ImGui::InputText("##Position", data.position, sizeof(data.position));

                ImGui::EndTable();
            }

            inputDataSaveButton(data, mode);

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Misc"))
        {
            if (ImGui::BeginTable("InputMiscTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                ImGui::TableSetupColumn("Text", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthStretch);

                // Notes

                // Format?

                ImGui::EndTable();
            }

            inputDataSaveButton(data, mode);

            ImGui::EndTabItem();
        }

        if (mode == TrackInputMode::EDIT && ImGui::BeginTabItem("Delete"))
        {

            float availX = ImGui::GetContentRegionAvail().x;

            // Text
            const char* line1 = "Permanently Delete";
            float line1Width = ImGui::CalcTextSize(line1).x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availX - line1Width) * 0.5f);
            ImGui::Text(line1);

            std::string line2 = TextUtil::FormartTitle(data.artist, data.title) + "?";
            float line2Width = ImGui::CalcTextSize(line2.c_str()).x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availX - line2Width) * 0.5f);
            ImGui::Text(line2.c_str());

            // Yes / No select
            const char* yesText = "Yes";
            const char* noText = "No";

            ImVec2 yesSize = ImGui::CalcTextSize(yesText);
            ImVec2 noSize = ImGui::CalcTextSize(noText);

            float spacing = 20.0f;
            float totalWidth = yesSize.x + noSize.x + spacing;

            // center the group
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availX - totalWidth) * 0.5f);

            // Yes
            ImVec2 yesPos = ImGui::GetCursorPos();
            setClearSelectableStyle();

            if (ImGui::Selectable("##Del Yes Sel", false, 0, yesSize))
            {
                manager.removeTrack(data.id);
                ImGui::CloseCurrentPopup();
            }

            ImGui::PopStyleColor(3);

            ImGui::SetCursorPos(yesPos);
            setTextColourOnHover(ImGui::IsItemHovered());
            ImGui::Text(yesText);
            ImGui::PopStyleColor();

            ImGui::SameLine(0, spacing);

            // No
            ImVec2 noPos = ImGui::GetCursorPos();
            setClearSelectableStyle();

            if (ImGui::Selectable("No", false, 0, noSize))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::PopStyleColor(3);

            ImGui::SetCursorPos(noPos);
            setTextColourOnHover(ImGui::IsItemHovered());
            ImGui::Text(noText);
            ImGui::PopStyleColor();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
        ImGui::PopStyleColor(3);
    }
}

void MixMatchApp::trackSearchTable(const std::vector<Track>& library, float x, float width)
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

            drawCenterJustifyTableText(trackText);
    
            //ImGui::TableSetColumnIndex(2);

        }
        ImGui::EndTable();
    }

    ImGui::PopStyleVar(1);
}

void MixMatchApp::displayLabelTable(std::vector<const Track*> trackList)
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
        float ratingSize = ImGui::CalcTextSize(Ui::Text::ICON_STAR).x * 5;
        ImGui::TableSetupColumn("Artist & Title", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Release", ImGuiTableColumnFlags_WidthStretch, 1.f);
        // Tags
        //ImGui::TableSetupColumn("BPM", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Rating", ImGuiTableColumnFlags_WidthFixed, ratingSize + tablePaddingX);

        ImGuiTable* table = ImGui::GetCurrentTable();
        float rowWidth = table->WorkRect.Max.x - table->WorkRect.Min.x;
        float rowHeight = ImGui::GetTextLineHeight();

        for (const Track* track : trackList)
        {
            if (track == nullptr)
                continue;

            ImGui::TableNextRow();
            ImGui::PushID(track->id);

            // Background
            ImGui::TableSetColumnIndex(0);
            ImVec2 pos = ImGui::GetCursorPos();

            bool hovered = drawTableBg(drawList, rowWidth, rowHeight, track->colour, track->bpm);

            ImU32 lineColour = ColourUtil::RgbToU32(track->colour, hovered ? 88 : 64);

            // Artist & title
            ImGui::SetCursorPos(pos);

            if (drawTableButton(TextUtil::FormartTitle(track->artist, track->title), false))
            {
                activeTrackId = track->id;
                mainPage = MainPage::ACTIVE_TRACK;
            }

            drawTableDividerLine(table, 0, drawList, rowHeight);

            // Release
            ImGui::TableSetColumnIndex(1);

            std::string release = track->release;
            if (drawTableButton(release))
            {
                activeRelease = release;
                mainPage = MainPage::RELEASE;
            }

            drawTableDividerLine(table, 1, drawList, rowHeight);

            // Rating
            ImGui::TableSetColumnIndex(2);

            float colWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + colWidth - ratingSize);
            
            int newRating = starRating(track->rating);
            if (newRating > -1)
            {
                Track editedTrack = *track;
                editedTrack.rating = newRating;
                manager.editTrack(editedTrack);
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
}


void MixMatchApp::displayReleaseTable(std::vector<const Track*> trackList)
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
        float ratingSize = ImGui::CalcTextSize(Ui::Text::ICON_STAR).x * 5;
        ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Artist & Title", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 1.f);
        // Tags
        //ImGui::TableSetupColumn("BPM", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Rating", ImGuiTableColumnFlags_WidthFixed, ratingSize + tablePaddingX);

        ImGuiTable* table = ImGui::GetCurrentTable();
        float rowWidth = table->WorkRect.Max.x - table->WorkRect.Min.x;
        float rowHeight = ImGui::GetTextLineHeight();

        for (const Track* track : trackList)
        {
            if (track == nullptr)
                continue;

            ImGui::TableNextRow();
            ImGui::PushID(track->id);

            // Background
            ImGui::TableSetColumnIndex(0);
            ImVec2 pos = ImGui::GetCursorPos();

            bool hovered = drawTableBg(drawList, rowWidth, rowHeight, track->colour, track->bpm);

            ImU32 lineColour = ColourUtil::RgbToU32(track->colour, hovered ? 88 : 64);

            // Position
            ImGui::SetCursorPos(pos);

            std::string position = track->position;

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.66f, 0.66f, 0.66f, 1.f));
            ImGui::Text(padTableText(position.empty() ? "-" : position, 2, 2).c_str());
            ImGui::PopStyleColor();

            drawTableDividerLine(table, 0, drawList, rowHeight);

            // Artist & title
            ImGui::TableSetColumnIndex(1);

            if (drawTableButton(TextUtil::FormartTitle(track->artist, track->title), false))
            {
                activeTrackId = track->id;
                mainPage = MainPage::ACTIVE_TRACK;
            }

            drawTableDividerLine(table, 1, drawList, rowHeight);

            // Label
            ImGui::TableSetColumnIndex(2);

            std::string label = track->label;

            if (drawTableButton(label))
            {
                activeLabel = label;
                mainPage = MainPage::LABEL;
            }

            drawTableDividerLine(table, 2, drawList, rowHeight);

            // Rating
            ImGui::TableSetColumnIndex(3);

            float colWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + colWidth - ratingSize);
            
            int newRating = starRating(track->rating);
            if (newRating > -1)
            {
                Track editedTrack = *track;
                editedTrack.rating = newRating;
                manager.editTrack(editedTrack);
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleVar();
}


void MixMatchApp::displayMixTable(const std::vector<Mix>& mixList)
{
    if (mixList.empty())
        return;

    static float digitWidth = ImGui::CalcTextSize("0").x; // TODO: move this somewhere else?
    static float iconWidth = ImGui::CalcTextSize(Ui::Text::ICON_SYNCALT).x;

    bool editMixNoteButtonPressed = false;
    bool deleteMixButtonPressed = false;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Sort setup

    // Convert mixList to trackList
    std::vector<MixTableItem> items;
    items.reserve(mixList.size());

    for (const Mix m : mixList)
    {
        const Track* t = manager.getTrackForDisplay(m.id);
        
        if (t == nullptr)
            continue;

        MixTableItem item;
        item.id = m.id;
        item.direction = static_cast<int>(m.direction);
        item.mixRating = m.rating;
        item.pitch = m.pitch;
        item.mixNote = m.note;
        item.artistAndTitle = TextUtil::FormartTitle(t->artist, t->title);
        item.label = t->label;
        item.release = t->release;
        item.bpm = t->bpm;
        item.trackRating = t->rating;
        item.colour = t->colour;

        items.emplace_back(item);
    }


    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImVec4(0, 0, 0, 0));

    if (ImGui::BeginTable(
        "Mix Table",
        9,
        ImGuiTableFlags_Sortable |
        ImGuiTableFlags_SortMulti |
        ImGuiTableFlags_SizingStretchProp |
        ImGuiTableFlags_ContextMenuInBody |
        ImGuiTableFlags_Hideable))
    {
        std::vector<float> rowPos;
        rowPos.reserve(mixList.size() + 1); // Header + track rows

        float ratingSize = ImGui::CalcTextSize(Ui::Text::ICON_STAR).x * 5;
        float removeSize = ImGui::CalcTextSize(padTableText(Ui::Text::ICON_REMOVE).c_str()).x;

        ImGui::TableSetupColumn("Direction", 
            ImGuiTableColumnFlags_WidthFixed |
            ImGuiTableColumnFlags_NoHide |
            ImGuiTableColumnFlags_NoHeaderWidth,
            iconWidth + tablePaddingX,
            (ImGuiID)MixTableColumn::Direction);

        ImGui::TableSetupColumn("Mix Rating", 
            ImGuiTableColumnFlags_WidthFixed,
            digitWidth,
            (ImGuiID)MixTableColumn::MixRating);

        ImGui::TableSetupColumn("Pitch Adjust",
            ImGuiTableColumnFlags_WidthFixed,
            (ImGuiID)MixTableColumn::PitchAdjust);
           
        ImGui::TableSetupColumn("Track", 
            ImGuiTableColumnFlags_WidthStretch | 
            ImGuiTableColumnFlags_NoHide |
            ImGuiTableColumnFlags_NoHeaderWidth,
             1.75f,
            (ImGuiID)MixTableColumn::Track);

        ImGui::TableSetupColumn("Label", 
            ImGuiTableColumnFlags_WidthStretch |
            ImGuiTableColumnFlags_NoHeaderWidth,
            1.0f,
            (ImGuiID)MixTableColumn::Label);

        ImGui::TableSetupColumn("Release", 
            ImGuiTableColumnFlags_WidthStretch |
            ImGuiTableColumnFlags_NoHeaderWidth,
            1.0f,
            (ImGuiID)MixTableColumn::Release);

        ImGui::TableSetupColumn("Mix Note",
            ImGuiTableColumnFlags_WidthStretch |
            ImGuiTableColumnFlags_NoHeaderWidth,
            1.0f,
            (ImGuiID)MixTableColumn::MixNote);

        //ImGui::TableSetupColumn("BPM", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableSetupColumn("Track Rating", 
            ImGuiTableColumnFlags_WidthFixed |
            ImGuiTableColumnFlags_NoHeaderWidth,
            ratingSize + tablePaddingX,
            (ImGuiID)MixTableColumn::TrackRating);

        ImGui::TableSetupColumn("Remove", 
            ImGuiTableColumnFlags_WidthFixed | 
            ImGuiTableColumnFlags_NoHide |
            ImGuiTableColumnFlags_NoSort,
            //ImGuiTableColumnFlags_NoHeaderLabel, 
            removeSize,
            (ImGuiID)MixTableColumn::Remove);

        ImGuiTable* table = ImGui::GetCurrentTable();
        float rowWidth = table->WorkRect.Max.x - table->WorkRect.Min.x;
        float rowHeight = ImGui::GetTextLineHeight();

        // TODO: Add sort function to custom header

        ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();

        if (sortSpecs && sortSpecs->SpecsCount > 0)
        {
            const ImGuiTableColumnSortSpecs& spec = sortSpecs->Specs[0];

            bool ascending = spec.SortDirection == ImGuiSortDirection_Ascending;

            switch ((MixTableColumn)spec.ColumnUserID)
            {
            case MixTableColumn::Direction:
                std::sort(
                    items.begin(),
                    items.end(),
                    [ascending](const MixTableItem& a, const MixTableItem& b)
                    {
                        return ascending
                            ? a.direction < b.direction
                            : a.direction > b.direction;
                    });
                break;

            case MixTableColumn::MixRating:
                std::sort(
                    items.begin(),
                    items.end(),
                    [ascending](const MixTableItem& a, const MixTableItem& b)
                    {
                        return ascending
                            ? a.mixRating < b.mixRating
                            : a.mixRating > b.mixRating;
                    });
                break;

            case MixTableColumn::PitchAdjust:
                std::sort(
                    items.begin(),
                    items.end(),
                    [ascending](const MixTableItem& a, const MixTableItem& b)
                    {
                        return ascending
                            ? a.pitch < b.pitch
                            : a.pitch > b.pitch;
                    });
                break;

            case MixTableColumn::Track:
                std::sort(
                    items.begin(),
                    items.end(),
                    [ascending](const MixTableItem& a, const MixTableItem& b)
                    {
                        return ascending
                            ? a.artistAndTitle < b.artistAndTitle
                            : a.artistAndTitle > b.artistAndTitle;
                    });
                break;

            case MixTableColumn::Label:
                std::sort(
                    items.begin(),
                    items.end(),
                    [ascending](const MixTableItem& a, const MixTableItem& b)
                    {
                        return ascending
                            ? a.label < b.label
                            : a.label > b.label;
                    });
                break;

            case MixTableColumn::Release:
                std::sort(
                    items.begin(),
                    items.end(),
                    [ascending](const MixTableItem& a, const MixTableItem& b)
                    {
                        return ascending
                            ? a.release < b.release
                            : a.release > b.release;
                    });
                break;

            case MixTableColumn::TrackRating:
                std::sort(
                    items.begin(),
                    items.end(),
                    [ascending](const MixTableItem& a, const MixTableItem& b)
                    {
                        return ascending
                            ? a.trackRating < b.trackRating
                            : a.trackRating > b.trackRating;
                    });
                break;
            }

            sortSpecs->SpecsDirty = false;
        }


        /*
        if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs())
            if (sortSpecs->SpecsDirty)
            {
                //MyItem::SortWithSortSpecs(sort_specs, items.Data, items.Size);

                const ImGuiTableColumnSortSpecs& spec = sortSpecs->Specs[0];
                bool ascending = spec.SortDirection == ImGuiSortDirection_Ascending;

                switch ((MixTableColumn)spec.ColumnUserID)
                {
                case MixTableColumn::Track:
                    std::sort(
                        items.begin(),
                        items.end(),
                        [ascending](const MixTableItem& a, const MixTableItem& b)
                        {
                            return ascending
                                ? a.artistAndTitle < b.artistAndTitle
                                : a.artistAndTitle > b.artistAndTitle;
                        });
                    break;

                case MixTableColumn::Label:
                    std::sort(
                        items.begin(),
                        items.end(),
                        [ascending](const MixTableItem& a, const MixTableItem& b)
                        {
                            return ascending
                                ? a.label < b.label
                                : a.label > b.label;
                        });
                    break;

                case MixTableColumn::Release:
                    std::sort(
                        items.begin(),
                        items.end(),
                        [ascending](const MixTableItem& a, const MixTableItem& b)
                        {
                            return ascending
                                ? a.release < b.release
                                : a.release > b.release;
                        });
                    break;
                }

                sortSpecs->SpecsDirty = false;
            }
        }
        */

        ImGui::TableHeadersRow();

        // Header
        /*
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(tablePaddingX, tablePaddingY * 0.5f));

        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        rowPos.emplace_back(table->RowPosY1);

        // Underline
        float headerHeight = ImGui::GetTextLineHeight() + (ImGui::GetStyle().CellPadding.y * 2.0f);
        float underlineY = ImGui::GetCursorScreenPos().y + headerHeight + 4; //table->RowPosY1 + headerHeight;

        drawList->AddLine(
            ImVec2(table->WorkRect.Min.x, underlineY),
            ImVec2(table->WorkRect.Max.x, underlineY),
            IM_COL32(85, 85, 85, 128),
            1.0f);

        // Header columns
        ImGui::TableSetColumnIndex(0);
        //ImGui::TextUnformatted("Mix"); // Direction

        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(""); // Mix Rating

        ImGui::TableSetColumnIndex(2);
        ImGui::TextUnformatted(""); // Pitch

        ImGui::TableSetColumnIndex(3);
        DrawCenterJustifyTableText("Track");

        ImGui::TableSetColumnIndex(4);
        DrawCenterJustifyTableText("Label");

        ImGui::TableSetColumnIndex(5);
        DrawCenterJustifyTableText("Release");

        ImGui::TableSetColumnIndex(6);
        DrawCenterJustifyTableText("Mix Note");

        ImGui::TableSetColumnIndex(7);
        DrawCenterJustifyTableText("Rating");

        ImGui::TableSetColumnIndex(8);
        ImGui::TextUnformatted(""); // Remove button

        ImGui::PopStyleVar();
        */


        // Track rows
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(tablePaddingX, tablePaddingY));

        for (const MixTableItem& item : items)
        {
            ImGui::TableNextRow();
            ImGui::PushID(item.id);
            rowPos.emplace_back(table->RowPosY1);

            // Background
            ImGui::TableSetColumnIndex(0);
            ImVec2 bgPos = ImGui::GetCursorPos();

            bool hovered = drawTableBg(drawList, rowWidth, rowHeight, item.colour, item.bpm);

            ImGui::SetCursorPos(bgPos);


            // Direction 
            std::string directionIcon = "";

            if (static_cast<MixDirection>(item.direction) == MixDirection::In)
                directionIcon = Ui::Text::ICON_ARROWFORWARD;
            else if (static_cast<MixDirection>(item.direction) == MixDirection::Out)
                directionIcon = Ui::Text::ICON_ARROWBACK;
            else
                directionIcon = Ui::Text::ICON_SYNCALT;

            // Old button
            setClearSelectableStyle();
            if (ImGui::Selectable("##Direction", false))
            {
                int direction = item.direction;

                direction++;

                if (direction >= static_cast<int>(MixDirection::NumDirections))
                    direction = 0;

                Mix editedMix = item.toMix();
                editedMix.direction = static_cast<MixDirection>(direction);

                manager.editMix(editedMix, activeTrackId);
            }
            ImGui::PopStyleColor(3);
            ImGui::SameLine();

            ImGui::TableSetColumnIndex(0);
            //SetTextColourOnHover(ImGui::IsItemHovered());
            setTextColourOnHover(false);
            drawCenterJustifyTableText(directionIcon.c_str());
            ImGui::PopStyleColor();


            /*
            if (DrawTableButton(directionIcon))
            {
                int direction = static_cast<int>(mix.direction);

                direction++;

                if (direction >= static_cast<int>(MixDirection::NumDirections))
                    direction = 0;

                Mix editedMix = mix;
                editedMix.direction = static_cast<MixDirection>(direction);

                manager.editMix(editedMix, activeTrackId);
            }
            */


            // Mix rating
            ImGui::TableSetColumnIndex(1);
            int rating = item.mixRating;

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));
            setTextColourOnHover(false);

            ImGui::SetNextItemWidth(digitWidth);
            if (ImGui::InputInt("##Mix Rating", &rating, 0.25f, 0, 5))
            {
                Mix editedMix = item.toMix();
                editedMix.rating = rating;
                manager.editMix(editedMix, activeTrackId);
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);


            // Pitch Adjust
            ImGui::TableSetColumnIndex(2);

            int pitch = item.pitch;

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));
            setTextColourOnHover(false);

            ImGui::SetNextItemWidth(ImGui::CalcTextSize((std::to_string(item.pitch) + "%").c_str()).x);
            if (ImGui::InputScalar("##Pitch Input", ImGuiDataType_S32, &pitch, nullptr, nullptr, "%d%%"))
            {
                Mix editedMix = item.toMix();
                editedMix.pitch = pitch;
                manager.editMix(editedMix, activeTrackId);
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);


            // Artist & title
            ImGui::TableSetColumnIndex(3);

            //std::string artistAndTitle = item.artistAndTitle;
            if (drawTableButton(item.artistAndTitle, false))
            {
                activeTrackId = item.id;
                mainPage = MainPage::ACTIVE_TRACK;
            }



            // Label
            ImGui::TableSetColumnIndex(4);
            std::string label = item.label;

            if (drawTableButton(label))
            {
                activeLabel = label;
                mainPage = MainPage::LABEL;
            }


            // Release
            ImGui::TableSetColumnIndex(5);
            std::string release = item.release;

            if (drawTableButton(release))
            {
                activeRelease = release;
                mainPage = MainPage::RELEASE;
            }


            // Mix Note
            ImGui::TableSetColumnIndex(6);

            if (drawTableButton(item.mixNote, true, true))
            {
                inputMixData = item.toMix();
                editMixNoteButtonPressed = true;
            }


            // Rating
            ImGui::TableSetColumnIndex(7);
            float colWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + colWidth - ratingSize);

            int newRating = starRating(item.trackRating);

            if (newRating > -1)
            {
                Track editedTrack = *manager.getTrack(item.id);
                editedTrack.rating = newRating;
                manager.editTrack(editedTrack);
            }


            // Remove
            ImGui::TableSetColumnIndex(8);

            if (drawTableButton(Ui::Text::ICON_CLOSE))
            {
                //deleteMixData.trackId = activeTrackId;
                //deleteMixData.mixId = mixTrack->id;
                //deleteMixData.label = artistAndTitle;

                deleteMixData = item.toMix();
                deleteMixButtonPressed = true;
            }

            ImGui::PopID();
        }







        /*
        for (const Mix& mix : mixList)
        {
            const Track* mixTrack = manager.getTrackForDisplay(mix.id);

            if (mixTrack == nullptr)
                continue;

            ImGui::TableNextRow();
            ImGui::PushID(mixTrack->id);
            rowPos.emplace_back(table->RowPosY1);

            // Background
            ImGui::TableSetColumnIndex(0);
            ImVec2 bgPos = ImGui::GetCursorPos();

            bool hovered = DrawTableBg(drawList, rowWidth, rowHeight, mixTrack->colour, mixTrack->bpm);

            ImGui::SetCursorPos(bgPos);


            // Direction 
            std::string directionIcon = "";

            if(mix.direction == MixDirection::In)
                directionIcon = Ui::Text::ICON_ARROWFORWARD;
            else if (mix.direction == MixDirection::Out)
                directionIcon = Ui::Text::ICON_ARROWBACK;
            else
                directionIcon = Ui::Text::ICON_SYNCALT;

            // Old button
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
            //SetTextColourOnHover(ImGui::IsItemHovered());
            SetTextColourOnHover(false);
            DrawCenterJustifyTableText(directionIcon.c_str());
            ImGui::PopStyleColor();
            

            //// new button
            if (DrawTableButton(directionIcon))
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
            
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));
            SetTextColourOnHover(false);

            ImGui::SetNextItemWidth(digitWidth);
            if (ImGui::InputInt("##Mix Rating", &rating, 0.25f, 0, 5))
            {
                Mix editedMix = mix;
                editedMix.rating = rating;
                manager.editMix(editedMix, activeTrackId);
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);

            
            // Pitch Adjust
            ImGui::TableSetColumnIndex(2);

            int pitch = mix.pitch;

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));
            SetTextColourOnHover(false);

            ImGui::SetNextItemWidth(ImGui::CalcTextSize((std::to_string(mix.pitch) + "%").c_str()).x);
            if (ImGui::InputScalar("##Pitch Input", ImGuiDataType_S32, &pitch, nullptr, nullptr, "%d%%"))
            {
                Mix editedMix = mix;
                editedMix.pitch = pitch;
                manager.editMix(editedMix, activeTrackId);
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);


            // Artist & title
            ImGui::TableSetColumnIndex(3);

            std::string artistAndTitle = TextUtil::FormartTitle(mixTrack->artist, mixTrack->title);

            if (DrawTableButton(artistAndTitle, false))
            {
                activeTrackId = mixTrack->id;
                mainPage = MainPage::ACTIVE_TRACK;
            }

          

            // Label
            ImGui::TableSetColumnIndex(4);
            std::string label = mixTrack->label;

            if (DrawTableButton(label))
            {
                activeLabel = label;
                mainPage = MainPage::LABEL;
            }


            // Release
            ImGui::TableSetColumnIndex(5);
            std::string release = mixTrack->release;

            if (DrawTableButton(release))
            {
                activeRelease = release;
                mainPage = MainPage::RELEASE;
            }


            // Note
            ImGui::TableSetColumnIndex(6);

            if (DrawTableButton(mix.note, true, true))
            {
                inputMixData = mix;
                editMixNoteButtonPressed = true;
            }


            // Rating
            ImGui::TableSetColumnIndex(7);
            float colWidth = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + colWidth - ratingSize);
            
            int newRating = StarRating(mixTrack->rating);

            if (newRating > -1)
            {
                Track editedTrack = *mixTrack;
                editedTrack.rating = newRating;
                manager.editTrack(editedTrack);
            }


            // Remove
            ImGui::TableSetColumnIndex(8);

            if (DrawTableButton(Ui::Text::ICON_CLOSE))
            {
                //deleteMixData.trackId = activeTrackId;
                //deleteMixData.mixId = mixTrack->id;
                //deleteMixData.label = artistAndTitle;

                deleteMixData = mix;
                deleteMixButtonPressed = true;
            }

            ImGui::PopID();
        }
        */

        drawTableBorderInnerV(table, drawList, mixList.size(), rowPos);

        ImGui::PopStyleVar();
        ImGui::EndTable();
    }
    ImGui::PopStyleColor();


    // Edit mix note popup
    if (editMixNoteButtonPressed)
    {
        showEditMixNotePopup = true;
        ImGui::OpenPopup("Edit Mix Note");
    }

    if (ImGui::BeginPopupModal("Edit Mix Note", &showEditMixNotePopup, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char buf[1024];

        if (ImGui::IsWindowAppearing())
            strncpy_s(buf, sizeof(buf), inputMixData.note.c_str(), _TRUNCATE);

        if (ImGui::InputTextMultiline("##Mix Note", buf, sizeof(buf)))
        {
            inputMixData.note = buf;
            manager.editMix(inputMixData, activeTrackId);
        }

        ImGui::EndPopup();
    }

    // Delete mix popup
    if (deleteMixButtonPressed)
    {
        showDeleteMixPopup = true;
        ImGui::OpenPopup("Delete Mix");
    }

    /*
    SetModalPopupPosAndSize(deleteMixData.label.c_str());
    if (ImGui::BeginPopupModal(deleteMixData.label.c_str(),
        &showDeleteMixPopup,
        ImGuiWindowFlags_AlwaysAutoResize))
        */

    //SetModalPopupPosAndSize("Delete Mix");

    if (ImGui::BeginPopupModal("Delete Mix",
        &showDeleteMixPopup, 
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        const Track* track = manager.getTrackForDisplay(deleteMixData.id);
        
        std::string text;

        if (track == nullptr)
            ImGui::CloseCurrentPopup();
        else
            text = TextUtil::FormartTitle(track->artist, track->title);

        float windowWidth = ImGui::GetWindowSize().x;
        float textWidth = ImGui::CalcTextSize(text.c_str()).x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::TextUnformatted(text.c_str());

        // Yes / No buttons
        float buttonWidth = 80.0f;
        float spacing = ImGui::GetStyle().ItemSpacing.x;

        float totalWidth = buttonWidth * 2.0f + spacing;

        float availableSpace = ImGui::GetContentRegionAvail().x;

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availableSpace - totalWidth) * 0.5f);

        // TODO: update stytle inline with DrawTableButton

        if (ImGui::Button("Yes"))
        {
            manager.removeMix(activeTrackId, deleteMixData.id);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("No"))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}


// Track map glue
void MixMatchApp::rebakeTrackMap(const std::vector<Track>& tracks)
{
    std::vector<TrackMap::Node> newNodes;
    newNodes.reserve(tracks.size());

    std::vector<TrackMap::Edge> newEdges;
    newEdges.reserve(tracks.size());

    for (const Track& t : tracks)
    {
        TrackMap::Node n{
            t.id,
            TrackMap::Vec{0.0, 0.0},
            //ColourUtil::RgbToU32(t.colour)
        };

        newNodes.emplace_back(n);

        for (const Mix& m : t.mix)
        {
            bool duplicate = false;

            for (TrackMap::Edge& existing : newEdges)
            {
                if ((existing.A_id == t.id && existing.B_id == m.id) ||
                    (existing.A_id == m.id && existing.B_id == t.id))
                {
                    duplicate = true;
                    break;
                }
            }

            if (!duplicate)
            {
                TrackMap::Edge e;
                e.A_id = t.id;
                e.B_id = m.id;
                
                if (m.direction == MixDirection::In)
                    e.A_direction = false;

                if (m.direction == MixDirection::Out)
                    e.B_direction = false;

                newEdges.push_back(e);
            }

            /*
            TrackMap::Edge e{
                t.id,
                m.id
            };

            newEdges.emplace_back(e);
            */
        }
    }

    map.Bake(std::move(newNodes), std::move(newEdges));
}