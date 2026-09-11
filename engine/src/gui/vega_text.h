/*
 * vega_text.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file.
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
#ifndef VEGA_STRIKE_ENGINE_GUI_VEGA_TEXT_H
#define VEGA_STRIKE_ENGINE_GUI_VEGA_TEXT_H

#include <string>

class TextPlane;

// Bridge that renders a legacy TextPlane's text through the vega_draw text
// pipeline. The text is parsed with the legacy '#' interpreter (TextPlane
// dialect), laid out at the font size the engine draws at (the user's Text
// Height), and drawn at the plane's position, wrapped to its width and placed
// against its layout resolution/letterbox offset.
//
// Transitional: used while the base room / main menu text migrates off
// TextPlane. HUD/VDU still render through TextPlane.
namespace vega_text {

// `transparent` replaces the old "automatte" flag: when true the text is drawn
// with no background; when false the plane's background colour is painted
// behind each run.
void DrawTextPlane(TextPlane &plane, const std::string &text, bool transparent);

} // namespace vega_text

#endif // VEGA_STRIKE_ENGINE_GUI_VEGA_TEXT_H
