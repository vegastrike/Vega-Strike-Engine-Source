/*
 * picker.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2026 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy, Evert Vorster
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
#include "vega_draw/picker.h"

namespace vega_draw {
namespace {

float inner_x0(const PickerStyle &s, const Viewport &v) {
    return GridToPixelX(s.region_x, v) + GridToPixelW(s.margin_x, v);
}

float inner_y0(const PickerStyle &s, const Viewport &v) {
    return GridToPixelY(s.region_y, v) + GridToPixelH(s.margin_y, v);
}

float inner_width(const PickerStyle &s, const Viewport &v) {
    const float w = GridToPixelW(s.region_w, v) - 2.0f * GridToPixelW(s.margin_x, v);
    return (w > 0.0f) ? w : 0.0f;
}

float inner_height(const PickerStyle &s, const Viewport &v) {
    const float h = GridToPixelH(s.region_h, v) - 2.0f * GridToPixelH(s.margin_y, v);
    return (h > 0.0f) ? h : 0.0f;
}

} // namespace

PickerLayout LayoutPicker(const PickerStyle &style,
                          const std::vector<PickerRow> &rows,
                          const Viewport &viewport,
                          const TextMeasurer &measurer,
                          float scroll_px) {
    PickerLayout result;
    const float font_px = FontGridToPixel(style.font_grid, viewport);
    const float padding = GridToPixelH(style.row_padding_grid, viewport);
    const float extra_height = GridToPixelH(style.extra_row_height_grid, viewport);
    const float indent_step = GridToPixelW(style.indent_grid, viewport);
    const float width = inner_width(style, viewport);
    result.viewport_x = inner_x0(style, viewport);
    result.viewport_y = inner_y0(style, viewport);
    result.viewport_width = width;
    result.viewport_height = inner_height(style, viewport);

    // Measure every row and stack them in content space.
    std::vector<PickerRowLayout> all;
    all.reserve(rows.size());
    float y = 0.0f;
    for (std::size_t i = 0; i < rows.size(); ++i) {
        const PickerRow &row = rows[i];
        const float indent = static_cast<float>(row.level) * indent_step;
        const float available = (width - indent > 0.0f) ? (width - indent) : 0.0f;

        TextStyle text_style;
        text_style.font_px = font_px;
        text_style.wrap = style.wrap_rows && available > 0.0f;
        text_style.wrap_width_px = available;
        text_style.region_width_px = available;
        text_style.line_spacing = style.line_spacing;

        TextLines parsed = ParseMarkup(row.text);
        if (!style.wrap_rows && !parsed.empty()) {
            parsed.resize(1); // a non-wrapping row is a single line
        }
        const TextLayout text = LayoutText(parsed, text_style, measurer);

        PickerRowLayout laid_out;
        laid_out.index = static_cast<int>(i);
        laid_out.id = row.id;
        laid_out.y = y;
        laid_out.height = text.height + 2.0f * padding + extra_height;
        laid_out.indent_px = indent;
        laid_out.padding_px = padding;
        laid_out.text_color = row.text_color;
        laid_out.text = text;
        result.row_tops.push_back(y);
        y += laid_out.height;
        all.push_back(std::move(laid_out));
    }

    result.content_height = y;
    result.max_scroll_px = (result.content_height > result.viewport_height)
                                   ? (result.content_height - result.viewport_height)
                                   : 0.0f;
    result.scroll_px = (scroll_px < 0.0f) ? 0.0f : scroll_px;
    if (result.scroll_px > result.max_scroll_px) {
        result.scroll_px = result.max_scroll_px;
    }

    // Keep only the rows intersecting the viewport.
    const float view_top = result.scroll_px;
    const float view_bottom = result.scroll_px + result.viewport_height;
    for (std::size_t i = 0; i < all.size(); ++i) {
        const PickerRowLayout &row = all[i];
        if (row.y + row.height > view_top && row.y < view_bottom) {
            result.visible_rows.push_back(row);
        }
    }
    return result;
}

int PickerRowAt(const PickerLayout &layout, float y_px) {
    for (std::size_t i = 0; i < layout.visible_rows.size(); ++i) {
        const PickerRowLayout &row = layout.visible_rows[i];
        const float top = layout.viewport_y + row.y - layout.scroll_px;
        if (y_px >= top && y_px < top + row.height) {
            return row.index;
        }
    }
    return -1;
}

float PickerScrollToRow(const PickerStyle &style,
                        const std::vector<PickerRow> &rows,
                        const Viewport &viewport,
                        const TextMeasurer &measurer,
                        int row_index) {
    if (row_index < 0) {
        return 0.0f;
    }
    const PickerLayout layout = LayoutPicker(style, rows, viewport, measurer, 0.0f);
    if (row_index >= static_cast<int>(layout.row_tops.size())) {
        return 0.0f;
    }
    float scroll = layout.row_tops[row_index];
    if (scroll > layout.max_scroll_px) {
        scroll = layout.max_scroll_px;
    }
    if (scroll < 0.0f) {
        scroll = 0.0f;
    }
    return scroll;
}

} // namespace vega_draw
