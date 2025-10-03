/*
 * splash_screen.h
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

#ifndef VEGA_STRIKE_LIBRARIES_GUI_SPLASH_SCREEN_H
#define VEGA_STRIKE_LIBRARIES_GUI_SPLASH_SCREEN_H

/** While this is in a widget and it is in the widget subfolder
 * it is not actually a widget in the sense it is part of a greater whole.
 * Instead, it is intended to be used as a standalone imgui element with its window.
 */

#include <string>
#include "SDL2/SDL.h"

extern void (*displayTexturePtr)(int);
void RenderSplashScreen(const std::string message, int width, int height);

#endif // VEGA_STRIKE_LIBRARIES_GUI_SPLASH_SCREEN_H