/*
 * navscreenoccupied.h
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
#ifndef VEGA_STRIKE_ENGINE_GFX_NAV_NAV_SCREEN_OCCUPIED_H
#define VEGA_STRIKE_ENGINE_GFX_NAV_NAV_SCREEN_OCCUPIED_H

//#include "masks.h"

class navscreenoccupied {
private:

//bits are down. sectors across. starting at top left of navscreen area
    int sector0;
    int sector1;
    int sector2;
    int sector3;
    int sector4;
    int sector5;
    int sector6;
    int sector7;
    int sector8;
    int sector9;

    float screenskipby4[4];
    bool markreturned;

public:
    navscreenoccupied();
    navscreenoccupied(float x_small, float x_large, float y_small, float y_large, bool _markreturned);

    void reset();
    float findfreesector(float x, float y);
    float findfreefloat(int &sector, float &y);
};

#endif //VEGA_STRIKE_ENGINE_GFX_NAV_NAV_SCREEN_OCCUPIED_H
