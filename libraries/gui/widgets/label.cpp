/*
 * label.cpp
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

#include "label.h"

#include "imgui.h"

Label::Label(const std::string& text, int width, ImFont* font, 
             const ImU32* color, TextAlignment alignment):
             text(text), width(width), font(font), color(color), alignment(alignment) {}
    
void Label::Draw() const {
    const ImVec2 cell_start = ImGui::GetCursorScreenPos(); // top-left of child
    const ImVec2 text_size = ImGui::CalcTextSize(text.c_str());

    if(alignment == TextAlignment::center) {
        ImGui::SetCursorScreenPos(ImVec2(
            cell_start.x + (width - text_size.x) * 0.5f,
            cell_start.y
        ));
    } else if(alignment == TextAlignment::right) {
        ImGui::SetCursorScreenPos(ImVec2(
            cell_start.x + (width - text_size.x),
            cell_start.y
        ));
    }

    if(font) {
        ImGui::PushFont(font);
    }

    ImGui::Text("%s", text.c_str());

    if(font) {
        ImGui::PopFont();
    }
}

