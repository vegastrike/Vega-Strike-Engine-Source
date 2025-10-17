/*
 * selection_group.cpp
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

#include "selection_group.h"

#include <iostream>

SelectionGroup::SelectionGroup(int width, ColorCollection colors, 
          ImFont* font, TextAlignment alignment):
          width(width), colors(colors), 
          font(font), alignment(alignment) {}

SelectionGroup::SelectionGroup(const std::vector<std::string>& texts, int width, ColorCollection colors, 
          ImFont* font, TextAlignment alignment):
          width(width), colors(colors), 
          font(font), alignment(alignment) {
    int index = 0;
    for(const std::string& text : texts) {
        toggleable_texts.push_back(ToggleableText(text, width, colors, font, alignment,
             index, this));
        index++;
    }
}

void SelectionGroup::Add(const std::string& text) {
    toggleable_texts.push_back(ToggleableText(text, width, colors, font, alignment, toggleable_texts.size(), this));
}
    
void SelectionGroup::SetSelected(const int index) {
    for(ToggleableText& toggleable_text : toggleable_texts) {
        if(toggleable_text.group_index != index && toggleable_text.toggled) {
            toggleable_text.toggled = false;
            toggleable_text.colors.color = toggleable_text.backup_color;
        }
    }
}
    
ToggleableText& SelectionGroup::GetText(const int index) {
    return toggleable_texts[index];
}

void SelectionGroup::Draw() {
    for(ToggleableText& toggleable_text : toggleable_texts) {
        toggleable_text.Draw();
    }
    
}

