/*
 * navscreen.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: hellcatv, ace123, surfdargent, klaussfreire, jacks, pyramid3d
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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
#ifndef VEGA_STRIKE_ENGINE_GFX_NAV_NAV_SCREEN_H
#define VEGA_STRIKE_ENGINE_GFX_NAV_NAV_SCREEN_H

#include "gfx/nav/navscreenoccupied.h"
#include "gfx/nav/drawlist.h"
#include "gfx/nav/navitemtypes.h"
#include "gfx/masks.h"
#include "gfx/nav/navcomputer.h"
#include "gfx/nav/navpath.h"
#include "gfx/hud.h"
#include "src/gnuhash.h"

#include <map>

#define NAVTOTALMESHCOUNT 8     //same as the button count, 1 mesh for screen and 1 per button(1+7)
#define MAXZOOM 10

// The smallest an item is drawn, in the units the nav positions use: the screen spans two
// units across, so this is about 15 pixels. Items never shrink below it, which is what
// keeps a distant item visible and gives it a click area worth aiming at.
static inline float NavMinItemSize() {
    const float pixels = 15.0f;
    const float resolution = static_cast<float>(g_game.x_resolution);
    return (resolution > 0.0f) ? ((2.0f * pixels) / resolution) : 0.015f;
}

void Beautify(string systemfile, string &sector, string &system);

#endif //VEGA_STRIKE_ENGINE_GFX_NAV_NAV_SCREEN_H
