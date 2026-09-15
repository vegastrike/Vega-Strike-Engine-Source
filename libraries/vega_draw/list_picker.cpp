/*
 * list_picker.cpp
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
#include "vega_draw/list_picker.h"

#include "vega_draw/imgui_backend.h"

namespace vega_draw {

PickerStyle ListPicker::styleForRect() const {
    PickerStyle style = m_style;
    style.region_x = m_rect.origin.x;
    style.region_y = m_rect.origin.y;
    style.region_w = m_rect.size.width;
    style.region_h = m_rect.size.height;
    return style;
}

int ListPicker::rowAt(const Point &grid, const Viewport &viewport, const TextMeasurer &measurer) const {
    const PickerLayout layout = LayoutPicker(styleForRect(), m_rows, viewport, measurer, m_scroll);
    const float pixel_y = GridToPixelY(grid.y, viewport);
    return PickerRowAt(layout, pixel_y);
}

void ListPicker::draw(ImDrawList *draw_list, const Viewport &viewport) {
    if (draw_list == nullptr || m_hidden) {
        return;
    }
    const ImGuiTextMeasurer measurer;
    DrawPicker(draw_list, styleForRect(), m_rows, viewport, m_colors, m_selected, m_highlighted, m_scroll);
}

bool ListPicker::onMouseDown(const InputEvent &event) {
    if (!hitTest(event.loc)) {
        return false;
    }
    const ImGuiTextMeasurer measurer;
    const int row = rowAt(event.loc, DisplayViewport(), measurer);
    if (row >= 0) {
        m_selected = row;
        return true;
    }
    return false;
}

bool ListPicker::onMouseMove(const InputEvent &event) {
    if (!hitTest(event.loc)) {
        m_highlighted = -1;
        return false;
    }
    const ImGuiTextMeasurer measurer;
    m_highlighted = rowAt(event.loc, DisplayViewport(), measurer);
    return false;
}

} // namespace vega_draw
