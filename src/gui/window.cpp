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

#include "window.h"

#include "eventmanager.h"
#include "groupcontrol.h"
#include "windowcontroller.h"

#include "eventmanager.h"

// For drawing the cursor.
#include "gfx/aux_texture.h"
#include "gfx/sprite.h"

using namespace std;

// The outside boundaries of the window.
void Window::setRect(const Rect& r) {
    m_rect = r;
}
void Window::setFullScreen(void) {
    setRect(FULL_SCREEN_RECT);
}
void Window::setSizeAndCenter(const Size& size) {
    setRect(Rect(-size.width/2.0, -size.height/2.0, size.width, size.height));
}

// Initially display the window.
// Call this when all the properties and controls of the window are set.
void Window::open(void) {
    globalWindowManager().openWindow(this);
}

// Done with the window.
void Window::close(void) {
    processCommand("Window::Close", NULL);
}


// Manage controls.
void Window::addControl(Control* c) {
    m_controls->addChild(c);
}
void Window::deleteControl(Control* c) {
    m_controls->deleteControl(c);
}

// Take a control away from this window and save it elsewhere.
Control* Window::removeControlFromWindow(Control* c) {
    return m_controls->removeControlFromGroup(c);
}

// Find a control using its id.  NULL returned if none found.
// Note that the control may be hidden.
Control* Window::findControlById(const std::string& id) {
    return m_controls->findControlById(id);
}

// Draw/redraw the whole window.
void Window::draw(void) {
    drawBackground();
    m_controls->draw();
}

// Draw window background.
void Window::drawBackground(void) {
	m_texture.draw(m_rect);

    if(!isClear(m_color)) {
        drawRect(m_rect, m_color);
    }

	if(!isClear(m_outlineColor)) {
		drawRectOutline(m_rect, m_outlineColor, m_outlineWidth);
	}
}

// Read window properties and controls from an XML file.
void Window::readFromXml(const std::string& fileName) {
}

// OVERRIDES
bool Window::processMouseDown(const InputEvent& event) {
    if(m_controls->processMouseDown(event)) {
        return true;
    }

    return EventResponder::processMouseDown(event);
}

bool Window::processMouseUp(const InputEvent& event) {
    if(m_controls->processMouseUp(event)) {
        return true;
    }

    return EventResponder::processMouseUp(event);
}

bool Window::processMouseMove(const InputEvent& event) {
    if(m_controls->processMouseMove(event)) {
        return true;
    }

    return EventResponder::processMouseMove(event);
}

bool Window::processMouseDrag(const InputEvent& event) {
    if(m_controls->processMouseDrag(event)) {
        return true;
    }

    return EventResponder::processMouseDrag(event);
}

bool Window::processCommand(const EventCommandId& command, Control* control) {
    // The controller should see the command first.
    if(m_controller != NULL) {
        if(m_controller->processWindowCommand(command, control)) {
            return true;
        }
    }

    // Default command responders.
    if(command == "Window::Close") {
        // Close the window!
        // CAREFUL!  This may delete this object!  Should be the last line in the function.
        globalWindowManager().closeWindow(this, m_deleteOnClose);
        return true;
    }

    return EventResponder::processCommand(command, control);
}

// CONSTRUCTION
Window::Window(void)
:
m_rect(0.0,0.0,0.0,0.0),
m_color(GUI_OPAQUE_BLACK),
m_outlineColor(GUI_CLEAR),
m_outlineWidth(1.0),
m_deleteOnClose(true),
m_controls(NULL),
m_controller(NULL)
{
    m_controls = new GroupControl();
}

Window::~Window(void) {
	EventManager::addToDeleteQueue(m_controls);
}




///////////////////  WINDOW MANAGER  /////////////////////////

extern void	ConditionalCursorDraw(bool);
// Draw all visible windows.
void WindowManager::draw() {
    vector <Window*>::iterator iter;

    GFXHudMode(true);              // Load identity matrices.
    GFXColorf(GUI_OPAQUE_WHITE);

    GFXDisable(DEPTHTEST);
    GFXEnable(DEPTHWRITE);
    GFXDisable(LIGHTING);
    GFXDisable(CULLFACE);
    GFXClear(GFXTRUE);
    GFXDisable(DEPTHWRITE);
    GFXBlendMode(SRCALPHA,INVSRCALPHA);
    GFXDisable(TEXTURE1);
    GFXEnable(TEXTURE0);

    // Just loop through all the windows, and remember if anything gets drawn.
    // Since the first window in the list is drawn first, z-order is
    //  maintained.  First entry is the bottom window, last is the top window.
    // FIXME mbyron -- I think the event manager needs to get involved with window z-order.
    //   (Mouse events should go to windows in zorder, shouldn't they?)
    for(iter = m_windows.begin() ; iter != m_windows.end() ; iter++ ) {
        (*iter)->draw();
    }

    // Emulate EndGUIFrame.
    static Sprite MouseSprite("mouse.spr",BILINEAR,GFXTRUE);
    static Texture dummy("white.bmp",0,NEAREST,TEXTURE2D,TEXTURE_2D,GFXTRUE);
	GFXDisable(CULLFACE);
	ConditionalCursorDraw(true);
   // Figure position of cursor sprite.
    float sizex=0.0, sizey=0.0;
    const Point loc = globalEventManager().mouseLoc();
    MouseSprite.GetSize(sizex,sizey);
    MouseSprite.SetPosition(loc.x+sizex/2, loc.y+sizey/2);

    dummy.MakeActive();
    GFXBlendMode(SRCALPHA,INVSRCALPHA);
    GFXColorf(GUI_OPAQUE_WHITE);

    // Draw the cursor sprite.
    GFXEnable(TEXTURE0);
    GFXDisable(DEPTHTEST);
    GFXDisable(TEXTURE1);
    MouseSprite.Draw();

    GFXHudMode(false);
    GFXEnable(CULLFACE);
}

// A new window has been created and is ready to be drawn.
void WindowManager::openWindow(Window* w) {
    m_windows.push_back(w);
    globalEventManager().pushResponder(w);
}

// A window has been closed.
void WindowManager::closeWindow(
    Window* w,                      // Old window.
    bool deleteWindow               // True = delete window.
) {
    vector <Window*>::iterator iter;

    for(iter = m_windows.begin() ; iter != m_windows.end() ; iter++ ) {
        if((*iter) == w) {
            m_windows.erase(iter);
            if(deleteWindow) {
				EventManager::addToDeleteQueue(w);
				globalEventManager().removeResponder(w);		// Have to do this now.
            }
            break;
        }
    }
}

// Close all windows.
void WindowManager::shutDown(void) {
    while(m_windows.size() > 0) {
        m_windows.back()->close();
		//m_windows.erase(m_windows.begin());
    }
}

// Pointer to the one, global window manager.
WindowManager* globalWindowManagerPtr = NULL;

// Get the one window manager.
WindowManager& globalWindowManager(void) {
    if( globalWindowManagerPtr == NULL ) {
        globalWindowManagerPtr = new WindowManager;
    }

    return *globalWindowManagerPtr;
}

