/*
 * Copyright (C) 2001-2022 Daniel Horn, Mike Byron, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
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

#include "newbutton.h"

#include "lin_time.h"                           //For cycling borders.
#include "eventmanager.h"
#include "painttext.h"

//The NewButton class supports the normal button control.  It can be
//pressed by the mouse, and, by default, send a command out when
//it is pressed.

//In variable-color borders, how many color-switches per second.
static const int CYCLE_STEPS_PER_SECOND = 10;

//Draw the button.
void NewButton::draw(void) {
    const float lineWidth = shadowWidth();

    GFXColor currentTextColor;                       //Color of label text.
    switch (m_drawingState) {
        case NORMAL_STATE:
            drawRect(m_rect, color());
            if (m_variableBorderCycleTime > 0.0) {
                drawCycleBorder(lineWidth);
            } else {
                drawLowRightShadow(m_rect, GUI_OPAQUE_BLACK(), lineWidth);
                drawUpLeftShadow(m_rect, GUI_OPAQUE_WHITE(), lineWidth);
            }
            currentTextColor = textColor();
            break;
        case DOWN_STATE: {
            GFXColor currentDownColor = downColor();
            if (isClear(currentDownColor)) {
                currentDownColor = color();
            }
            drawRect(m_rect, currentDownColor);

            currentTextColor = downTextColor();
            if (isClear(currentTextColor)) {
                currentTextColor = textColor();
            }
            if (m_variableBorderCycleTime > 0.0) {
                drawCycleBorder(lineWidth);
            } else {
                drawUpLeftShadow(m_rect, GUI_OPAQUE_BLACK(), lineWidth);
                drawLowRightShadow(m_rect, GUI_OPAQUE_WHITE(), lineWidth);
            }
            break;
        }
        case HIGHLIGHT_STATE:
            drawRect(m_rect, highlightColor());

            currentTextColor = textHighlightColor();
            if (isClear(currentTextColor)) {
                currentTextColor = textColor();
            }
            if (m_variableBorderCycleTime > 0.0) {
                drawCycleBorder(lineWidth);
            } else {
                drawLowRightShadow(m_rect, GUI_OPAQUE_BLACK(), lineWidth);
                drawUpLeftShadow(m_rect, GUI_OPAQUE_WHITE(), lineWidth);
            }
            break;
        case DISABLED_STATE:
            //Just the button -- no shadows.
            if (m_variableBorderCycleTime > 0.0) {
                drawCycleBorder(lineWidth);
            } else {
                drawRect(m_rect, color());
            }
            currentTextColor = textColor();
            break;
    }
    //Draw the button label.
    m_paintText.setRect(m_rect);
    m_paintText.setText(label());
    m_paintText.setFont(font());
    m_paintText.setJustification(CENTER_JUSTIFY);
    m_paintText.setColor(currentTextColor);
    m_paintText.draw();
}

//Draw the cycled border.  Checks time to change colors, etc.
void NewButton::drawCycleBorder(float lineWidth) {
    if (m_cycleStepCount <= 0) {
        //We need to figure out how many steps to use.
        m_cycleStepCount = float_to_int(m_variableBorderCycleTime * CYCLE_STEPS_PER_SECOND);
    }
    const double elapsedTime = getNewTime();
    if (elapsedTime - m_lastStepTime >= m_variableBorderCycleTime / m_cycleStepCount) {
        m_lastStepTime = elapsedTime;

        //It's time.  Change the step in the cycle.
        m_currentCycle += m_cycleDirection;
        if (m_currentCycle <= 0) {
            //At the bottom of the cycle.
            m_currentCycleColor = m_borderColor;
            m_cycleDirection = 1;
            m_cycleColorDelta.r = (m_endBorderColor.r - m_borderColor.r) / m_cycleStepCount;
            m_cycleColorDelta.g = (m_endBorderColor.g - m_borderColor.g) / m_cycleStepCount;
            m_cycleColorDelta.b = (m_endBorderColor.b - m_borderColor.b) / m_cycleStepCount;
            m_cycleColorDelta.a = (m_endBorderColor.a - m_borderColor.a) / m_cycleStepCount;
        } else if (m_currentCycle >= m_cycleStepCount) {
            //At the top of the cycle.
            m_currentCycleColor = m_endBorderColor;
            m_cycleDirection = (-1);
            m_cycleColorDelta.r = (m_borderColor.r - m_endBorderColor.r) / m_cycleStepCount;
            m_cycleColorDelta.g = (m_borderColor.g - m_endBorderColor.g) / m_cycleStepCount;
            m_cycleColorDelta.b = (m_borderColor.b - m_endBorderColor.b) / m_cycleStepCount;
            m_cycleColorDelta.a = (m_borderColor.a - m_endBorderColor.a) / m_cycleStepCount;
        } else {
            //Somewhere in the middle of the cycle.
            m_currentCycleColor.r += m_cycleColorDelta.r;
            m_currentCycleColor.g += m_cycleColorDelta.g;
            m_currentCycleColor.b += m_cycleColorDelta.b;
            m_currentCycleColor.a += m_cycleColorDelta.a;
        }
    }
    //Whew!  Now actually draw it.
    drawRectOutline(m_rect, m_currentCycleColor, lineWidth);
}

//Set the button drawing state.  If the state changes, it will redraw.
void NewButton::setDrawingState(int newState) {
    if (m_drawingState != newState) {
        m_drawingState = newState;
        draw();
    }
}

int NewButton::drawingState(void) {
    return m_drawingState;
}

//This function is called when the button is pressed.
//Override to change the behavior.
void NewButton::sendButtonCommand(void) {
    sendCommand(m_commandId, this);
}

bool NewButton::processMouseDown(const InputEvent &event) {
    if (event.code == LEFT_MOUSE_BUTTON) {
        m_leftPressed = true;             //Remember this for mouse-up.
        setModal(true);                 //Make sure we don't miss anything.
        //Make sure we see mouse events *first* until we get a mouse-up.
        globalEventManager().pushResponder(this);
        setDrawingState(DOWN_STATE);
        return true;
    }
    return Control::processMouseDown(event);
}

bool NewButton::processMouseUp(const InputEvent &event) {
    if (m_leftPressed && event.code == LEFT_MOUSE_BUTTON) {
        //Send the button command if the button goes up inside the button.
        //If not, consider the button action cancelled.
        const bool doCommand = (hitTest(event.loc));

        setDrawingState(NORMAL_STATE);

        //Make sure we get off the event chain.
        globalEventManager().removeResponder(this, true);
        setModal(false);
        m_leftPressed = false;
        //Send the command now, after we've cleaned up the event handling.
        if (doCommand) {
            sendButtonCommand();
        }
        return Control::processMouseUp(event);
    }
    return false;
}

//CONSTRUCTION
NewButton::NewButton(void) :
        m_drawingState(NewButton::NORMAL_STATE),
        m_commandId(),
        m_leftPressed(false),
        m_highlightColor(GUI_OPAQUE_WHITE()),
        m_textHighlightColor(GUI_CLEAR),
        m_downColor(GUI_CLEAR),
        m_downTextColor(GUI_CLEAR),
        m_shadowWidth(1.0),
        m_variableBorderCycleTime(0.0),
        m_borderColor(GUI_OPAQUE_BLACK()),
        m_endBorderColor(GUI_OPAQUE_WHITE()),
        m_currentCycleColor(GUI_OPAQUE_BLACK()),
        m_currentCycle(0),
        m_cycleStepCount(-1),
        m_cycleDirection(0),
        m_cycleColorDelta(),
        m_lastStepTime(0.0) {
}

