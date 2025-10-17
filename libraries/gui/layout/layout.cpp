/*
 * layout.cpp
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

#include "layout.h"
#include "widgets/widget.h"
#include "imgui.h"

std::vector<ImFont*> *Layout::fonts = nullptr;

Layout::Layout(LayoutType type, 
               ColorCollection colors, int columns): 
               type(type),
               colors(colors), columns(columns) {
    name = std::to_string((unsigned long long)this);
}

void Layout::AddWidget(Widget* widget) {
    // Don't allow widgets other than layouts in horizontal layout
    if(type == LayoutType::horizontal) {
        Layout* child_layout = dynamic_cast<Layout*>(widget);
        if(!child_layout) {
            return;
        }
    }

    widgets.push_back(widget);
}
    
void Layout::Draw() {
    // TODO: margin and padding

    if(type == LayoutType::vertical) {
        for(Widget* widget : widgets) {
            widget->Draw();
        }
    }
    

    if(type == LayoutType::horizontal) {
        ImVec2 size = ImGui::GetContentRegionAvail();
        if (!ImGui::BeginChild(name.c_str(), size, false, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar)) {
            ImGui::EndChild();
            return;            
        }

        size.x /= columns;
        size.y = 0;
        int i=0;
        for(Widget* widget : widgets) {
            Layout* child_layout = dynamic_cast<Layout*>(widget);
            if(!child_layout) {
                return;
            }

            if (!ImGui::BeginChild(child_layout->name.c_str(), size, false, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar)) {
                ImGui::EndChild();
                continue;
            }
                
            child_layout->Draw();

            if(child_layout->border_width > 0) {
                DrawBorder();
            }

            ImGui::EndChild();
            i++;

            if(widget != widgets.back()) {
                ImGui::SameLine((size.x+20) * i);
            }
        }

        if(border_width > 0) {
            DrawBorder();
        }

        ImGui::EndChild();
    }
}

void Layout::DrawBorder() const {
    // TODO: turn into style
    float corner_radius = 0.0f;

    ImVec2 layout_start = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    
    ImVec2 layout_end(layout_start.x + size.x, layout_start.y + size.y);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    if(colors.background_color != 0) {
        ImVec2 fill_start(layout_start.x+1, layout_start.y+1);
        ImVec2 fill_end(layout_end.x-1, layout_end.y-1);
        dl->AddRectFilled(fill_start, fill_end, colors.background_color, corner_radius, 0);
    } 
    
    if(colors.border_color != 0) {
        dl->AddRect(layout_start, layout_end, colors.border_color, corner_radius, 0, border_width);
    }
}

void Layout::SetColumns(const int columns) {
    this->columns = columns;
}

void Layout::SetBorder(const float border_width) {
    this->border_width = border_width;
}

