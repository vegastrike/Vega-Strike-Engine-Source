/*
 * toggleable_text.h
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

#ifndef VEGA_STRIKE_LIBRARIES_GUI_TOGGLEABLE_TEXT_H
#define VEGA_STRIKE_LIBRARIES_GUI_TOGGLEABLE_TEXT_H

#include <string>

// Must come before imgui.h
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

#include "clickable_text.h"
#include "collections.h"

class SelectionGroup;

class ToggleableText : public ClickableText {
    bool toggled = false;
    bool in_group;
    int group_index;
    SelectionGroup* group;
    

    friend class SelectionGroup;
public:
    ToggleableText(const std::string& text, int width, ColorCollection colors, 
                   ImFont* font = nullptr, TextAlignment alignment = TextAlignment::left, 
                   int group_index = -1, SelectionGroup *group = nullptr);
    void RenderText();
    bool Toggled() const;
};

#endif //VEGA_STRIKE_LIBRARIES_GUI_TOGGLEABLE_TEXT_H