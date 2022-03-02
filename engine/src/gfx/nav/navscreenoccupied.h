/**
 * navscreenoccupied.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef _NAVSCREENOCCUPIED_H_
#define _NAVSCREENOCCUPIED_H_

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
#endif

