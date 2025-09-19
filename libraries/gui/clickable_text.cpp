/*
 * clickable_text.cpp
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

#include "clickable_text.h"

#include <iostream>

ClickableText::ClickableText(const std::string& text): text(text), click_counter(0) {}

void ClickableText::RenderText() {
    // Button Style
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0)); // Transparent background
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 0, 0, 0)); // Transparent on hover
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 0)); // Transparent on active
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f); // Remove border
    ImGui::PushStyleColor(ImGuiCol_Text, transparent_color);

    const ImGuiStyle& style = ImGui::GetStyle();
    const ImVec2 label_size = ImGui::CalcTextSize(text.c_str(), nullptr, true);
    ImVec2 padding = style.FramePadding;
    ImVec2 size = label_size + padding * 2.0f;

    clicked = ImGui::Button(text.c_str(), size);

    if(clicked) {
        click_counter = 10;
        std::cout << text << " clicked\n";
    } 
    
    if(click_counter > 0) {
        ImGui::GetWindowDrawList()->AddText(ImGui::GetItemRectMin(), 
                                            ImGui::ColorConvertFloat4ToU32(click_color),
                                            text.c_str());
        
        if(click_counter >0) {
            click_counter--;
        }
    }
    else if (ImGui::IsItemHovered()) {
        ImGui::GetWindowDrawList()->AddText(ImGui::GetItemRectMin(), 
                                            ImGui::ColorConvertFloat4ToU32(hovered_color),
                                            text.c_str());
    } else {
        ImGui::GetWindowDrawList()->AddText(ImGui::GetItemRectMin(), 
                                            ImGui::ColorConvertFloat4ToU32(off_white_color),
                                            text.c_str());
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
}

bool ClickableText::GetClickAndReset() {
    bool c = clicked;
    clicked = false;
    return c;
}