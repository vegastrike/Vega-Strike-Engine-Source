#include <iostream>
#include <vector>
#include "SDL2/SDL.h"
#include <SDL2/SDL_image.h>

// Must come before imgui.h
#define IMGUI_DEFINE_MATH_OPERATORS

#include "clickable_text.h"
#include "credits.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"



void setBackgroundColor(float red, float green, float blue, float transparent) {
    // Get a reference to the ImGui style
    ImGuiStyle& style = ImGui::GetStyle();

    // Get a pointer to the array of colors in the style
    ImVec4* colors = style.Colors;

    // Define your desired background color (RGBA values from 0.0 to 1.0)
    // For example, a light gray with some transparency
    const ImVec4 newBgColor = ImVec4(red, green, blue, transparent);

    // Assign the new color to ImGuiCol_WindowBg
    colors[ImGuiCol_WindowBg] = newBgColor;
}

SDL_Texture* createBackgroundImage(SDL_Renderer* renderer, const std::string& filename) {
    SDL_Texture* background_texture = nullptr;

    SDL_Surface* imageSurface = IMG_Load(filename.c_str());
    if (!imageSurface) {
        std::cerr << "Failed to load image: " << filename << " Error: " << IMG_GetError() << std::endl;
        return nullptr;
    }

    background_texture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_FreeSurface(imageSurface);

    if (!background_texture) {
        std::cerr << "Failed to create texture from image: " << filename << " Error: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    return background_texture;
}

// Helper to render the background image (call at the start of your render loop)
void renderBackgroundImage(SDL_Renderer* renderer, SDL_Window* window, SDL_Texture* background_texture) {
    if (!background_texture) return;

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_Rect destRect = {0, 0, w, h};
    SDL_RenderCopy(renderer, background_texture, nullptr, &destRect);
}

void destroyBackgroundImage(SDL_Texture* background_texture) {
    // Free texture if any
    if (background_texture) {
        SDL_DestroyTexture(background_texture);
        background_texture = nullptr;
    }
}


std::vector<ImFont*> GenerateFonts() {
    ImGuiIO& io{ImGui::GetIO()};
    std::vector<ImFont*> fonts;
    ImFont* font_small = io.Fonts->AddFontFromFileTTF("FrontPageNeue.otf", 16.0f);
    fonts.push_back(font_small);
    ImFont* font_medium = io.Fonts->AddFontFromFileTTF("FrontPageNeue.otf", 18.0f);
    fonts.push_back(font_medium);
    ImFont* font_large = io.Fonts->AddFontFromFileTTF("FrontPageNeue.otf", 36.0f);
    fonts.push_back(font_large);
    ImFont* open_sans_16 = io.Fonts->AddFontFromFileTTF("OpenSans-VariableFont_wdth,wght.ttf", 16.0f);
    fonts.push_back(open_sans_16);
    io.Fonts->Build(); 
    return fonts;
}



// Show Menu
void showMenu(SDL_Renderer* renderer, SDL_Window *window) {
    std::cout << "Begin showMenu\n";

    SDL_Texture* background_texture = createBackgroundImage(renderer, "main_menu.png");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io{ImGui::GetIO()};
    ImGui::StyleColorsDark();

    std::cout << "Finished init\n";

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    std::cout << "Finished setup\n";

    setBackgroundColor(0.0f,0.0f,0.0f,1.0f);
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

    std::vector<ImFont*> fonts = GenerateFonts();

    ClickableText new_game("New Game");
    ClickableText load_game("Load Game");
    ClickableText credits("Credits");
    ClickableText help("Help");
    ClickableText settings("Settings");
    ClickableText quit("Quit");

    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        ImGuiWindowFlags window_flags = 
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBackground;   // makes it transparent

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

            ImGui::Begin("Hello, world!", nullptr, window_flags);                          // Create a window called "Hello, world!" and append into it.

            ImGui::PushFont(fonts[2]);

            new_game.RenderText();
            load_game.RenderText();
            credits.RenderText();
            help.RenderText();
            settings.RenderText();
            quit.RenderText();
            
            ImGui::PopFont();
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        renderBackgroundImage(renderer, window, background_texture);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);

        if(credits.GetClickAndReset()) {
            ShowCredits(renderer, window, fonts);
        }

        if(quit.GetClickAndReset()) {
            done = true;
        }
    }

    // Cleanup
    
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}