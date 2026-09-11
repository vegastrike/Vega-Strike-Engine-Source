/*
 * legacy_text.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_LEGACY_TEXT_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_LEGACY_TEXT_H

#include <string>

#include "vega_draw/markup.h"

// vega_draw: parser for the legacy Vega Strike '#' text format.
//
// This is the format the engine ships today and the one every live consumer
// still authors. It is kept as the engine's active format while the drawing
// path is unified; content is migrated to the HTML-subset format (ParseMarkup)
// later. Holding the interpreter here means there is one '#' parser for the
// whole engine, and switching the engine over later is a single call-site
// change -- grep for ParseLegacyVegaText.
//
// Two dialects exist in the current engine code; both are supported:
//
//   ImGuiText -- GUI controls (base computer, staticDisplay/picker/button,
//       navcomputer, modaldialog). '#cR:G:B[:A]#' colour, '#b[weight]#' stroke,
//       '#-' pop, '#!' reset, '#n' break, '#l' line spacing, '\' line break,
//       '##' for a literal '#'.
//
//   TextPlane -- HUD, VDU, cockpit, briefing, navscreen, navpath. '#RRGGBB'
//       colour ('#000000' resets to the default colour), '#n' break, '_'
//       rendered as a space, and the colour reset at every line break.
//
// Both dialects also accept '#RRGGBB' hex and the '#c'/'#b' stack codes; where
// their behaviour differs, `dialect` selects the historical semantics so a
// migrated consumer renders exactly as it does today. A six-hex sequence is
// always read as a colour before the format codes, so '#b0b0b0'/'#cccccc' are
// colours and not stroke/colour codes.
namespace vega_draw {

enum class LegacyTextDialect {
    ImGuiText,
    TextPlane,
};

// Parse `source`. When `reveal_safe` is true, a trailing token that has not been
// fully typed yet (a lone '#', '#c' with no closing '#', '#-' with no target, or
// '#' followed by fewer than six hex digits) is dropped instead of rendered as
// literal text. A word-by-word reveal grows the string one character at a time,
// so without this a partial colour token flashes on screen; the flag lets a
// reveal path stay clean while the default keeps a literal '#' literal.
TextLines ParseLegacyVegaText(const std::string &source,
                              LegacyTextDialect dialect,
                              bool reveal_safe = false);

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_LEGACY_TEXT_H
