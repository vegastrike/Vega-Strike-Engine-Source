/*
 * gui.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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

#ifndef VEGA_STRIKE_LIBRARIES_GUI_GUI_H
#define VEGA_STRIKE_LIBRARIES_GUI_GUI_H

class SDL_Window;

/** Init and cleanup ImGui.
 */
extern bool gui_initialized;
extern SDL_Window* current_window;

void InitGui(SDL_Window *window, const SDL_GLContext *context, float fontSize, const char *fontFile = nullptr);
void CleanupGui();

/** Request that the ImGui font atlas be rebuilt at the given pixel size on the
 *  next frame (used by settings changes; no-op until the next frame).
 */
void RequestImGuiFontSize(float fontSize);
/** Request that the ImGui font be swapped to the given .ttf file (path into the
 *  data fonts/ directory) on the next frame; pass nullptr/empty to use Roboto.
 *  Used by settings changes; no-op until the next frame.
 */
void RequestImGuiFont(const char *fontFile);
/** Apply a pending font-size/font change by rebuilding the font atlas. Call at the
 *  start of each ImGui frame, before ImGui::NewFrame(). No-op when nothing pending.
 */
void ImGui_ApplyPendingFontSize();

#endif // VEGA_STRIKE_LIBRARIES_GUI_GUI_H