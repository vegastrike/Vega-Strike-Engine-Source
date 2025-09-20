/*
 * drawgalaxy.h
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


/// Nav computer functions
/// Draws in-system map, and  galaxy map of known sectors and systems

#ifndef VEGA_STRIKE_ENGINE_GFX_NAV_DRAW_GALAXY_H
#define VEGA_STRIKE_ENGINE_GFX_NAV_DRAW_GALAXY_H

#include <string>

#include "gfxlib_struct.h"

extern float SYSTEM_DEFAULT_SIZE;
extern GFXColor GrayColor;

class navscreenoccupied;

void DrawNodeDescription(std::string text,
        float x_,
        float y_,
        float size_x,
        float size_y,
        bool ignore_occupied_areas,
        const GFXColor &col,
        navscreenoccupied *screenoccupation);


char GetSystemColor(std::string source);

void DrawNode(int type,
        float size,
        float x,
        float y,
        std::string source,
        navscreenoccupied *screenoccupation,
        bool moused,
        GFXColor race,
        bool mouseover = false,
        bool willclick = false,
        std::string insector = "");

bool checkedVisited(const std::string &n);


#endif //VEGA_STRIKE_ENGINE_GFX_NAV_DRAW_GALAXY_H

