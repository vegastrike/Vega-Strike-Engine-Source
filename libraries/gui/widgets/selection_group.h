/*
 * selection_group.h
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

#ifndef VEGA_STRIKE_LIBRARIES_GUI_SELECTION_GROUP_H
#define VEGA_STRIKE_LIBRARIES_GUI_SELECTION_GROUP_H

#include <string>
#include <vector>

#include "imgui.h"
#include "widget.h"
#include "toggleable_text.h"

class SelectionGroup: public Widget {
    std::vector<ToggleableText> toggleable_texts;
    int selected = -1;

    const int width;
    ColorCollection colors;
    ImFont* font;
    TextAlignment alignment;

public:
    SelectionGroup(int width, ColorCollection colors, 
          ImFont* font = nullptr, 
          TextAlignment alignment = TextAlignment::left);
    SelectionGroup(const std::vector<std::string>& texts, int width, ColorCollection colors, 
          ImFont* font = nullptr, 
          TextAlignment alignment = TextAlignment::left);
    void Add(const std::string& text);
    void SetSelected(const int index);
    ToggleableText& GetText(const int index);
    void Render(const int index);
};

#endif //VEGA_STRIKE_LIBRARIES_GUI_SELECTION_GROUP_H