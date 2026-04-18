// Dear ImGui Windows API + DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp
// Dear ImGui: standalone example application for Windows API + DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

#include "../include/catalogueManager.hpp"

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int main(int, char**)
{
    // Make process DPI aware and obtain main monitor scale
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, (int)(1280 * main_scale), (int)(800 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefaultVector();
    //io.Fonts->AddFontDefaultBitmap();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

    style.FontSizeBase = 20.f;
    ImFont* mainFont =  io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/Bahnschrift.ttf");

    // Style
    //style.FrameRounding = 10.f;

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    CatalogueManager manager;
    manager.load();

    int activeTrackId = -1;

    // Search window
    //static int searchWindowHeight = 50;
    static char mainSearchBuffer[64] = "";
    bool showSearchResultWindow = false;
    //static ImGuiTextFilter searchFilter;
   
    // Add window
    bool showAddTrackWindow = false;
    static char newTrackName[64] = "";
    static char newArtistName[64] = "";

    // Track view / edit
    bool showAddMixWindow = false;
    static char mixSearchBuffer[64] = "";

    bool done = false;
    bool open = true;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        //ImGui::ShowDemoWindow();

        ImGui::PushFont(mainFont);
        ImGui::PopFont();

        /*              Search                 */

        

        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 0));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("Search", &open,
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse);

        ImVec2 searchBarWindowSize = ImGui::GetWindowSize();

        // Search bar
        ImGui::InputText("##MainSearchInput", mainSearchBuffer, sizeof(mainSearchBuffer));
        if (ImGui::IsItemActive())
        {
            showSearchResultWindow = true;
            // TODO: get this working
            /*
            if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                showSearchResultWindow = false;    
            */
        }

        ImGui::SameLine();

        // Add new track button
        if (ImGui::Button("Add"))
            showAddTrackWindow = true;
        
        ImGui::SameLine();

        // Refresh catalogue button
        if (ImGui::Button("Refresh"))
            manager.refresh();
        
        if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
            !ImGui::IsAnyItemActive())
            showSearchResultWindow = false;
       
        ImGui::End();

        // Add new track window
        if (showAddTrackWindow)
        {
            showSearchResultWindow = false;
            ImGui::Begin("Add New Track", &showAddTrackWindow,
                ImGuiWindowFlags_NoCollapse);

            ImGui::InputText("Artist Name", newArtistName, sizeof(newArtistName));
            ImGui::InputText("Track Name", newTrackName, sizeof(newTrackName));

            if (ImGui::Button("Save"))
            {
                Track newTrack;
                newTrack.artistName = newArtistName;
                newTrack.trackName = newTrackName;
                bool trackAdded = manager.addTrack(newTrack);

                if (trackAdded)
                {
                    if (!manager.getCatalogue().empty())
                        activeTrackId = manager.getCatalogue().back().id;

                    newArtistName[0] = '\0';
                    newTrackName[0] = '\0';

                    showAddTrackWindow = false;
                }   
            }
            ImGui::End();
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

                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(60, 60, 60, 80));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(60, 60, 60, 120));

                if (ImGui::Button(trackDisplayText.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                {
                    activeTrackId = t.id;
                    mainSearchBuffer[0] = '\0';
                    showSearchResultWindow = false;
                }

                ImGui::PopStyleColor(3);

                
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
            // ImGui::GetContentRegionAvail().x
            // io.DisplaySize.x
    
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();

            ImVec2 bannerPos(
                cursorPos.x,
                cursorPos.y + style.WindowPadding.y);

            ImVec2 bannerSize(ImGui::GetContentRegionAvail().x, 200);
            float bannerPadding = 15;

            ImDrawList* draw = ImGui::GetWindowDrawList();

            // Background
            draw->AddRectFilled(
                bannerPos,
                ImVec2(bannerPos.x + bannerSize.x, bannerPos.y + bannerSize.y),
                IM_COL32(50, 50, 70, 255),
                6.0f);

            // Artist name
            std::string artistName = activeTrack->artistName;
            ImVec2 artistSize = ImGui::CalcTextSize(artistName.c_str());

            draw->AddText(
                ImVec2(
                    bannerPos.x + bannerPadding,
                    bannerPos.y + bannerSize.y * 0.33f - artistSize.y * 0.5f),
                IM_COL32(255, 255, 255, 255),
                artistName.c_str());

            // Track name
            std::string trackName = activeTrack->trackName;
            ImVec2 trackSize = ImGui::CalcTextSize(trackName.c_str());

            draw->AddText(
                ImVec2(
                    bannerPos.x + bannerPadding,
                    bannerPos.y + bannerSize.y * 0.66f - trackSize.y * 0.5f),
                IM_COL32(255, 255, 255, 255),
                trackName.c_str());


            // Remove active track button
            ImVec2 buttonText = ImGui::CalcTextSize("Remove");
            ImVec2 buttonSize(
                buttonText.x + style.FramePadding.x * 2.0f,
                buttonText.y + style.FramePadding.y * 2.0f);

            ImGui::SetCursorScreenPos(
                ImVec2(
                    bannerPos.x + bannerSize.x - buttonSize.x - bannerPadding,
                    bannerPos.y + bannerSize.y - buttonSize.y - bannerPadding));

            if (ImGui::Button("Remove"))
                removeActiveTrack = true;
            
            ImGui::SetCursorScreenPos(bannerPos);
            ImGui::Dummy(bannerSize);

            // Mixes
            ImGui::Text("Mixes");

            ImGui::SameLine();

            // Add new mix button
            if (ImGui::Button("Add"))
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
            for (int mixId : activeTrack->mixIds)
            {
                const Track* mixTrack = manager.getTrack(mixId);

                if (mixTrack == nullptr)
                    continue;

                std::string label = mixTrack->artistName + " - " + mixTrack->trackName;

                if (ImGui::Button(label.c_str(), ImVec2(300, 0)))
                    activeTrackId = mixTrack->id;

                ImGui::SameLine();

                std::string buttonId = "Remove##" + std::to_string(mixId);

                if (ImGui::Button(buttonId.c_str()))
                    removeMixId = mixId;
            }

            if (removeMixId != -1)
                manager.removeMix(activeTrackId, removeMixId);
        }

        if (removeActiveTrack == true)
        {
            manager.removeTrack(activeTrackId);
            activeTrack = nullptr;
            activeTrackId = -1;
        }
        ImGui::End();

        ImGui::EndFrame();


        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
        //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    // This is a basic setup. Optimally could use e.g. DXGI_SWAP_EFFECT_FLIP_DISCARD and handle fullscreen mode differently. See #8979 for suggestions.
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
