/* 
 * Vega Strike
 * Copyright (C) 2003 Mike Byron
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "vegastrike.h"

#include "control.h"


// Whether the specified point is inside this control.
bool Control::hitTest(const Point& p) {
    const float maxX = m_rect.origin.x + m_rect.size.width;
    const float maxY = m_rect.origin.y + m_rect.size.height;

    return(p.x >= m_rect.origin.x && p.x < maxX && p.y >= m_rect.origin.y && p.y < maxY);
}

// CONSTRUCTION

Control::Control(void)
:
m_rect(0.0,0.0,0.0,0.0),
m_id(),
m_color( GUI_OPAQUE_WHITE ),
m_textColor( GUI_OPAQUE_BLACK ),
m_font(0.1),
m_hidden(false)
{
}
