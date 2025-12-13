#include "SDL3/SDL.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"

#include <iostream>

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

// Show Menu
void showMenu(SDL_Renderer* renderer, SDL_Window *window) {
    std::cout << "Begin showMenu\n";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io{ImGui::GetIO()};
    ImGui::StyleColorsDark();

    std::cout << "Finished init\n";

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    std::cout << "Finished setup\n";

    setBackgroundColor(0.0f,0.0f,0.0f,1.0f);
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

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
            ImGui_ImplSDL3_ProcessEvent(&event);
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

        bool show_another_window;

        ImGuiWindowFlags window_flags = 
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBackground;   // makes it transparent

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

            ImGui::Begin("Hello, world!", nullptr, window_flags);                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            // ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            // ImGui::Text("Hello from another window!");
            // if (ImGui::Button("Close Me"))
            //     show_another_window = false;
            // ImGui::End();
        }

        // Rendering
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
