/*
 * list_picker.h
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
#ifndef VEGA_STRIKE_LIBRARIES_VEGA_DRAW_LIST_PICKER_H
#define VEGA_STRIKE_LIBRARIES_VEGA_DRAW_LIST_PICKER_H

#include <vector>

#include "vega_draw/picker.h"
#include "vega_draw/widget.h"

namespace vega_draw {

// A selectable list widget wrapping the picker utility. Its rect drives the
// picker region; rows/style/colours are configured directly. Selection is by row
// index.
class ListPicker : public Widget {
public:
    std::vector<PickerRow> &rows() {
        return m_rows;
    }
    const std::vector<PickerRow> &rows() const {
        return m_rows;
    }

    PickerStyle &style() {
        return m_style;
    }
    PickerColors &colors() {
        return m_colors;
    }

    int selectedIndex() const {
        return m_selected;
    }
    void setSelectedIndex(int index) {
        m_selected = index;
    }
    int highlightedIndex() const {
        return m_highlighted;
    }

    float scroll() const {
        return m_scroll;
    }
    void setScroll(float scroll) {
        m_scroll = scroll;
    }

    // Row index under a grid point (needs a viewport + measurer for the layout).
    int rowAt(const Point &grid, const Viewport &viewport, const TextMeasurer &measurer) const;

    void draw(ImDrawList *draw_list, const Viewport &viewport) override;
    bool onMouseDown(const InputEvent &event) override;
    bool onMouseMove(const InputEvent &event) override;

private:
    PickerStyle styleForRect() const;

    std::vector<PickerRow> m_rows;
    PickerStyle m_style;
    PickerColors m_colors;
    int m_selected = -1;
    int m_highlighted = -1;
    float m_scroll = 0.0f;
};

} // namespace vega_draw

#endif // VEGA_STRIKE_LIBRARIES_VEGA_DRAW_LIST_PICKER_H
