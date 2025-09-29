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
#include "widget.h"
#include "collections.h"


enum class LayoutType {
    horizontal, vertical
};

class Layout : public Widget {
public:
    static std::vector<ImFont*> *fonts;

private:
    LayoutType type;
    ColorCollection colors;
    std::vector<Widget*> widgets;
    int columns;
    std::string name;
    float border_width = 0;

public:
    Layout(LayoutType type = LayoutType::vertical, 
           ColorCollection colors = ColorCollection(),
           int columns = -1);
    void AddWidget(Widget* widget);
    void Draw() override;
    void DrawBorder() const;
    void SetColumns(const int columns);
    void SetBorder(const float border_width);
};

#endif //VEGA_STRIKE_LIBRARIES_GUI_LAYOUT_LAYOUT_H
