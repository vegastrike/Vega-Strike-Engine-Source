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

#ifndef __WINDOWCONTROLLER_H__
#define __WINDOWCONTROLLER_H__

#include "window.h"
#include "control.h"

// This class is meant to run, or control, a Window.  It probably creates
//  the Window and its Controls, loads the controls with lists if necessary, etc.
// By default, this object deletes itself when the window closes.
// The Window passes command events to the controller, which manages any
//  interdependencies between the controls.
// The controller basically manages the state changes in the application that
//  the Window (the UI) requests.
class WindowController
{
public:
    // Set up the window and get everything ready.
    // This is separate from run() so the creator gets a chance to get involved
    //  with the window settings.
    virtual void init(void) = 0;

    // Make everything happen.
    virtual void run(void);

    // The window we are controlling.
    virtual Window* window(void) { return m_window; };
    virtual void setWindow(Window* w) { m_window = w; };

    // Process a command event from the window.
    virtual bool processWindowCommand(const EventCommandId& command, Control* control);

    // CONSTRUCTION
    WindowController();
    virtual ~WindowController(void);

protected:

    // VARIABLES
    Window* m_window;               // The window we control.
    bool m_deleteOnWindowClose;     // True = Delete this object when the window closes.
};

template <class T>
struct WindowControllerTableEntry {
	typedef bool (T::*Handler)(const EventCommandId& command, Control* control);
	
	EventCommandId command;
	std::string controlId;
	Handler function;
	WindowControllerTableEntry(const EventCommandId& cmd, const std::string &cid, const Handler& func)
			:command(cmd), controlId(cid), function(func) {
	}
};


template <class Subclass>
class WctlBase : public WindowController {
public:
	typedef WindowControllerTableEntry<Subclass> WctlTableEntry;
	
	virtual bool processWindowCommand(const EventCommandId& command, Control* control) {
		
		// Iterate through the dispatch table.
		for(const WctlTableEntry *p = &WctlCommandTable[0]; p->function ; p++) {
			if(p->command == command) {
				if(p->controlId.size() == 0 || p->controlId == control->id()) {
					// Found a handler for the command.
					return( ((static_cast<Subclass*>(this))->*(p->function))(command, control) );
				}
			}
		}
		
		// Let the base class have a try at the command first.
		if(WindowController::processWindowCommand(command, control)) {
			return true;
		}
		
		// Didn't find a handler.
		return false;
	}
protected:
    // Dispatch table declarations.
    // This is a member table so the handler functions don't need to be public.
    static const WctlTableEntry WctlCommandTable[];
};

#endif   // __WINDOWCONTROLLER_H__
