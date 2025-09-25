/*
 * clickable_text.h
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

#ifndef VEGA_STRIKE_LIBRARIES_GUI_CLICKABLE_TEXT_H
#define VEGA_STRIKE_LIBRARIES_GUI_CLICKABLE_TEXT_H

#include <string>

// Must come before imgui.h
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

class ClickableText {
protected:
    std::string text;
    bool clicked = false;
    int click_counter = 0;

    ImVec4 transparent_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); 
    ImVec4 color = ImVec4(0.98f, 0.97f, 0.96f, 1.0f); // Of-white
    ImVec4 hovered_color = ImVec4(0.56f, 0.93f, 0.56f, 1.0f); // Light Green
    ImVec4 click_color = ImVec4(1.0f, 0.72f, 0.72f, 1.0f); 

public:
    ClickableText(const std::string& text);
    void RenderText();
    bool GetClickAndReset();
    void SetColor(const ImVec4 new_color);
};

#endif //VEGA_STRIKE_LIBRARIES_GUI_CLICKABLE_TEXT_H