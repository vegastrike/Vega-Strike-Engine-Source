/*
 * imgui_support.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy, Danny Gehl
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

/**
 * This util class contains helpers which can be used in the event loops
 */

#include <boost/parameter/aux_/void.hpp>
#include <vector>
using std::vector;

enum MousePointerStyle {
    MOUSE_POINTER_NONE,
    MOUSE_POINTER_NORMAL,
    MOUSE_POINTER_HOVER
};

void SetSoftwareMousePosition(int x, int y);
void StartGUIFrame(void);
void DrawMouseCursor(MousePointerStyle pointerStyle);
void EndGUIFrame(MousePointerStyle pointerStyle);
