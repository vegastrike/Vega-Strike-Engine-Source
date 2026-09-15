/*
 * slider.cpp
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
#include "vega_draw/slider.h"

#include "imgui/imgui.h"

#include "vega_draw/imgui_backend.h"

namespace vega_draw {
namespace {

ImU32 ToU32(const Color &c) {
    return IM_COL32(c.r, c.g, c.b, c.a);
}

float axis_coord(const Point &p, ScrollOrientation orientation) {
    return (orientation == ScrollOrientation::Vertical) ? p.y : p.x;
}

} // namespace

void Slider::setRange(float min, float max) {
    m_min = min;
    m_max = max;
    setValue(m_value);
}

void Slider::setValue(float value) {
    if (m_max <= m_min) {
        m_value = m_min;
        return;
    }
    if (value < m_min) {
        value = m_min;
    }
    if (value > m_max) {
        value = m_max;
    }
    m_value = value;
}

float Slider::normalized() const {
    if (m_max <= m_min) {
        return 0.0f;
    }
    return (m_value - m_min) / (m_max - m_min);
}

Rect Slider::thumbRect() const {
    const float axis_length = (m_style.orientation == ScrollOrientation::Vertical) ? m_rect.size.height
                                                                                  : m_rect.size.width;
    float thumb = m_style.thumb_grid;
    if (thumb > axis_length) {
        thumb = axis_length;
    }
    const float travel = axis_length - thumb;
    if (m_style.orientation == ScrollOrientation::Vertical) {
        return Rect{m_rect.origin.x, m_rect.origin.y + normalized() * travel, m_rect.size.width, thumb};
    }
    return Rect{m_rect.origin.x + normalized() * travel, m_rect.origin.y, thumb, m_rect.size.height};
}

void Slider::valueFromPoint(const Point &p) {
    const float start = (m_style.orientation == ScrollOrientation::Vertical) ? m_rect.origin.y : m_rect.origin.x;
    const float length = (m_style.orientation == ScrollOrientation::Vertical) ? m_rect.size.height : m_rect.size.width;
    if (length <= 0.0f) {
        return;
    }
    float n = (axis_coord(p, m_style.orientation) - start) / length;
    if (n < 0.0f) {
        n = 0.0f;
    }
    if (n > 1.0f) {
        n = 1.0f;
    }
    setValue(m_min + n * (m_max - m_min));
}

void Slider::draw(ImDrawList *draw_list, const Viewport &viewport) {
    if (draw_list == nullptr || m_hidden) {
        return;
    }
    DrawRectFill(draw_list, m_rect, viewport, ToU32(m_style.track));
    if (m_style.border.a > 0) {
        DrawRectOutline(draw_list, m_rect, viewport, ToU32(m_style.border), m_style.border_px);
    }
    DrawRectFill(draw_list, thumbRect(), viewport, ToU32(m_style.thumb));
}

bool Slider::onMouseDown(const InputEvent &event) {
    if (!hitTest(event.loc)) {
        return false;
    }
    valueFromPoint(event.loc);
    m_dragging = true;
    return true;
}

bool Slider::onMouseDrag(const InputEvent &event) {
    if (!m_dragging) {
        return false;
    }
    valueFromPoint(event.loc);
    return true;
}

bool Slider::onMouseUp(const InputEvent &) {
    if (!m_dragging) {
        return false;
    }
    m_dragging = false;
    return true;
}

} // namespace vega_draw
