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

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "gui/eventresponder.h"

#include <vector>


// Forward reference.
class Control;
class WindowController;
class GroupControl;

/* The Window class owns an area of the screen.  It manages the
 * controls it contains, doing event handling and drawing.
 */
class Window : public EventResponder
{
public:

    // The outside boundaries of the window.
    virtual void setRect(const Rect& r);
    void setFullScreen(void);
    void setSizeAndCenter(const Size& size);

    // Initially display the window.
    // Call this when all the properties and controls of the window are set.
    virtual void open(void);

    // Done with the window.  This will normally delete the window object.
    virtual void close(void);

    // Manage controls.
    virtual void addControl(Control* c);
    virtual void deleteControl(Control* c);

    // Take a control away from this window and save it elsewhere.
    virtual Control* removeControlFromWindow(Control* c);

    // Find a control using its id.  NULL returned if none found.
    // Note that the control may be hidden.
    virtual Control* findControlById(const std::string& id);

    // The background color of the window.
    virtual void setColor(const GFXColor& c) { m_color = c; };
    virtual GFXColor color(void) { return m_color; };

    // Set up a controller object.
    virtual WindowController* controller(void) { return m_controller; };
    virtual void setController(WindowController* controller) { m_controller = controller; } ;

    // Draw the window.  Return true if *anything* gets drawn.
    virtual bool draw(void);

    // Read window properties and controls from an XML file.
    virtual void readFromXml(const std::string& fileName);

    // Set whether to delete this object when it closes.
    virtual bool deleteOnClose(void) { return m_deleteOnClose; };
    virtual void setDeleteOnClose(bool del) { m_deleteOnClose = del; };

    // OVERRIDES
    virtual bool processMouseDown(const InputEvent& event);
    virtual bool processMouseUp(const InputEvent& event);
    virtual bool processMouseMove(const InputEvent& event);
    virtual bool processMouseDrag(const InputEvent& event);
    virtual bool processCommand(const EventCommandId& command, Control* control); 

    // CONSTRUCTION
public:
    Window(void);
    virtual ~Window(void);


    // INTERNAL IMPLEMENTATION
protected:
    // Draw window background.
    bool drawBackground(void);

    // VARIABLES
protected:
    Rect m_rect;                    // Rectangle representing window.
    GFXColor m_color;               // Background color of window.
    GroupControl* m_controls;       // List of controls that are in this window.
    bool m_deleteOnClose;           // True = delete window object when closed.
    WindowController* m_controller; // Object that controls this window.
};


// This class keeps track of windows, maintaining z-order and rendering them.
class WindowManager
{
public:
    friend Window;                  // Most operations here are done only by windows.

    // Draw all visible windows.  Return true if anything is drawn.
    bool draw();

    // Shut down all windows.
    void shutDown(void);

protected:
    // A new window has been created and is ready to be drawn.
    void openWindow(Window* w);
    // A window has been closed.
    void closeWindow(Window* w, bool deleteWindow = true);

    // VARIABLES
    std::vector<Window*> m_windows;   // Array of windows.  Last entry is top window.
};

// Get the one window manager.
WindowManager& globalWindowManager(void);

#endif   // __WINDOW_H__
