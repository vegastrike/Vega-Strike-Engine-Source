/*
 * layout.h
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

#ifndef VEGA_STRIKE_LIBRARIES_GUI_LAYOUT_LAYOUT_H
#define VEGA_STRIKE_LIBRARIES_GUI_LAYOUT_LAYOUT_H

#include <vector>
#include <string>

#include "imgui.h"

class Label;

enum class LayoutType {
    horizontal, vertical, cell
};

class Layout {
public:
    static std::vector<ImFont*> *fonts;

private:
    LayoutType type;
    bool root;
    std::vector<Layout*> cells;
    //std::vector<Label*> widget;
    ImVec2 size;
    ImVec2 layout_start;
    ImVec2 layout_end;
    ImVec2 (*custom_draw_func)() = nullptr;


    ImU32 border_color;
    ImU32 fill_color;

public:
    Layout(LayoutType type, bool root = false, ImVec2 (*custom_draw_func)() = nullptr, 
           ImU32 border_color = IM_COL32(0,0,0,0), ImU32 fill_color = IM_COL32(0,0,0,0));
    void AddChildLayout(Layout* child_layout);
    ImVec2 Draw();
    void DrawBorder() const;
};

#endif //VEGA_STRIKE_LIBRARIES_GUI_LAYOUT_LAYOUT_H
