/*
 * collections.h
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

#ifndef VEGA_STRIKE_LIBRARIES_GUI_COLLECTIONS_H
#define VEGA_STRIKE_LIBRARIES_GUI_COLLECTIONS_H

#include "imgui.h"

struct ColorCollection {
    ImU32 color = IM_COL32(255,255,224,255); // Light yellow
    ImU32 background_color;
    ImU32 border_color = IM_COL32(255,255,224,255); // Light yellow
    ImU32 hover_color = IM_COL32(144, 238, 144, 255); // Light Green
    ImU32 click_color = IM_COL32(255, 204, 203, 255); // Light red
    ImU32 toggle_color = IM_COL32(144, 213, 255, 255); // Light blue
    ImU32 non_toggle_color;
    ImU32 transparent_color = IM_COL32(0,0,0,0); 
};

#endif //VEGA_STRIKE_LIBRARIES_GUI_COLLECTIONS_H