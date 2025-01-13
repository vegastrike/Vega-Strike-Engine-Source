/*
 * control.h
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
#ifndef VEGA_STRIKE_ENGINE_GUI_CONTROL_H
#define VEGA_STRIKE_ENGINE_GUI_CONTROL_H

#include "eventresponder.h"
#include "font.h"

//See cpp file for detailed descriptions of classes, functions, etc.

/* The control virtual base class manages a rectangle in a window.
 * It handles input events (like mouse events) if necessary.  It draws the
 * rectangle it owns.
 */
class Control : public EventResponder {
public:

//The outside boundaries of the control.
    virtual Rect rect(void) {
        return m_rect;
    }

    virtual void setRect(const Rect &r) {
        m_rect = r;
    }

//Whether the specified point is inside this control.
    virtual bool hitTest(const Point &p);

//Whether to show the control or not.
    virtual bool hidden(void) {
        return m_hidden;
    }

    virtual void setHidden(bool h = true) {
        m_hidden = h;
    }

//Control have id's.  This makes it easy to find them programmatically.
//See window::findControlById.
    virtual const std::string &id(void) {
        return m_id;
    }

    virtual void setId(const std::string &newId) {
        m_id = newId;
    }

//The color of the control.
//Meaning depends on control.  Often background color.
    virtual GFXColor color(void) {
        return m_color;
    }

    virtual void setColor(const GFXColor &c) {
        m_color = c;
    }

//The color of the outline around the control.
    virtual GFXColor outlineColor(void) {
        return m_outlineColor;
    }

    virtual void setOutlineColor(const GFXColor &c) {
        m_outlineColor = c;
    }

//Color of text in control.
    virtual GFXColor textColor(void) {
        return m_textColor;
    }

    virtual void setTextColor(const GFXColor &c) {
        m_textColor = c;
    }

//Font for text in control.
    virtual Font font(void) {
        return m_font;
    }

    virtual void setFont(const Font &f) {
        m_font = f;
    }

//The list of controls "grouped" into this control.
    virtual bool hasGroupChildren(void) {
        return false;
    }

//Draw the control.
//This should not draw outside its rectangle!
    virtual void draw(void) = 0;

//CONSTRUCTION
public:
    Control(void);

    virtual ~Control(void) {
    }

protected:
//INTERNAL IMPLEMENTATION

//Draw background.
    virtual void drawBackground(void);

//VARIABLES
protected:
    Rect m_rect;         //Boundary rectangle of this control.
    std::string m_id;           //ID of the control.  See window::findControlByName.
    GFXColor m_color;        //Color of control.  Meaning depends on control.
    GFXColor m_outlineColor;         //Color of outline around control.
    GFXColor m_textColor;    //Text color, if control uses text.
    Font m_font;         //Font for the text, if text is needed.
    bool m_hidden;       //False = show the control on the window.
};

#endif   //VEGA_STRIKE_ENGINE_GUI_CONTROL_H
