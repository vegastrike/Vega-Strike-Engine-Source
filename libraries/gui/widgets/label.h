/*
 * label.h
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

#ifndef VEGA_STRIKE_LIBRARIES_GUI_WIDGETS_LABEL_H
#define VEGA_STRIKE_LIBRARIES_GUI_WIDGETS_LABEL_H

#include <vector>
#include <string>

#include "imgui.h"
#include "widget.h"
#include "collections.h"

enum class TextAlignment {
    left, right, center
};

class Label: public Widget {
protected:
    const std::string text;
    const int width;
    ColorCollection colors;
    ImFont* font;
    TextAlignment alignment;

public:
    Label(const std::string& text, int width = 0, ColorCollection colors = ColorCollection(), 
          ImFont* font = nullptr, 
          TextAlignment alignment = TextAlignment::left);
    ~Label() override {}
    void Draw() override;
};

#endif //VEGA_STRIKE_LIBRARIES_GUI_WIDGETS_LABEL_H
