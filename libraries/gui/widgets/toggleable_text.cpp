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


ToggleableText::ToggleableText(const std::string& text, int width, 
                ColorCollection colors, 
                ImFont* font, TextAlignment alignment, 
                int group_index, SelectionGroup *group): 
                ClickableText(text, width, colors, font, alignment),
                in_group(group_index != -1), group_index(group_index), group(group) {}

void ToggleableText::RenderText() {
    if(toggled) {
        color = colors.toggle_color;
    } else {
        color = colors.non_toggle_color;
    }

    //ClickableText::RenderText();

    if(clicked) {
        toggled = !toggled;
        clicked = false;

        if(in_group && group) {
            group->SetSelected(group_index);
        }
    }
}

bool ToggleableText::Toggled() const {
    return toggled;
}