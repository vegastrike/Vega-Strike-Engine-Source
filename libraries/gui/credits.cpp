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

    // For demonstration, print parsed data
    std::cout << "Title: " << credits.title << std::endl;
    std::cout << "Subtitle: " << credits.subtitle << std::endl;
    for (const auto& section : credits.sections) {
        std::cout << "Section: " << section.title << std::endl;
        for (const auto& line : section.lines) {
            std::cout << "  " << line << std::endl;
        }
    }

    return credits;
}

void RenderTitle(ImGuiWindowFlags window_flags, std::vector<ImFont*> fonts) {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

    ImGui::Begin("Title", nullptr, window_flags);                          // Create a window called "Hello, world!" and append into it.
    ImGui::PushFont(fonts[2]);

    ImVec2 window_size = ImGui::GetWindowSize();
    ImVec2 text_size = ImGui::CalcTextSize("Vega Strike Credits");
    float x = (window_size.x - text_size.x) * 0.5f;
    ImGui::SetCursorPosX(x);
    ImGui::Text("Vega Strike Credits");    

    ImGui::PopFont();
    ImGui::End();
}

void RenderCredits(Credits credits, std::vector<ImFont*> fonts, bool& done, int width) {
    ColorCollection colors;
    Layout layout(LayoutType::vertical, true);
           
    // Title
    Label title(credits.title, width, colors, fonts[2], TextAlignment::center);
    Layout title_row(LayoutType::cell, false);
    title_row.AddWidget(&title);
    layout.AddChildLayout(&title_row);

    // Subtitle
    Label subtitle(credits.subtitle, width, colors, fonts[1], TextAlignment::center);
    Layout subtitle_row(LayoutType::cell, false);
    subtitle_row.AddWidget(&subtitle);
    layout.AddChildLayout(&subtitle_row);

    // Spacer
    Layout spacer_row(LayoutType::cell, false);
    layout.AddChildLayout(&spacer_row);

    std::vector<Layout*> layouts;
    std::vector<Widget*> widgets;
    for(const Section& section : credits.sections) {
        // Section Title section_title
        Label* label = new Label(section.title, width/3, colors, fonts[1], TextAlignment::center);
        widgets.push_back(label);

        Layout* child_layout = new Layout(LayoutType::cell, false);
        layouts.push_back(child_layout);
        child_layout->AddWidget(label);
        layout.AddChildLayout(child_layout);

        int column = 0;
        
        Layout* row = nullptr;
        Layout* cell = nullptr;
        for(const std::string& name : section.lines) {
            if(column == 0) {
                row = new Layout(LayoutType::horizontal, false);
                layouts.push_back(row);
                layout.AddChildLayout(row);
            }

            cell = new Layout(LayoutType::cell, false);
            layouts.push_back(cell);
            row->AddChildLayout(cell);

            label = new Label(name, width/3, colors, fonts[0], TextAlignment::left);
            widgets.push_back(label);
            cell->AddWidget(label);

            column++;
            if(column == 3) {
                column = 0;
            }
        }
    }

    layout.Draw();

    // Cleanup
    for(Layout* child_layout : layouts) {
        delete child_layout;
    }

    for(Widget* widget : widgets) {
        delete widget;
    }

    /*

        for(const Section& section : credits.sections) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushFont(fonts[1]);
            ImGui::Text("%s", section.title.c_str());
            ImGui::PopFont();

            ImGui::PushFont(fonts[3]);
            int column = 0;
            for(const std::string& name : section.lines) {
                if(column == 0) {
                    ImGui::TableNextRow();
                }

                ImGui::TableNextColumn();
                ImGui::Text("%s", name.c_str());

                column++;
                if(column == 3) {
                    column = 0;
                }
            }
            ImGui::PopFont();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", "");
        }
    
        // Space row
        ImGui::TableNextRow();
        ImGui::TableNextRow();
        // 3rd column
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        //ImGui::Text("Back");
        //back.RenderText();

        ImGui::EndTable();
    }*/
}

// Show Credits Screen
void ShowCredits(SDL_Renderer* renderer, SDL_Window *window, std::vector<ImFont*> fonts, int width) {
    Credits credits = ParseJSON("credits.json");
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
            //RenderTitle(window_flags, fonts);
            RenderCredits(credits, fonts, done, width);

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
