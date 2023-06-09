/*
 * Copyright (C) 2001-2023 David Ranger, Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
 * and other Vega Strike contributors.
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
#ifndef VEGA_STRIKE_ENGINE_GUI_GLUT_SUPPORT_H
#define VEGA_STRIKE_ENGINE_GUI_GLUT_SUPPORT_H

/* If you have functions that perform the same operation, but use different parameters,
 * It may be best if you replace the following functions with wrappers to your own functions
 */
#include <stdio.h>
#include <vector>
#include "gfxlib.h"
using std::vector;

enum MousePointerStyle {
    MOUSE_POINTER_NONE,
    MOUSE_POINTER_NORMAL,
    MOUSE_POINTER_HOVER
};

void ShowColor(float x, float y, float wid, float hei, float red, float green, float blue, float alpha);
void ShowText(float x, float y, float wid, int size, const char *string, int no_end);
float WidthOfChar(char chr);
void SetSoftwareMousePosition(int x, int y);
void StartGUIFrame(GFXBOOL clear = GFXTRUE);
void DrawGlutMouse(int mousex, int mousey, class VSSprite *spr);
void EndGUIFrame(MousePointerStyle pointerStyle);
extern int HAS_ALPHA;

#endif    //VEGA_STRIKE_ENGINE_GUI_GLUT_SUPPORT_H
