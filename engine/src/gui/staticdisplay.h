/*
 * staticdisplay.h
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
#ifndef VEGA_STRIKE_ENGINE_GUI_STATIC_DISPLAY_H
#define VEGA_STRIKE_ENGINE_GUI_STATIC_DISPLAY_H

#include "control.h"
#include "painttext.h"
#include "guitexture.h"

//See cpp file for detailed descriptions of classes, functions, etc.

//FORWARD REFERENCES.
class Scroller;

//The StaticDisplay class is used to show something on a window.
//Right now, it only supports text, but could be expanded to support
//images, textures, meshes, etc.
//This control does not respond to input events.

class StaticDisplay : public Control {
public:
//Draw the control.
    virtual void draw(void);

//Text that appears on the control.
    virtual std::string text(void) {
        return m_paintText.text();
    }

    virtual void setText(std::string t) {
        m_paintText.setText(t);
    }

//Set justification.
    virtual Justification justification(void) {
        return m_paintText.justification();
    }

    virtual void setJustification(Justification j) {
        m_paintText.setJustification(j);
    }

//Whether the text is multi-line.
    virtual bool multiLine(void) {
        return m_paintText.widthExceeded() == PaintText::MULTI_LINE;
    }

    virtual void setMultiLine(bool multi) {
        m_paintText.setWidthExceeded(multi ? PaintText::MULTI_LINE : PaintText::ELLIPSIS);
    }

//Text margins.
    virtual Size textMargins(void) {
        return m_textMargins;
    }

    virtual void setTextMargins(const Size &s);

//Set the object that takes care of scrolling.
    virtual void setScroller(Scroller *s);

//OVERRIDES

//Color of text in control.
    virtual GFXColor textColor(void) {
        return m_paintText.color();
    }

    virtual void setTextColor(const GFXColor &c) {
        m_paintText.setColor(c);
    }

//Font for text in control.
    virtual Font font(void) {
        return m_paintText.font();
    }

    virtual void setFont(const Font &f) {
        m_paintText.setFont(f);
    }

//The outside boundaries of the control.
    virtual void setRect(const Rect &r);

//OVERRIDES - used for scrolling.
    virtual bool processMouseDown(const InputEvent &event);

//Process a command event.
    virtual bool processCommand(const EventCommandId &command, Control *control);

//CONSTRUCTION
public:
    StaticDisplay(void);

    virtual ~StaticDisplay(void) {
    }

protected:
//INTERNAL IMPLEMENTATION

//The rect for the text object has changed -- reset it.
    void setPaintTextRect(void);

//VARIABLES
protected:
    PaintText m_paintText;      //Text object.
    Size m_textMargins;         //Inset area where no text appears.
    int m_scrollPosition;      //Index of first display cell shown.
    int m_layoutVersion;       //A way to tell when the PaintText layout has changed.
    Scroller *m_scroller;
};

class StaticImageDisplay : public Control {
public:
//Draw the control.
    virtual void draw(void) {
        if (texturename.length()) {
            texture.draw(m_rect);
        }
    }

//Text that appears on the control.
    virtual void setTexture(std::string t) {
        texturename = t;
        texture.read(t);
    }

//CONSTRUCTION
public:
    StaticImageDisplay(void) {
    }

    virtual ~StaticImageDisplay(void) {
    }

protected:
    std::string texturename;
    GuiTexture texture;
//INTERNAL IMPLEMENTATION
};

#endif   //VEGA_STRIKE_ENGINE_GUI_STATIC_DISPLAY_H
