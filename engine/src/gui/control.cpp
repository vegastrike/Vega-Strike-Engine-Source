/*
 * Copyright (C) 2001-2022 Daniel Horn, Mike Byron, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "vegastrike.h"

#include "control.h"

//Line width for control outline.
static const float DEFAULT_OUTLINE_LINE_WIDTH = 1.0;

//Whether the specified point is inside this control.
bool Control::hitTest(const Point &p)
{
    const float maxX = m_rect.origin.x + m_rect.size.width;
    const float maxY = m_rect.origin.y + m_rect.size.height;

    return p.x >= m_rect.origin.x && p.x < maxX && p.y >= m_rect.origin.y && p.y < maxY;
}

//Draw window background.
void Control::drawBackground(void)
{
    if (!isClear(m_color)) {
        drawRect(m_rect, m_color);
    }
    if (!isClear(m_outlineColor)) {
        drawRectOutline(m_rect, m_outlineColor, DEFAULT_OUTLINE_LINE_WIDTH);
    }
}

//CONSTRUCTION
Control::Control(void) :
        m_rect(0.0, 0.0, 0.0, 0.0),
        m_id(),
        m_color(GUI_OPAQUE_WHITE()),
        m_outlineColor(GUI_CLEAR),
        m_textColor(GUI_OPAQUE_BLACK()),
        m_font(0.1),
        m_hidden(false)
{
}

