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
#include "collections.h"

class Widget;

enum class LayoutType {
    horizontal, vertical, cell
};

class Layout {
public:
    static std::vector<ImFont*> *fonts;

private:
    LayoutType type;
    bool root;
    ColorCollection colors;
    std::vector<Layout*> cells;
    std::vector<Widget*> widgets;
    ImVec2 size;
    ImVec2 layout_start;
    ImVec2 layout_end;
    int columns = -1;
    float border_width = 0;

public:
    Layout(LayoutType type, bool root = false,
           ColorCollection colors = ColorCollection());
    void AddChildLayout(Layout* child_layout);
    void AddWidget(Widget* widget);
    ImVec2 Draw();
    void DrawBorder() const;
    void SetColumns(const int columns);
    void SetBorder(const float border_width);
};

#endif //VEGA_STRIKE_LIBRARIES_GUI_LAYOUT_LAYOUT_H
