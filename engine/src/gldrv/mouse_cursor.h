/*
 * mouse_cursor.h
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

#ifndef VEGA_STRIKE_ENGINE_GLDRV_MOUSE_CURSOR_H
#define VEGA_STRIKE_ENGINE_GLDRV_MOUSE_CURSOR_H

#include <utility>

enum class CursorType {
    arrow, over_arrow, crosshairs
};

void initMouseCursors();
void freeMouseCursors();
void showCursor();
void hideCursor();
void changeCursor(const CursorType type);
std::pair<float, float> CalculateRelativeXY(int orig_x, int orig_y);
std::pair<int, int> CalculateAbsoluteXY(float fraction_x, float fraction_y);
std::pair<double, double> GetJoystickFromMouse();
std::pair<int, int> GetMousePosition();
void SetMousePosition(int x, int y);

#endif //VEGA_STRIKE_ENGINE_GLDRV_MOUSE_CURSOR_H