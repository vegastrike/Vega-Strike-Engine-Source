/*
 * mouse_cursor.cpp
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

#include "mouse_cursor.h"

#include <string>
#include <iostream>
#include <memory>
#include <boost/format.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "vs_logging.h"
#include "configuration/configuration.h"

// Mouse Cursors
static SDL_Cursor* arrow_cursor = nullptr;
static SDL_Cursor* over_arrow_cursor = nullptr;
static SDL_Cursor* cross_hairs_cursor = nullptr;


static SDL_Cursor* initMouseCursor(const std::string& filename, int hot_x = 0, int hot_y = 0) {
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (!surface) {
        // TODO: figure out error code
        VS_LOG_FLUSH_EXIT(fatal, 
                         (boost::format("IMG_Load failed: %1%\n") % IMG_GetError()).str(), -1);
    }

    // Create a color cursor with transparency support
    SDL_Cursor* cursor = SDL_CreateColorCursor(surface, hot_x, hot_y);
    SDL_FreeSurface(surface);

    return cursor;
}

void initMouseCursors() {
    arrow_cursor = initMouseCursor("sprites/mouse.png");
    over_arrow_cursor = initMouseCursor("sprites/mouseover.png");
    cross_hairs_cursor = initMouseCursor("sprites/crosshairs.png", 64, 64);
}

void freeMouseCursors() {
    SDL_FreeCursor(arrow_cursor);
    SDL_FreeCursor(over_arrow_cursor);
    SDL_FreeCursor(cross_hairs_cursor);
}

void showCursor() {
    SDL_ShowCursor(SDL_ENABLE);
}

void hideCursor() {
    SDL_ShowCursor(SDL_DISABLE);
}

void changeCursor(const CursorType type) {
    switch(type) {
        case CursorType::arrow: SDL_SetCursor(arrow_cursor); break;
        case CursorType::over_arrow: SDL_SetCursor(over_arrow_cursor); break;
        case CursorType::crosshairs: SDL_SetCursor(cross_hairs_cursor); break;
    }
}

// This is simpler code that disregards the mouse sensitivity and returns
// mouse cursor behavior to the standard operating system one.
std::pair<float, float> CalculateRelativeXY(int orig_x, int orig_y) {
    // Center of the window
    float center_x = configuration().graphics.resolution_x / 2;
    float center_y = configuration().graphics.resolution_y / 2;

    // Location of mouse relative to the center
    float relative_x = orig_x - center_x;
    float relative_y = center_y - orig_y;

    // Location of mouse relative to the window center (0,0)
    float fraction_x = relative_x / center_x;
    float fraction_y = relative_y / center_y;

    return std::pair<float,float>(fraction_x, fraction_y);
}

std::pair<int, int> CalculateAbsoluteXY(float fraction_x, float fraction_y) {
    float center_x = configuration().graphics.resolution_x / 2;
    float center_y = configuration().graphics.resolution_y / 2;

    int orig_x = static_cast<int>(center_x + fraction_x * center_x);
    int orig_y = static_cast<int>(center_y - fraction_y * center_y);

    return std::make_pair(orig_x, orig_y);
}

void SetMousePosition(int x, int y) {
    SDL_Window *window = SDL_GL_GetCurrentWindow();
    SDL_WarpMouseInWindow(window, x,y);
}

