/*
 * Copyright (C) 2001-2022 Daniel Horn, David Ranger, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "button.h"
#include "gldrv/winsys.h"
#include <cstdlib>
#include <cstring>

//Button::Button(float x, float y, float wid, float hei, char *name) { ; }
Button::Button(float x, float y, float wid, float hei, const char *name)
{
    //Initialize the variables
    xcoord = x;
    ycoord = y;
    width = wid;
    height = hei;
    label = strdup(name);
    highlight = 0;

    Refresh();
}

void Button::ModifyName(const char *newname)
{
    if (label && newname) {
        free(label);
    }
    if (newname) {
        label = strdup(newname);
    }
}

Button::~Button(void)
{
    if (label != nullptr) {
        free(label);
        label = nullptr;
    }
}

void Button::Refresh(void)
{
    if (highlight == 0) {
        ShowColor(xcoord, ycoord, width, height, 0.51, 0.47, 0.79, 1);
    } else {
        ShowColor(xcoord, ycoord, width, height, 0.66, 0.6, 1, 1);
    }
    ShowColor(0, 0, 0, 0, 0, 0, 0, 1);
    ShowText(xcoord + 0.01, ycoord - height + ((height - 0.04) / 2), width, 4, label, 0);
}

int Button::MouseClick(int button, int state, float x, float y)
{
    if (Inside(x, y) == 0) {
        return 0;
    }
    if (state != WS_MOUSE_UP) {
        return 0;
    }                                      //Returning the 1 says it's been clicked

    return 1;
}

int Button::MouseMove(float x, float y)
{
    if (Inside(x, y) == 0) {
        highlight = 0;
        return 0;
    }
    highlight = 1;
    return 1;
}

int Button::MouseMoveClick(float x, float y)
{
    //Nothing to do
    return 0;
}

int Button::DoMouse(int type, float x, float y, int button, int state)
{
    if (type == 1) {
        return MouseClick(button, state, x, y);
    }
    if (type == 2) {
        return MouseMoveClick(x, y);
    }
    if (type == 3) {
        return MouseMove(x, y);
    }
    return 0;
}

int Button::Inside(float x, float y)
{
    if (x < xcoord || y > ycoord) {
        return 0;
    }
    if (x > (xcoord + width)) {
        return 0;
    }
    if (y < (ycoord - height)) {
        return 0;
    }
    return 1;
}

