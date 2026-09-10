/*
 * text_layout.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_TEXT_LAYOUT_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_TEXT_LAYOUT_H

#include <string>
#include <vector>

#include "vega_draw/markup.h"

// Renderer-agnostic text layout: turns parsed lines into positioned runs (px).
// Measurement is injected via TextMeasurer, so this core has no ImGui/font
// dependency and is unit-tested headlessly. A backend adapter supplies a real
// measurer and draws the resulting runs.
namespace vega_draw {

struct TextMetrics {
    float width = 0.0f;
    float height = 0.0f;
};

// Measures text at a pixel font size. Measure("").height gives the line height.
class TextMeasurer {
public:
    virtual ~TextMeasurer() = default;
    virtual TextMetrics Measure(const std::string &text, float font_px) const = 0;
};

enum class Justification {
    Left,
    Center,
    Right
};

struct TextStyle {
    float font_px = 16.0f;
    bool wrap = false;            // opt-in word wrap (never reflows a manual <br>)
    float wrap_width_px = 0.0f;   // breaking width, used only when wrap is on
    float region_width_px = 0.0f; // width used for justification; 0 -> no shift
    float line_spacing = 0.0f;    // extra spacing between lines, as a fraction of the line height
    Justification justification = Justification::Left;
};

// One run placed on a line, x measured from the line's left edge.
struct LaidOutRun {
    std::string text;
    Style style;
    float x = 0.0f;
    float width = 0.0f;
};

// One line placed in the layout, y measured from the top of the layout.
struct LaidOutLine {
    std::vector<LaidOutRun> runs;
    float width = 0.0f;
    float height = 0.0f;
    float y = 0.0f;
};

struct TextLayout {
    std::vector<LaidOutLine> lines;
    float width = 0.0f;  // widest line
    float height = 0.0f; // total height
};

// Lay out parsed lines. Manual <br> breaks are always honoured; word wrap is an
// additional opt-in that only reflows within a parsed line.
TextLayout LayoutText(const TextLines &parsed, const TextStyle &style, const TextMeasurer &measurer);

// Truncate `line` so it fits within `max_width` pixels, appending `ellipsis` if it
// was cut. Advances whole UTF-8 sequences (never splits a multi-byte character).
// Does nothing when the line already fits.
void TruncateLineWithEllipsis(LaidOutLine &line,
                              float max_width,
                              float font_px,
                              const TextMeasurer &measurer,
                              const std::string &ellipsis = "...");

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_TEXT_LAYOUT_H
