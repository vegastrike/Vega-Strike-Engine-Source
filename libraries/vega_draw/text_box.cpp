/*
 * text_box.cpp
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
#include "vega_draw/text_box.h"

#include "vega_draw/markup.h"

namespace vega_draw {

bool operator==(const TextBox &a, const TextBox &b) {
    return a.region_x == b.region_x && a.region_y == b.region_y && a.region_w == b.region_w &&
           a.region_h == b.region_h && a.margin_x == b.margin_x && a.margin_y == b.margin_y &&
           a.font_grid == b.font_grid && a.multiline == b.multiline && a.wrap == b.wrap &&
           a.line_spacing == b.line_spacing && a.justification == b.justification &&
           a.overflow == b.overflow && a.scroll_start_line == b.scroll_start_line &&
           a.autofit == b.autofit && a.autofit_min_grid == b.autofit_min_grid &&
           a.autofit_max_grid == b.autofit_max_grid;
}

bool operator!=(const TextBox &a, const TextBox &b) {
    return !(a == b);
}

BoxRect TextBoxInnerRectPx(const TextBox &box, const Viewport &viewport) {
    const float inset_x = GridToPixelW(box.margin_x, viewport);
    const float inset_y = GridToPixelH(box.margin_y, viewport);
    const float x0 = GridToPixelX(box.region_x, viewport) + inset_x;
    const float y0 = GridToPixelY(box.region_y, viewport) + inset_y;
    const float x1 = GridToPixelX(box.region_x + box.region_w, viewport) - inset_x;
    const float y1 = GridToPixelY(box.region_y + box.region_h, viewport) - inset_y;

    BoxRect rect;
    rect.x = x0;
    rect.y = y0;
    rect.width = (x1 > x0) ? (x1 - x0) : 0.0f;
    rect.height = (y1 > y0) ? (y1 - y0) : 0.0f;
    return rect;
}

namespace {

TextLayout BuildLayout(const TextBox &box,
                       const TextLines &parsed,
                       const Viewport &viewport,
                       const TextMeasurer &measurer,
                       float font_grid,
                       float inner_width_px) {
    TextStyle style;
    style.font_px = FontGridToPixel(font_grid, viewport);
    style.wrap = box.wrap && box.multiline && inner_width_px > 0.0f;
    style.wrap_width_px = inner_width_px;
    style.region_width_px = inner_width_px;
    style.line_spacing = box.line_spacing;
    style.justification = box.justification;
    return LayoutText(parsed, style, measurer);
}

} // namespace

TextBoxLayout LayoutTextBox(const TextBox &box,
                            const std::string &markup,
                            const Viewport &viewport,
                            const TextMeasurer &measurer) {
    const BoxRect inner = TextBoxInnerRectPx(box, viewport);

    TextLines parsed = ParseMarkup(markup);
    if (!box.multiline && !parsed.empty()) {
        parsed.resize(1); // a single-line box shows only the first line
    }

    float font_grid = box.font_grid;
    if (box.autofit) {
        font_grid = box.autofit_min_grid;
        for (float candidate = box.autofit_max_grid; candidate >= box.autofit_min_grid; candidate -= 1.0f) {
            const TextLayout layout = BuildLayout(box, parsed, viewport, measurer, candidate, inner.width);
            if (layout.height <= inner.height) {
                font_grid = candidate;
                break;
            }
        }
    }

    TextBoxLayout result;
    result.font_px = FontGridToPixel(font_grid, viewport);
    result.layout = BuildLayout(box, parsed, viewport, measurer, font_grid, inner.width);

    // A single-line box shows one line and ellipsises it if it overflows.
    if (!box.multiline && !result.layout.lines.empty()) {
        TruncateLineWithEllipsis(result.layout.lines[0], inner.width, result.font_px, measurer);
        result.layout.width = result.layout.lines[0].width;
    }
    return result;
}

int TextBoxVisibleLineCount(const TextBox &box,
                            const TextLayout &layout,
                            const Viewport &viewport,
                            int start_line) {
    const BoxRect inner = TextBoxInnerRectPx(box, viewport);
    float remaining = inner.height;
    int count = 0;
    for (int i = (start_line < 0 ? 0 : start_line); i < static_cast<int>(layout.lines.size()); ++i) {
        const float h = layout.lines[i].height;
        if (h > remaining + 0.5f) {
            break;
        }
        remaining -= h;
        ++count;
    }
    return count;
}

const TextBoxLayout &TextBoxLayoutCache::Update(const TextBox &box,
                                                const std::string &markup,
                                                const Viewport &viewport,
                                                const TextMeasurer &measurer) {
    if (!m_has_layout || box != m_box || markup != m_markup || viewport != m_viewport) {
        m_layout = LayoutTextBox(box, markup, viewport, measurer);
        m_box = box;
        m_markup = markup;
        m_viewport = viewport;
        ++m_version;
        m_has_layout = true;
    }
    return m_layout;
}

} // namespace vega_draw
