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

Layout::Layout(LayoutType type, bool root,  
               ColorCollection colors): 
               type(type), root(root), 
               colors(colors) {

}

void Layout::AddChildLayout(Layout* child_layout) {
    if(type != LayoutType::cell) {
        cells.push_back(child_layout);
    } else {
        std::cerr << "Tried to add a child layout to a cell. Exiting.\n" << std::flush;
        assert(0);
    }
}

void Layout::AddWidget(Widget* widget) {
    if(type == LayoutType::cell) {
        widgets.push_back(widget);
    } else {
        std::cerr << "Tried to add a widget to a non-cell layout. Exiting.\n" << std::flush;
        assert(0);
    }
}

ImVec2 Layout::Draw() {
    layout_start = ImGui::GetCursorPos();
    // TODO: margin and padding

    if(root) {
        size = ImGui::GetContentRegionAvail();
    }
    if(type == LayoutType::cell) {
        for(const auto& widget : widgets) {
            widget->Draw();
            layout_end = ImGui::GetCursorScreenPos();
        }   
    }

    if(type == LayoutType::vertical) {
        for(Layout* cell : cells) {
            layout_end = cell->Draw();
        }
    }

    if(type == LayoutType::horizontal) {
        size = ImGui::GetContentRegionAvail();
        int column_width = size.x / cells.size();
        ImVec2 cursor_position = ImGui::GetCursorPos();
        for(Layout* cell : cells) {
             layout_end = cell->Draw();
             cursor_position.x += column_width;
             ImGui::SetCursorPos(cursor_position);
        }
    }

    if(type == LayoutType::cell) {
        DrawBorder();
    }

    return layout_end;
}

void Layout::DrawBorder() const {
    // TODO: turn into style
    float corner_radius = 4.0f;
    float thickness = 4.0f;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    if(colors.background_color != 0) {
        ImVec2 fill_start(layout_start.x+1, layout_start.y+1);
        ImVec2 fill_end(layout_end.x-1, layout_end.y-1);
        dl->AddRectFilled(fill_start, fill_end, colors.background_color, corner_radius, 0);
    } 
    
    if(colors.border_color != 0) {
        dl->AddRect(layout_start, layout_end, colors.border_color, corner_radius, 0, thickness);
    }
}

