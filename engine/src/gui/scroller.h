/*
 * scroller.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
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
#ifndef VEGA_STRIKE_ENGINE_GUI_SCROLLER_H
#define VEGA_STRIKE_ENGINE_GUI_SCROLLER_H

#include "gui/groupcontrol.h"

//See cpp file for detailed descriptions of classes, functions, etc.

//The Scroller class is a Control that offers scroll positioning.
//It has a drag "thumb", and scroll buttons.
//This implementation keeps track of the scroll position internally,
//and uses an integer scroll position.
//The only "event" it communicates to the outside world is a change
//of scroll position.

class Scroller : public GroupControl {
public:
//Parameters for the scrolling range:  Max value, visible values, optional min value.
//The "visible" value sets the "thumb" size.
    void setRangeValues(int max, int visible, int min = 0);

//Scroll position.
//This is some value between min and max.
    int scrollPosition(void) {
        return m_scrollPosition;
    }

    void setScrollPosition(int pos);

//The color of the thumb.
    void setThumbColor(const GFXColor &c, const GFXColor &outline);

//The background color of the buttons.
    void setButtonColor(const GFXColor &c);

//Draw the control.
    virtual void draw(void);

//OVERRIDES

//The outside boundaries of the control.
    virtual void setRect(const Rect &r);

//Background color of control.
    virtual void setColor(const GFXColor &c);

//This is used as the color of the arrows on the scroller buttons.
    virtual void setTextColor(const GFXColor &c);

//Process a command event.
    virtual bool processCommand(const EventCommandId &command, Control *control);

//CONSTRUCTION
public:
    Scroller(void);

    virtual ~Scroller(void) {
    }

protected:
//INTERNAL IMPLEMENTATION

//Calculate the rects for the child controls.
    void calcLayout(void);

//Create the child controls.
    void createControls(void);

//VARIABLES
protected:
    int m_minValue;         //The minimum value in the scrolling range.
    int m_maxValue;         //The maximum value in the scrolling range.
    int m_visible;          //Number of visible cells.  Used for thumb length.
    int m_scrollPosition;   //The current position of this scroller.
    GFXColor m_thumbColor;  //Color to paint the thumb.
    GFXColor m_thumbOutlineColor; //Color of outline for thumb.

    bool m_needLayout;  //True = Need to re-layout the controls.
};

#endif   //VEGA_STRIKE_ENGINE_GUI_SCROLLER_H
