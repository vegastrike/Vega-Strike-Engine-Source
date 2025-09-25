/*
 * toggleable_text.cpp
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

#include "toggleable_text.h"

#include <iostream>

#include "selection_group.h"

ToggleableText::ToggleableText(const std::string& text): ClickableText(text) {
    std::cout << "ToggleableText::ToggleableText("<<text<<")\n";
}

ToggleableText::ToggleableText(const std::string& text, int index, SelectionGroup *group):
    ClickableText(text), in_group(true), group_index(index), group(group) {
    std::cout << "ToggleableText::ToggleableText("<<text<<","<<index<<")\n";
}

void ToggleableText::RenderText() {
    ImVec4 color = ImVec4(0.98f, 0.97f, 0.96f, 1.0f); // Of-white
    ImVec4 toggle_color = ImVec4(0.1f, 0.1f, 0.56f, 1.0f); // Light blue

    if(toggled) {
        SetColor(toggle_color);
    } else {
        SetColor(color);
    }

    ClickableText::RenderText();

    if(clicked) {
        std::cout << this << " " << toggled << std::endl;
        toggled = !toggled;
        clicked = false;

        if(in_group || group) {
            std::cout << this << "Index " << group_index << " toggled to " << toggled << std::endl;
        }

        if(in_group && group) {
            group->SetSelected(group_index);
        }

        if(in_group || group) {
            std::cout << this << "Index " << group_index << " still toggled to " << toggled << std::endl << std::flush;
        }
    }
}

bool ToggleableText::Toggled() const {
    return toggled;
}