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

#include "newbutton.h"

#include "eventmanager.h"
#include "painttext.h"


// The NewButton class supports the normal button control.  It can be
// pressed by the mouse, and, by default, send a command out when
// it is pressed.

// Draw the button.
void NewButton::draw(void)
{
    const float lineWidth = shadowWidth();

    switch(m_drawingState) {
        case NORMAL_STATE:
            drawRect(m_rect, color());
            drawLowRightShadow(m_rect, GUI_OPAQUE_BLACK, lineWidth);
            drawUpLeftShadow(m_rect, GUI_OPAQUE_WHITE, lineWidth);
            break;
        case DOWN_STATE:
            drawRect(m_rect, color());
            drawUpLeftShadow(m_rect, GUI_OPAQUE_BLACK, lineWidth);
            drawLowRightShadow(m_rect, GUI_OPAQUE_WHITE, lineWidth);
            break;
        case HIGHLIGHT_STATE:
            drawRect(m_rect, highlightColor());
            drawLowRightShadow(m_rect, GUI_OPAQUE_BLACK, lineWidth);
            drawUpLeftShadow(m_rect, GUI_OPAQUE_WHITE, lineWidth);
            break;
        case DISABLED_STATE:
            // Just the button -- no shadows.
            drawRect(m_rect, color());
            break;
    }

    // Draw the button label.
    m_paintText.setRect(m_rect);
    m_paintText.setText(label());
    m_paintText.setFont(font());
    m_paintText.setColor(textColor());
    m_paintText.setJustification(CENTER_JUSTIFY);
    m_paintText.draw();
}

// Set the button drawing state.  If the state changes, it will redraw.
void NewButton::setDrawingState(int newState) {
    if( m_drawingState != newState ) {
        m_drawingState = newState;
        draw();
    }
}

int NewButton::drawingState(void) {
    return m_drawingState;
}

// This function is called when the button is pressed.
// Override to change the behavior.
void NewButton::sendButtonCommand(void) {
    sendCommand(m_commandId, this);
}


bool NewButton::processMouseDown(const InputEvent& event) {
    if(event.code == LEFT_MOUSE_BUTTON) {
        m_leftPressed = true;             // Remember this for mouse-up.
        setModal(true);                   // Make sure we don't miss anything.
        // Make sure we see mouse events *first* until we get a mouse-up.
        globalEventManager().pushResponder(this);
        setDrawingState(DOWN_STATE);
        return true;
    }

    return Control::processMouseDown(event);
}

bool NewButton::processMouseUp(const InputEvent& event) {
    if(m_leftPressed && event.code == LEFT_MOUSE_BUTTON) {
        // Send the button command if the button goes up inside the button.
        //  If not, consider the button action cancelled.
        const bool doCommand = ( hitTest(event.loc) );

        setDrawingState(NORMAL_STATE);

        // Make sure we get off the event chain.
        globalEventManager().removeResponder(this, true);
        setModal(false);
        m_leftPressed = false;

        // Send the command now, after we've cleaned up the event handling.
        if(doCommand) sendButtonCommand();

        return Control::processMouseUp(event);
    }

    return false;
}

// CONSTRUCTION
NewButton::NewButton(void)
:
m_drawingState(NewButton::NORMAL_STATE),
m_commandId(),
m_leftPressed(false),
m_highlightColor( GUI_OPAQUE_WHITE ),
m_shadowWidth(1.0)
{
}
