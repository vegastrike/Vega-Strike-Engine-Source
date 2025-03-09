/*
 * windowcontroller.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file; Mike Byron specifically
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

#include "vegastrike.h"

#include "eventmanager.h"

#include "windowcontroller.h"

//Make everything happen.
void WindowController::run(void) {
    if (m_window) {
        m_window->setDeleteOnClose(true);         //We want the window to delete itself.
        m_window->setController(this);
        m_window->open();
    } else {
        //We have no window.  Close down.
        delete this;
    }
}

void WindowController::draw(void) {
    //Do nothing.
}

//Process a command from the window.
bool WindowController::processWindowCommand(const EventCommandId &command, Control *control) {
    if (command == "Window::Close") {
        //Our window is closing.
        if (m_deleteOnWindowClose) {
            delete this;
            //We return false so that the window itself can see this command.
            return false;
        }
    }
    //Didn't find a handler.
    return false;
}

//CONSTRUCTOR
WindowController::WindowController() : m_window(NULL), m_deleteOnWindowClose(true) {
}

//DESTRUCTOR
WindowController::~WindowController(void) {
}
