/*
 * scroll_bar.cpp
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
#include "vega_draw/scroll_bar.h"

#include "vega_draw/imgui_backend.h"

namespace vega_draw {

float ScrollBar::axisCoord(const Point &p) const {
    return (m_style.orientation == ScrollOrientation::Vertical) ? p.y : p.x;
}

void ScrollBar::positionToPoint(const Point &p) {
    m_style.track = m_rect;
    m_model.position = ScrollerPositionForThumbCentre(m_style, m_model, axisCoord(p));
}

void ScrollBar::draw(ImDrawList *draw_list, const Viewport &viewport) {
    if (draw_list == nullptr || m_hidden) {
        return;
    }
    m_style.track = m_rect;
    const ScrollerColors colors;
    DrawScroller(draw_list, m_style, m_model, viewport, colors);
}

bool ScrollBar::onMouseDown(const InputEvent &event) {
    if (!hitTest(event.loc)) {
        return false;
    }
    positionToPoint(event.loc);
    m_dragging = true;
    return true;
}

bool ScrollBar::onMouseDrag(const InputEvent &event) {
    if (!m_dragging) {
        return false;
    }
    positionToPoint(event.loc);
    return true;
}

bool ScrollBar::onMouseUp(const InputEvent &) {
    if (!m_dragging) {
        return false;
    }
    m_dragging = false;
    return true;
}

} // namespace vega_draw
