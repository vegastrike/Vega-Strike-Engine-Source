/*
 * windowcontroller.h
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
#ifndef VEGA_STRIKE_ENGINE_GUI_WINDOW_CONTROLLER_H
#define VEGA_STRIKE_ENGINE_GUI_WINDOW_CONTROLLER_H

#include "window.h"
#include "control.h"

//This class is meant to run, or control, a Window.  It probably creates
//the Window and its Controls, loads the controls with lists if necessary, etc.
//By default, this object deletes itself when the window closes.
//The Window passes command events to the controller, which manages any
//interdependencies between the controls.
//The controller basically manages the state changes in the application that
//the Window (the UI) requests.
class WindowController {
public:
//Set up the window and get everything ready.
//This is separate from run() so the creator gets a chance to get involved
//with the window settings.
    virtual void init(void) = 0;

//Make everything happen.
    virtual void run(void);

    virtual void draw(void);

//The window we are controlling.
    virtual Window *window(void) {
        return m_window;
    }

    virtual void setWindow(Window *w) {
        m_window = w;
    }

//Process a command event from the window.
    virtual bool processWindowCommand(const EventCommandId &command, Control *control);

//CONSTRUCTION
    WindowController();
    virtual ~WindowController(void);

protected:

//VARIABLES
    Window *m_window;               //The window we control.
    bool m_deleteOnWindowClose;  //True = Delete this object when the window closes.
};

template<class T>
struct WindowControllerTableEntry {
    typedef bool (T::*Handler)(const EventCommandId &command, Control *control);

    EventCommandId command;
    std::string controlId;
    Handler function;

    WindowControllerTableEntry(const EventCommandId &cmd, const std::string &cid, const Handler &func) :
            command(cmd), controlId(cid), function(func) {
    }
};

template<class Subclass>
class WctlBase : public WindowController {
public:
    typedef WindowControllerTableEntry<Subclass> WctlTableEntry;

    virtual bool processWindowCommand(const EventCommandId &command, Control *control) {
        //Iterate through the dispatch table.
        for (const WctlTableEntry *p = &WctlCommandTable[0]; p->function; p++) {
            if (p->command == command) {
                if (p->controlId.size() == 0 || p->controlId == control->id()) {
                    //Found a handler for the command.
                    return ((static_cast< Subclass * > (this))->*(p->function))(command, control);
                }
            }
        }
        //Let the base class have a try at the command first.
        if (WindowController::processWindowCommand(command, control)) {
            return true;
        }
        //Didn't find a handler.
        return false;
    }

protected:
//Dispatch table declarations.
//This is a member table so the handler functions don't need to be public.
    static const WctlTableEntry WctlCommandTable[];
};

#endif   //VEGA_STRIKE_ENGINE_GUI_WINDOW_CONTROLLER_H
