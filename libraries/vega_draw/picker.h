/*
 * picker.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_PICKER_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_PICKER_H

#include <string>
#include <vector>

#include "vega_draw/markup.h"
#include "vega_draw/text_layout.h"
#include "vega_draw/units.h"

// Picker: a scrollable, selectable list of rows over the text path. Rows may be
// indented by level (grouped/hierarchical lists) and may grow to fit wrapped
// text (variable-height rows). Renderer-agnostic: layout uses an injected
// TextMeasurer; the ImGui adapter draws it and paints per-row selection.
//
// Hierarchy note: the caller supplies an already-flattened row list (expanding
// or collapsing children is the caller's tree walk), with `level` carrying the
// indent -- this matches how the base computer builds its display list.
namespace vega_draw {

struct PickerRow {
    std::string text;    // markup for the row
    int level = 0;       // indent level (0 = none)
    Color text_color;    // optional explicit row colour (set == false -> default)
};

struct PickerStyle {
    // Region on the 1000-grid, inset by margins to give the row viewport.
    float region_x = 0.0f;
    float region_y = 0.0f;
    float region_w = 1000.0f;
    float region_h = 1000.0f;
    float margin_x = 0.0f;
    float margin_y = 0.0f;

    float font_grid = 20.0f;
    float indent_grid = 20.0f;     // indent per level
    float row_padding_grid = 4.0f; // vertical padding added to each row (top and bottom)
    float line_spacing = 0.0f;
    bool wrap_rows = false;        // rows grow to fit wrapped text
};

struct PickerRowLayout {
    int index = 0;             // index into the supplied rows
    float y = 0.0f;            // content-space top (before scrolling)
    float height = 0.0f;
    float indent_px = 0.0f;
    float padding_px = 0.0f;   // top padding inside the row
    Color text_color;
    TextLayout text;
};

struct PickerLayout {
    std::vector<PickerRowLayout> visible_rows;
    float viewport_x = 0.0f;     // top-left of the row viewport, in pixels
    float viewport_y = 0.0f;
    float viewport_width = 0.0f;
    float viewport_height = 0.0f;
    float content_height = 0.0f; // total stacked height of all rows
    float scroll_px = 0.0f;      // clamped scroll actually applied
    float max_scroll_px = 0.0f;
};

PickerLayout LayoutPicker(const PickerStyle &style,
                          const std::vector<PickerRow> &rows,
                          const Viewport &viewport,
                          const TextMeasurer &measurer,
                          float scroll_px = 0.0f);

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_PICKER_H
