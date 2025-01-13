/*
 * in_mouse.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_MOUSE_H
#define VEGA_STRIKE_ENGINE_MOUSE_H

#include "in.h"

extern int mousex, mousey;
int getMouseButtonStatus();  //returns button status that are bitwise anded (i.e. 1,3 down  the number looks like 0x1|(0x1<<2)
void InitMouse();
void RestoreMouse();
void ProcessMouse();
void BindKey(int key, MouseHandler handler);
void UnbindMouse(int key);
int getMouseDrawFunc();
void GetMouseDelta(int &dx, int &dy);
int lookupMouseButton(int winsys_button); //returns 0 for left click, 1 for middle, 2 for right. ....

#endif //VEGA_STRIKE_ENGINE_MOUSE_H
