/*
 * display.h
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

// Check to see if we're already loaded
#ifndef VEGA_STRIKE_ENGINE_SETUP_DISPLAY_H
#define VEGA_STRIKE_ENGINE_SETUP_DISPLAY_H

#include "central.h"
#include "version.h"

void InitGraphics(int *, char ***);
void ShowMain();

#define GET_TITLE char title[1000]="Vega Strike Settings"; \
  if (strlen(CONFIG.program_name)+strlen(VERSION)<800) { \
    sprintf(title, "Settings - %s - Version %s", \
    CONFIG.program_name, VEGASTRIKE_VERSION_STR); \
  }

#define GET_STATIC_TEXT char static_text[1000]="Vega Strike requires \
the latest drivers for your video card.\nIf you run into problems please \
upgrade your video drivers.\n\nTo adjust volume levels in-game, use F9/F10 \
for sound and F11/F12 for music."

#endif    //VEGA_STRIKE_ENGINE_SETUP_DISPLAY_H
