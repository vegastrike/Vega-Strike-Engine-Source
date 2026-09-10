/*
 * text_box.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_TEXT_BOX_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_TEXT_BOX_H

#include <string>

#include "vega_draw/text_layout.h"
#include "vega_draw/units.h"

// Text box: a rectangular region that lays out text into lines. Renderer-
// agnostic -- layout is computed with an injected TextMeasurer, so the box is
// unit-tested headlessly; the ImGui adapter draws the result.
//
// Per-box behaviours are orthogonal: word wrap (line breaking), overflow
// clip-or-scroll (vertical), and autofit (font-size solving).
namespace vega_draw {

enum class Overflow {
    Clip,  // default: content beyond the region is clipped (VDU/HUD have no cursor)
    Scroll // opt-in: the owner scrolls by line index (logs, lists, pickers)
};

struct TextBox {
    // Region on the 1000-grid.
    float region_x = 0.0f;
    float region_y = 0.0f;
    float region_w = 1000.0f;
    float region_h = 1000.0f;

    // Text margins (inset) on the 1000-grid.
    float margin_x = 0.0f;
    float margin_y = 0.0f;

    // Font size, authored per-1000 (a fraction of the space height).
    float font_grid = 20.0f;

    bool multiline = true;
    bool wrap = false;             // opt-in; wrap width is the inner region width
    float line_spacing = 0.0f;     // extra spacing between lines (fraction of line height)
    Justification justification = Justification::Left;

    Overflow overflow = Overflow::Clip;
    int scroll_start_line = 0;     // first line drawn when overflow == Scroll

    // Autofit: solve the largest font that fits the region height, within
    // [autofit_min_grid, autofit_max_grid]. Opt-in; solve on set/size-change.
    bool autofit = false;
    float autofit_min_grid = 5.0f;
    float autofit_max_grid = 120.0f;
};

// Value equality (used for change detection by TextBoxLayoutCache).
bool operator==(const TextBox &a, const TextBox &b);
bool operator!=(const TextBox &a, const TextBox &b);

// Inner text region in pixels (the region inset by the margins).
struct BoxRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

// A computed box layout plus the pixel font size it was laid out at (needed to
// draw, and equal to the solved size when autofit is on).
struct TextBoxLayout {
    TextLayout layout;
    float font_px = 0.0f;
};

BoxRect TextBoxInnerRectPx(const TextBox &box, const Viewport &viewport);

// Lay out `markup` inside the box for the given viewport.
TextBoxLayout LayoutTextBox(const TextBox &box,
                            const std::string &markup,
                            const Viewport &viewport,
                            const TextMeasurer &measurer);

// Number of lines, from `start_line` onward, that fully fit in the box's inner
// height.
int TextBoxVisibleLineCount(const TextBox &box,
                            const TextLayout &layout,
                            const Viewport &viewport,
                            int start_line);

// Caches a box's layout and re-computes it only when an input (box config, text,
// viewport) changes -- so an owner can lay out once per change rather than every
// frame. `version()` increments on each recompute, giving a change signal an owner
// (e.g. a scroller) can poll to resync.
class TextBoxLayoutCache {
public:
    const TextBoxLayout &Update(const TextBox &box,
                                const std::string &markup,
                                const Viewport &viewport,
                                const TextMeasurer &measurer);

    int version() const {
        return m_version;
    }

    bool hasLayout() const {
        return m_has_layout;
    }

private:
    TextBox m_box;
    std::string m_markup;
    Viewport m_viewport;
    TextBoxLayout m_layout;
    int m_version = 0;
    bool m_has_layout = false;
};

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_TEXT_BOX_H
