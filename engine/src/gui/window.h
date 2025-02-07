/*
 * window.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file; Mike Byron specifically
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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
#ifndef VEGA_STRIKE_ENGINE_GUI_WINDOW_H
#define VEGA_STRIKE_ENGINE_GUI_WINDOW_H

#include "eventresponder.h"
#include "guitexture.h"

#include <vector>

//Forward reference.
class Control;
class WindowController;
class GroupControl;

/* The Window class owns an area of the screen.  It manages the
 * controls it contains, doing event handling and drawing.
 */
class Window : public EventResponder {
public:

//The outside boundaries of the window.
    virtual void setRect(const Rect &r);
    void setFullScreen(void);
    void setSizeAndCenter(const Size &size);

//Initially display the window.
//Call this when all the properties and controls of the window are set.
    virtual void open(void);

//Done with the window.  This will normally delete the window object.
    virtual void close(void);

//Manage controls.
    virtual void addControl(Control *c);
    virtual void deleteControl(Control *c);

//Take a control away from this window and save it elsewhere.
    virtual Control *removeControlFromWindow(Control *c);

//Find a control using its id.  NULL returned if none found.
//Note that the control may be hidden.
    virtual Control *findControlById(const std::string &id);

//The background color of the window.
    virtual GFXColor color(void) {
        return m_color;
    }

    virtual void setColor(const GFXColor &c) {
        m_color = c;
    }

//The background texture for the window.
    virtual GuiTexture &texture(void) {
        return m_texture;
    }

    virtual void setTexture(const std::string &textureName) {
        m_texture.read(textureName);
    }

//The color of the outline around the window.
    virtual GFXColor outlineColor(void) {
        return m_outlineColor;
    }

    virtual void setOutlineColor(const GFXColor &c) {
        m_outlineColor = c;
    }

//The width of the outline around the window (in pixels).
    virtual float outlineWidth(void) {
        return m_outlineWidth;
    }

    virtual void setOutlineWidth(float width) {
        m_outlineWidth = width;
    }

//Set up a controller object.
    virtual WindowController *controller(void) {
        return m_controller;
    }

    virtual void setController(WindowController *controller) {
        m_controller = controller;
    }

//Draw the window.
    virtual void draw(void);

//Read window properties and controls from an XML file.
    virtual void readFromXml(const std::string &fileName);

//Set whether to delete this object when it closes.
    virtual bool deleteOnClose(void) {
        return m_deleteOnClose;
    }

    virtual void setDeleteOnClose(bool del) {
        m_deleteOnClose = del;
    }

//OVERRIDES
    virtual bool processMouseDown(const InputEvent &event);
    virtual bool processMouseUp(const InputEvent &event);
    virtual bool processMouseMove(const InputEvent &event);
    virtual bool processMouseDrag(const InputEvent &event);
    virtual bool processCommand(const EventCommandId &command, Control *control);

//CONSTRUCTION
public:
    virtual ~Window();
    Window();

//INTERNAL IMPLEMENTATION
protected:
//Draw window background.
    void drawBackground(void);

//VARIABLES
protected:
    Rect m_rect;       //Rectangle representing window.
    GFXColor m_color;      //Background color of window.
    GFXColor m_outlineColor;           //Color of outline around control.
    float m_outlineWidth;           //Width of outline (in pixels).
    GuiTexture m_texture;                //Background texture.
    bool m_deleteOnClose; //True = delete window object when closed.
    GroupControl *m_controls;   //List of controls that are in this window.
    WindowController *m_controller; //Object that controls this window.
};

//This class keeps track of windows, maintaining z-order and rendering them.
class WindowManager {
public:
    friend class Window;                  //Most operations here are done only by windows.

//Draw all visible windows.
    void draw();

//Shut down all windows.
    void shutDown(void);

protected:
//A new window has been created and is ready to be drawn.
    void openWindow(Window *w);
//A window has been closed.
    void closeWindow(Window *w, bool deleteWindow = true);

//VARIABLES
    std::vector<Window *> m_windows;  //Array of windows.  Last entry is top window.
};

//Get the one window manager.
WindowManager &globalWindowManager(void);

#endif   //VEGA_STRIKE_ENGINE_GUI_WINDOW_H
