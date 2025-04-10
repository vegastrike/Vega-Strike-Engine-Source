/**
 * drawlist.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_GFX_NAV_DRAW_LIST_H
#define VEGA_STRIKE_ENGINE_GFX_NAV_DRAW_LIST_H

#include <stdlib.h>
#include <string>
#include "cmd/unit_generic.h"
#include "gfx/nav/navscreenoccupied.h"
#include "gui/glut_support.h"

//#include "masks.h"
#include "gfx/nav/navscreen.h"
#include "gfx/nav/navitemtypes.h"

class navdrawnode {
public:
    int type;
    float size;
    float x;
    float y;

    navdrawnode *nextitem;
    Unit *source;

    navdrawnode();
    navdrawnode(int type, float size, float x, float y, navdrawnode *nextitem);
    navdrawnode(int type, float size, float x, float y, Unit *source, navdrawnode *nextitem);
};

void drawdescription(Unit *source,
        float x,
        float y,
        float size_x,
        float size_y,
        bool ignore_occupied_areas,
        navscreenoccupied *screenoccupation,
        const GFXColor &col);
void drawdescription(string text,
        float x,
        float y,
        float size_x,
        float size_y,
        bool ignore_occupied_areas,
        navscreenoccupied *screenoccupation,
        const GFXColor &col);

void drawlistitem(int type,
        float size,
        float x,
        float y,
        Unit *source,
        navscreenoccupied *screenoccupation,
        bool inmouserange,
        bool currentistail,
        float unselectedalpha,
        GFXColor *factioncolours);

class navdrawlist       //not really a list... it inserts at head, and reads head first. like a stack, but it isnt popping. its just going through it, and whiping.
{
    int n_contents;
    bool inmouserange;
    bool localcolours;
    navdrawnode *head;
    navdrawnode *tail;
    GFXColor *factioncolours;
    navscreenoccupied *screenoccupation;

//this is a restatement of that found in the navscreen
    float screenskipby4[4];     //0 = x-small	1 = x-large	2 = y-small	3 = y-large

//10 sectors for text on the nav screen.
//10 sectors across, divided into 32 sections top/down.

public:
    void insert(int type, float size, float x, float y);
    void insert(int type, float size, float x, float y, Unit *source);
    void wipe();
    void rotate();
    void draw();
    int get_n_contents();
    float unselectedalpha;
    Unit *gettailunit();
    navdrawlist(bool mouse, navscreenoccupied *screenoccupation, GFXColor *factioncolours);
    ~navdrawlist();
};

#endif //VEGA_STRIKE_ENGINE_GFX_NAV_DRAW_LIST_H
