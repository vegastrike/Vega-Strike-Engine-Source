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

#include "staticdisplay.h"

#include "guidefs.h"
#include "scroller.h"


// The StaticDisplay class is used to show something on a window.
// Right now, it only supports text, but could be expanded to support
//  images, textures, meshes, etc.
// This control does not respond to input events.

// The rect for the text object has changed -- reset it.
void StaticDisplay::setPaintTextRect(void) {
    const Rect textRect = m_rect.copyAndInset(m_textMargins);
    m_paintText.setRect(textRect);
}

// Set text margins.
void StaticDisplay::setTextMargins(const Size& s) {
    m_textMargins = s;
    setPaintTextRect();
}

// The outside boundaries of the control.
void StaticDisplay::setRect(const Rect& r) {
    Control::setRect(r);
    setPaintTextRect();
}

// Draw the control. Return true if anything is drawn.
bool StaticDisplay::draw(void)
{
    // Draw the background.
    if(!isClear(m_color)) {
        drawRect(m_rect, m_color);
    }

    // If we have a scroller and the layout has changed, need to reset the scroller.
    if(m_scroller && m_layoutVersion != m_paintText.layoutVersion()) {
        const int lineCount = m_paintText.lineCount();
        const int visible = m_paintText.visibleLineCountStartingWith(m_scrollPosition, m_rect.size.height);
        m_scroller->setRangeValues(lineCount-1, visible);
        m_scrollPosition = 0;
        m_scroller->setScrollPosition(m_scrollPosition);
        m_layoutVersion = m_paintText.layoutVersion();      // Remember layout version for next time.
    }

    m_paintText.drawLines(m_scrollPosition);

    return true;
}

// Set the object that takes care of scrolling.
void StaticDisplay::setScroller(Scroller* s) {
    m_scroller = s;
    s->setCommandTarget(this);
}

// Process a command event.
bool StaticDisplay::processCommand(const EventCommandId& command, Control* control) {
    if(command == "Scroller::PositionChanged") {
        assert(control == m_scroller);
        m_scrollPosition = m_scroller->scrollPosition();
        return true;
    }

    return Control::processCommand(command, control);
}

// CONSTRUCTION
StaticDisplay::StaticDisplay(void)
:
m_scroller(NULL),
m_scrollPosition(0),
m_layoutVersion(m_paintText.layoutVersion()),
m_textMargins(Size(0.0,0.0))
{
}
