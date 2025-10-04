/*
 * load_game.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <boost/json.hpp>
#include <boost/system/error_code.hpp>

#include "imgui.h"
#include "widgets/label.h"
#include "clickable_text.h"
#include "selection_group.h"
#include "spacer.h"
#include "toggleable_text.h"
#include "layout.h"
#include "selection_group.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"



struct DummySaveGame {
    std::string name;
    long time_date_saved;
    int credits;
    std::string sector;
    std::string system;
    DummySaveGame(const std::string& name, const long time_stamp, const int credits,
        const std::string& sector, const std::string& system):
        name(name), time_date_saved(time_stamp), credits(credits), 
        sector(sector), system(system) {}
};

std::vector<DummySaveGame> dummy_save_games = {
    {"save1.sav", 1719878400, 5000, "Sol", "Earth"},
    {"save2.sav", 1719964800, 12000, "Alpha Centauri", "Proxima"},
    {"save3.sav", 1720051200, 800, "Sirius", "Sirius A"},
    {"save4.sav", 1720137600, 25000, "Vega", "Vega Prime"},
    {"save5.sav", 1720224000, 1500, "Barnard's Star", "Barnard I"}
};



static void GenerateLoadLayout(std::vector<DummySaveGame> dummy_save_games, 
                               std::vector<Layout*>& layouts,
                               std::vector<Widget*>& widgets, 
                               std::vector<ImFont*> fonts, int width) {
    ColorCollection colors;
    Layout* layout = new Layout();
    layouts.push_back(layout);
           
    // Title
    Label* title = new Label("Load Game", width, colors, fonts[2], TextAlignment::center);
    Layout* title_row = new Layout();
    title_row->AddWidget(title);
    layout->AddWidget(title_row);
    layouts.push_back(title_row);
    widgets.push_back(title);

    // Spacer
    Layout* spacer_row = new Layout();
    layout->AddWidget(spacer_row);
    Spacer* spacer = new Spacer(0,40);
    spacer_row->AddWidget(spacer);
    layouts.push_back(spacer_row);
    widgets.push_back(spacer);

    // Two columns row
    Layout* row = new Layout(LayoutType::horizontal);
    layouts.push_back(row);
    layout->AddWidget(row);

    Layout* left_cell = new Layout();
    layouts.push_back(left_cell);
    row->AddWidget(left_cell);

    // Iterate over save games
    SelectionGroup* group = new SelectionGroup(width/2, colors);
    left_cell->AddWidget(group);
    widgets.push_back(group);
    for(const DummySaveGame& game : dummy_save_games) {
        group->Add(game.name);
    }


    Layout* right_cell = new Layout();
    layouts.push_back(right_cell);
    row->AddWidget(right_cell);
        

}


// Show Credits Screen
void ShowLoadScreen(SDL_Renderer* renderer, SDL_Window *window, std::vector<ImFont*> fonts, int width) {
    std::vector<Layout*> layouts;
    std::vector<Widget*> widgets;

    GenerateLoadLayout(dummy_save_games, layouts, widgets, fonts, width);

    // TODO: different background

    // SelectionGroup selection_group;
    // for(const DummySaveGame save_game : dummy_save_games) {
    //     selection_group.Add(save_game.name);
    // }
    // ClickableText load("Load");
    // ClickableText back("Back");
    
    bool done = false;

    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;

        ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

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

            ImGui::Begin("Hello, world!", nullptr, window_flags); // Create a window called "Hello, world!" and append into it.
            layouts[0]->Draw();


            // ImGui::PushFont(font_small);
            // ImGui::PushFont(font_medium);
            ImGui::PushFont(fonts[2]);

            //back.RenderText();
            
            // ImGui::PopFont();
            // ImGui::PopFont();
            ImGui::PopFont();
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        ImGuiIO& io{ImGui::GetIO()};
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);
        //renderBackgroundImage(renderer, window, background_texture);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);

        // if(back.GetClickAndReset()) {
        //     done = true;
        // }
    }
}
