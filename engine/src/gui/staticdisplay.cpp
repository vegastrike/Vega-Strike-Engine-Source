/*
 * staticdisplay.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: Mike Byron
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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

#include "src/vegastrike.h"

#include "staticdisplay.h"

#include "guidefs.h"
#include "scroller.h"

#include "root_generic/vs_globals.h"
#include "src/config_xml.h"
#include "root_generic/xml_support.h"

//The StaticDisplay class is used to show something on a window.
//Right now, it only supports text, but could be expanded to support
//images, textures, meshes, etc.
//This control does not respond to input events.

//The rect for the text object has changed -- reset it.
void StaticDisplay::setPaintTextRect(void) {
    const Rect textRect = m_rect.copyAndInset(m_textMargins);
    m_paintText.setRect(textRect);
}

//Set text margins.
void StaticDisplay::setTextMargins(const Size &s) {
    m_textMargins = s;
    setPaintTextRect();
}

//The outside boundaries of the control.
void StaticDisplay::setRect(const Rect &r) {
    Control::setRect(r);
    setPaintTextRect();
}

//Draw the control.
void StaticDisplay::draw(void) {
    //Draw the background.
    drawBackground();
    //If we have a scroller and the layout has changed, need to reset the scroller.
    if (m_scroller && m_layoutVersion != m_paintText.layoutVersion()) {
        const int lineCount = m_paintText.lineCount();
        const int visible = m_paintText.visibleLineCountStartingWith(m_scrollPosition, m_rect.size.height);
        m_scroller->setRangeValues(lineCount - 1, visible);
        if (m_scrollPosition > lineCount - 2 && lineCount > visible) {
            m_scrollPosition = lineCount - 1;
        }
        m_scroller->setScrollPosition(m_scrollPosition);
        m_layoutVersion = m_paintText.layoutVersion();         //Remember layout version for next time.
    }
    m_paintText.drawLines(m_scrollPosition);
}

//Set the object that takes care of scrolling.
void StaticDisplay::setScroller(Scroller *s) {
    m_scroller = s;
    s->setCommandTarget(this);
}

//Process a command event.
bool StaticDisplay::processCommand(const EventCommandId &command, Control *control) {
    if (command == "Scroller::PositionChanged") {
        assert(control == m_scroller);
        m_scrollPosition = m_scroller->scrollPosition();
        return true;
    }
    return Control::processCommand(command, control);
}

//Process wheel events for scrolling.
bool StaticDisplay::processMouseDown(const InputEvent &event) {
    static int zoominc = XMLSupport::parse_int(vs_config->getVariable("general", "wheel_increment_lines", "3"));
    if (m_scroller) {
        if (event.code == WHEELUP_MOUSE_BUTTON) {
            if (hitTest(event.loc)) {
                m_scroller->setScrollPosition(m_scroller->scrollPosition() - zoominc);
            }
        } else if (event.code == WHEELDOWN_MOUSE_BUTTON) {
            if (hitTest(event.loc)) {
                m_scroller->setScrollPosition(m_scroller->scrollPosition() + zoominc);
            }
        }
    }
    return Control::processMouseDown(event);
}

//CONSTRUCTION
StaticDisplay::StaticDisplay(void) :
        m_textMargins(Size(0.0, 0.0)),
        m_scrollPosition(0),
        m_layoutVersion(m_paintText.layoutVersion()),
        m_scroller(NULL) {
}

