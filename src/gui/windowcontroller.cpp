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

#include "windowController.h"

// Make everything happen.
void WindowController::run(void) {
    if(m_window) {
        m_window->setDeleteOnClose(true);      // We want the window to delete itself.
        m_window->setController(this);
        m_window->open();
    } else {
        // We have no window.  Close down.
        delete this;
    }
}

// Process a command from the window.
bool WindowController::processWindowCommand(const EventCommandId& command, Control* control) {
    if(command == "Window::Close") {
        // Our window is closing.
        if(m_deleteOnWindowClose) {
            delete this;
            // We return false so that the window itself can see this command.
            return false;
        }
    }

    // Didn't find a handler.
    return false;
};

// CONSTRUCTOR
WindowController::WindowController() : m_window(NULL), m_deleteOnWindowClose(true)
{
}

// DESTRUCTOR
WindowController::~WindowController(void) {
}

