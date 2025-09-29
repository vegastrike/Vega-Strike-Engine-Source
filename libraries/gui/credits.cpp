/*
 * credits.cpp
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
#include <memory>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <boost/json.hpp>
#include <boost/system/error_code.hpp>

#include "imgui.h"
#include "label.h"
#include "spacer.h"
#include "clickable_text.h"
#include "layout.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"

struct Section {
    std::string title;
    std::vector<std::string> lines;
};

struct Credits {
    std::string title;
    std::string subtitle;
    std::vector<Section> sections;
};


Credits ParseJSON(const std::string& filename) {
    Credits credits;

    // Open and read the file
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return credits;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_content = buffer.str();

    // Parse JSON using Boost.JSON
    boost::json::error_code ec;
    boost::json::value jv = boost::json::parse(json_content, ec);
    if (ec) {
        std::cerr << "JSON parse error: " << ec.message() << std::endl;
        return credits;
    }
    const boost::json::object& obj = jv.as_object();

    credits.title = obj.at("title").as_string().c_str();
    credits.subtitle = obj.at("subtitle").as_string().c_str();

    if (obj.contains("sections") && obj.at("sections").is_array()) {
        for (const auto& sec_val : obj.at("sections").as_array()) {
            const auto& sec_obj = sec_val.as_object();
            Section section;
            section.title = sec_obj.at("title").as_string().c_str();
            if (sec_obj.contains("lines") && sec_obj.at("lines").is_array()) {
                for (const auto& line_val : sec_obj.at("lines").as_array()) {
                    section.lines.push_back(line_val.as_string().c_str());
                }
            }
            credits.sections.push_back(std::move(section));
        }
    }

    return credits;
}



void RenderCredits(Credits credits, std::vector<Layout*>& layouts,
                   std::vector<Widget*>& widgets, std::vector<ImFont*> fonts, 
                   int width) {
    ColorCollection colors;
    Layout* layout = new Layout();
    layouts.push_back(layout);
           
    // Title
    Label* title = new Label(credits.title, width, colors, fonts[2], TextAlignment::center);
    Layout* title_row = new Layout();
    title_row->AddWidget(title);
    layout->AddWidget(title_row);
    layouts.push_back(title_row);
    widgets.push_back(title);

    // Subtitle
    Label* subtitle = new Label(credits.subtitle, width, colors, fonts[1], TextAlignment::center);
    Layout* subtitle_row = new Layout();
    subtitle_row->AddWidget(subtitle);
    layout->AddWidget(subtitle_row);
    layouts.push_back(subtitle_row);
    widgets.push_back(subtitle);

    // Spacer
    Layout* spacer_row = new Layout();
    layout->AddWidget(spacer_row);
    Spacer* spacer = new Spacer(0,40);
    spacer_row->AddWidget(spacer);
    layouts.push_back(spacer_row);
    widgets.push_back(spacer);

    colors.border_color = IM_COL32(255,255,224,255);
    for(const Section& section : credits.sections) {
        if(colors.border_color == IM_COL32(255,255,224,255)) {
            colors.border_color = IM_COL32(255,0,0,255);
        } else {
            colors.border_color = IM_COL32(255,255,224,255);
        }
        // Section Title section_title
        Label* label = new Label(section.title, width/3, colors, fonts[1], TextAlignment::center);
        widgets.push_back(label);
        
        Layout* child_layout = new Layout(LayoutType::vertical, colors);
        layouts.push_back(child_layout);
        child_layout->AddWidget(label);
        child_layout->SetBorder(1.0);
        
        layout->AddWidget(child_layout);

        int column = 0;
        
        Layout* row = new Layout(LayoutType::horizontal, colors, 3);
        row->SetBorder(1);
        layouts.push_back(row);
        layout->AddWidget(row);

        Layout* cell_1 = new Layout();
        layouts.push_back(cell_1);
        row->AddWidget(cell_1);
        Layout* cell_2 = new Layout();
        layouts.push_back(cell_2);
        row->AddWidget(cell_2);
        Layout* cell_3 = new Layout();
        layouts.push_back(cell_3);
        row->AddWidget(cell_3);

        for(const std::string& name : section.lines) {
            label = new Label(name, 0, colors, fonts[0], TextAlignment::left);
            widgets.push_back(label);

            switch(column) {
                case 0: cell_1->AddWidget(label); break;
                case 1: cell_2->AddWidget(label); break;
                case 2: cell_3->AddWidget(label); break;
            }
            
            column++;
            if(column == 3) {
                column = 0;
            }
        }
    }

    
}

void RenderCredits2(Credits credits, std::vector<Layout*>& layouts,
                   std::vector<Widget*>& widgets, std::vector<ImFont*> fonts, 
                   int width) {
    ColorCollection colors;
    Layout* layout = new Layout(LayoutType::horizontal, colors, 3);
    layouts.push_back(layout);

    Layout* cell_1 = new Layout();
    cell_1->SetBorder(1.0);
    layouts.push_back(cell_1);
    layout->AddWidget(cell_1);
    Label* label_1 = new Label("Label_1", 0, colors, fonts[0], TextAlignment::left);
    cell_1->AddWidget(label_1);
    widgets.push_back(label_1);
    
    Layout* cell_2 = new Layout();
    cell_2->SetBorder(1.0);
    layouts.push_back(cell_2);
    layout->AddWidget(cell_2);
    Label* label_2 = new Label("Label_2", 0, colors, fonts[0], TextAlignment::left);
    cell_2->AddWidget(label_2);
    widgets.push_back(label_2);
    
    Layout* cell_3 = new Layout();
    cell_3->SetBorder(1.0);
    layouts.push_back(cell_3);
    layout->AddWidget(cell_3);
    Label* label_3 = new Label("Label_3", 0, colors, fonts[0], TextAlignment::left);
    cell_3->AddWidget(label_3);
    widgets.push_back(label_3);
}

// Show Credits Screen
void ShowCredits(SDL_Renderer* renderer, SDL_Window *window, std::vector<ImFont*> fonts, int width) {
    Credits credits = ParseJSON("credits.json");

    std::vector<Layout*> layouts;
    std::vector<Widget*> widgets;

    RenderCredits2(credits, layouts, widgets, fonts, width);


    // TODO: different background

    //ClickableText back("Back");
    
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
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBackground;   // makes it transparent

        // Start the Dear ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            ImVec2 starting_position(20,20);
            ImVec2 window_size = ImGui::GetIO().DisplaySize;
            window_size.x -= 20;
            window_size.y -= 20;
            ImGui::SetNextWindowPos(starting_position, ImGuiCond_Always);
            ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);

            ImGui::Begin("Hello, world!", nullptr, window_flags); // Create a window called "Hello, world!" and append into it.
            
            layouts[0]->Draw();

            

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

    // Cleanup
    for(Layout* child_layout : layouts) {
        delete child_layout;
    }

    for(Widget* widget : widgets) {
        delete widget;
    }
}
