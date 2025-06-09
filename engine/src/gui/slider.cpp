/*
 * slider.cpp
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

#include "slider.h"

#include "eventmanager.h"

#include "root_generic/vs_globals.h"
#include "src/config_xml.h"
#include "root_generic/xml_support.h"

//The Slider class controls the setting for a simple integer range.

//These limits are in thumb length units -- 0.0 -> 1.0.
static const float MAX_THUMB_LENGTH = 1.0;      //Can't have a thumb this long.
static const float MIN_THUMB_LENGTH = .05;      //Make the thumb at least this thick.
static const float NO_THUMB_LENGTH = 1.0;      //If we don't have a thumb, this is the length.

//This is absolute -- thumbs must be at least this big in coordinates.
static const float ABSOLUTE_MIN_THUMB_LENGTH = .05;

//Margins for cancelling a thumb drag operation.
//That is, if you drag the mouse out of this area, the slider position goes back
//to it's original position before the drag aperation started.
//These are margins outside the slider rectangle.  They are negative because they are used to
//"inset" the rectangle.
//NOTE: This is used for both vertical and horizontal.  Margins should be the same.
static const Size THUMB_DRAG_CANCEL_MARGINS = Size(-.3, -.3);

//Set the position of this scroller.
void Slider::setPosition(int pos) {
    int newPosition = pos;
    if (pos > m_maxValue) {
        newPosition = m_maxValue;
    } else if (pos < m_minValue) {
        newPosition = m_minValue;
    }
    if (m_position != newPosition) {
        m_position = newPosition;
        sendCommand("Slider::PositionChanged", this);
    }
}

//Range represented by the slider.
void Slider::setMaxMin(int max, int min) {
    m_maxValue = max;
    m_minValue = min;

    //Set default page size
    const int pageSize = float_to_int((max - min) / 10.0 + 0.5);
    setPageSize(pageSize);
}

//"Thumb" length.  1.0 = Entire range.
void Slider::setThumbLength(float len) {
    float realLen = m_originalThumbLength = len;
    if (realLen > MAX_THUMB_LENGTH || realLen < 0.0) {
        realLen = NO_THUMB_LENGTH;
    } else if (realLen < MIN_THUMB_LENGTH) {
        realLen = MIN_THUMB_LENGTH;
    }
    //Make sure the thumb is at least an absolute size.
    if (m_vertical) {
        if (realLen * m_rect.size.height < ABSOLUTE_MIN_THUMB_LENGTH && m_rect.size.height > 0) {
            realLen = ABSOLUTE_MIN_THUMB_LENGTH / m_rect.size.height;
        }
    } else if (realLen * m_rect.size.width < ABSOLUTE_MIN_THUMB_LENGTH && m_rect.size.height > 0) {
        realLen = ABSOLUTE_MIN_THUMB_LENGTH / m_rect.size.width;
    }
    m_thumbLength = realLen;
}

//The outside boundaries of the control.
void Slider::setRect(const Rect &r) {
    Control::setRect(r);

    //Calculate other things based on new rect.
    m_vertical = (m_rect.size.height >= m_rect.size.width);
    setThumbLength(m_originalThumbLength);
}

//Whether color is light or dark.
static bool isColorLight(const GFXColor &c) {
    return c.r > .6 || c.g > .6 || c.b > .6;
}

//Make a color somewhat darker.
static GFXColor darkenColor(const GFXColor &c, const float factor) {
    GFXColor result;
    result.r = guiMax(0, c.r - factor);
    result.g = guiMax(0, c.g - factor);
    result.b = guiMax(0, c.b - factor);
    result.a = c.a;

    return result;
}

//Make a color somewhat darker.
static GFXColor lightenColor(const GFXColor &c, const float factor) {
    GFXColor result;
    result.r = guiMin(1.0, c.r + factor);
    result.g = guiMin(1.0, c.g + factor);
    result.b = guiMin(1.0, c.b + factor);
    result.a = c.a;

    return result;
}

//Calculate the thumb colors based on the specified background color.
void Slider::setThumbColorBasedOnColor(const GFXColor &c) {
    if (!isClear(c)) {
        if (isColorLight(c)) {
            //Light color.  Make thumb darker.
            setThumbColor(darkenColor(c, .3), GUI_OPAQUE_WHITE());
        } else {
            //Dark Color.
            setThumbColor(lightenColor(c, .3), GUI_OPAQUE_WHITE());
        }
    }
}

//Set the background color.
void Slider::setColor(const GFXColor &c) {
    //Calculate a reasonable thumb color.
    setThumbColorBasedOnColor(c);

    Control::setColor(c);
}

//Draw the control.
void Slider::draw(void) {
    //Draw the background.
    drawBackground();
    //Draw the thumb.
    if (!(isClear(m_thumbColor) && isClear(m_thumbOutlineColor))) {
        float relativePosition = 0.0;
        if (m_maxValue != m_minValue) {
            relativePosition = ((float) (m_position - m_minValue)) / (m_maxValue - m_minValue);
        }
        Rect thumbRect = m_rect;
        if (m_vertical) {
            //Vertical thumb.
            thumbRect.size.height = m_thumbLength * m_rect.size.height;
            thumbRect.origin.y += (1.0 - relativePosition) * (m_rect.size.height - thumbRect.size.height);
        } else {
            //Horizontal thumb.
            thumbRect.size.width = m_thumbLength * m_rect.size.width;
            thumbRect.origin.x += relativePosition * (m_rect.size.width - thumbRect.size.width);
        }
        m_thumbRect = thumbRect;            //Want to save away the bigger version for mouse hits.
        thumbRect.inset(Size(.01, .01));
        drawRect(thumbRect, m_thumbColor);
        drawRectOutline(thumbRect, m_thumbOutlineColor, 1.0);
    }
}

bool Slider::processMouseDown(const InputEvent &event) {
    static int zoominc = XMLSupport::parse_int(vs_config->getVariable("general", "wheel_increment_lines", "3"));
    if (event.code == LEFT_MOUSE_BUTTON && m_thumbLength != NO_THUMB_LENGTH && hitTest(event.loc)) {
        if (m_vertical) {
            if (event.loc.y < m_thumbRect.origin.y) {
                m_mouseState = MOUSE_PAGE_DOWN;
            } else if (event.loc.y > m_thumbRect.top()) {
                m_mouseState = MOUSE_PAGE_UP;
            } else {
                m_mouseState = MOUSE_THUMB_DRAG;
                m_buttonDownMouse = event.loc.y;
                m_buttonDownPosition = m_position;
            }
        } else {
            if (event.loc.x < m_thumbRect.origin.x) {
                m_mouseState = MOUSE_PAGE_UP;
            } else if (event.loc.x > m_thumbRect.right()) {
                m_mouseState = MOUSE_PAGE_DOWN;
            } else {
                m_mouseState = MOUSE_THUMB_DRAG;
                m_buttonDownMouse = event.loc.x;
                m_buttonDownPosition = m_position;
            }
        }
        setModal(true);                 //Make sure we don't miss anything.
        //Make sure we see mouse events *first* until we get a mouse-up.
        globalEventManager().pushResponder(this);
        return true;
    } else if (event.code == WHEELUP_MOUSE_BUTTON) {
        if (hitTest(event.loc)) {
            setPosition(position() - zoominc);
        }
    } else if (event.code == WHEELDOWN_MOUSE_BUTTON) {
        if (hitTest(event.loc)) {
            setPosition(position() + zoominc);
        }
    }
    return Control::processMouseDown(event);
}

bool Slider::processMouseDrag(const InputEvent &event) {
    //The interface for mouse dragging is a little weird.  There is no button information.  All
    //we know is that some button is down.  This is enough, since we don't get into a specific
    //mouse state in this control unless we know which mouse button was pressed...
    if (m_mouseState == MOUSE_THUMB_DRAG) {
        if (m_thumbLength == NO_THUMB_LENGTH) {
            return true;
        }
        const Rect cancelRect = m_rect.copyAndInset(THUMB_DRAG_CANCEL_MARGINS);
        if (!cancelRect.inside(event.loc)) {
            //We are outside the cancel rect.  Go back to original position.
            setPosition(m_buttonDownPosition);
            return true;
        } else {
            //We are dragging the thumb -- get a new scroll position.
            if (m_vertical) {
                //Calculate the factor to convert a change in mouse coords to a change in slider position.
                //This is derived from the ratio of the non-thumb length in the slider to the
                //total range.
                const float totalMouseLength = (1.0 - m_thumbLength) * m_rect.size.height;
                const int totalRange = m_maxValue - m_minValue;
                const int positionChange =
                        float_to_int((m_buttonDownMouse - event.loc.y) * totalRange / totalMouseLength + 0.5);
                setPosition(m_buttonDownPosition + positionChange);
                return true;
            } else {
                const float totalMouseLength = (1.0 - m_thumbLength) * m_rect.size.width;
                const int totalRange = m_maxValue - m_minValue;
                const int positionChange =
                        float_to_int((event.loc.x - m_buttonDownMouse) * totalRange / totalMouseLength + 0.5);
                setPosition(m_buttonDownPosition + positionChange);
                return true;
            }
        }
        return Control::processMouseDrag(event);
    }
    return false;
}

bool Slider::processMouseUp(const InputEvent &event) {
    if (m_mouseState != MOUSE_NONE && event.code == LEFT_MOUSE_BUTTON) {
        //We are now done with the modal mouse loop.
        switch (m_mouseState) {
            case MOUSE_PAGE_UP:
                if (hitTest(event.loc)) {
                    setPosition(position() - m_pageSize);
                }
                break;
            case MOUSE_PAGE_DOWN:
                if (hitTest(event.loc)) {
                    setPosition(position() + m_pageSize);
                }
                break;
            case MOUSE_THUMB_DRAG:
                //Whatever position we have is the correct one.
                //Do nothing.
                break;
            default:
                assert(false);             //Forgot an enum case.
        }
        //Make sure we get off the event chain.
        globalEventManager().removeResponder(this, true);
        setModal(false);
        m_mouseState = MOUSE_NONE;

        return Control::processMouseUp(event);
    }
    return false;
}

//CONSTRUCTION
Slider::Slider(void) :
        m_minValue(0),
        m_maxValue(100),
        m_thumbLength(.15),
        m_originalThumbLength(m_thumbLength),
        m_pageSize(10),
        m_thumbColor(m_color),
        m_thumbOutlineColor(GUI_OPAQUE_BLACK()),
        m_position(m_minValue),
        m_vertical(true),
        m_mouseState(MOUSE_NONE),
        m_buttonDownMouse(0.0),
        m_buttonDownPosition(0),
        m_thumbRect() {
}

