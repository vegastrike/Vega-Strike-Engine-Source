/*
 * markup.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_MARKUP_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_MARKUP_H

#include <cstdint>
#include <string>
#include <vector>

// vega_draw: the unified drawing/text library. This header is the self-rolled
// HTML-subset markup parser that feeds the text utilities. It is deliberately
// renderer-agnostic (no ImGui, no drawing) so it can be unit-tested on its own.
namespace vega_draw {

// An 8-bit RGBA colour. `set` distinguishes a colour written by a tag from "no
// explicit colour" (the text box applies its own default when set is false).
struct Color {
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255;
    bool set = false;
};

// Stroke-weight presets and the threshold at which a run counts as bold for
// rendering. Weight is continuous (content/config may author values like 1.75);
// how a backend renders a heavier weight is its own decision.
constexpr float kWeightLight = 0.6f;
constexpr float kWeightNormal = 1.0f;
constexpr float kWeightBold = 1.5f;
constexpr float kBoldThreshold = 1.25f;

inline bool IsBoldWeight(float weight) noexcept {
    return weight >= kBoldThreshold;
}

// Inline style of a run.
struct Style {
    float weight = kWeightNormal; // 1.0 = normal; see the presets above
    Color color;                  // color.set == false means no colour tag applied
};

// One contiguous run of text sharing a single style.
struct Run {
    std::string text;
    Style style;
};

// One line of runs. `manual_break` is true when the line was ended by an explicit
// <br> (as opposed to the end of the text), so a wrapping text box knows not to
// reflow a hand-authored break.
struct Line {
    std::vector<Run> runs;
    bool manual_break = false;
};

using TextLines = std::vector<Line>;

// Parse the HTML subset into styled lines.
//
// Grammar (self-rolled HTML subset -- there is no full HTML renderer):
//   <color=RRGGBB> ... </color>   colour span (6 hex; 8 hex RRGGBBAA also accepted)
//   <b> ... </b>                  bold span
//   <br>                          hard line break
//   &lt;  &gt;  &amp;               literal <  >  &   (entities, for escaping)
//
// Spans nest and are block-scoped: a colour applies until its matching </color>
// (or the end of the text), and because the markup carries an explicit close tag,
// the old line-scoped colour behaviour (colour bleeding past a line break) is not
// needed and does not happen unless the content omits a close tag.
//
// Anything that is not a recognised tag is literal text -- in particular a '<'
// that does not begin a known tag is rendered literally (so 'Ship <foo> 4' is
// safe); literal '<' '>' '&' should still be written as &lt; &gt; &amp;.
TextLines ParseMarkup(const std::string &source);

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_MARKUP_H
