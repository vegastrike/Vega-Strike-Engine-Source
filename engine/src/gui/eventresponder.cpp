/*
 * eventresponder.cpp
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

#include "vegastrike.h"

#include "eventresponder.h"

#include "eventmanager.h"

//The EventResponder class is a virtual base class that allows objects
//to intercept and respond to user events.  There are two kinds of
//events:
//1. Input events.  Raw events from input devices that are specific
//to the kind of device.  Examples:  key events, mouse down, etc.
//there are specific functions to support each kind of input device.
//2. Command events.  These are often caused by input events, but are
//higher-level.  They are identified by a string ID.  Examples:
//"Cut", "Cockpit::ECM", "Buy Cargo".  Input events are usually
//translated into command events.  Command events usually execute the
//operation requested by the user.
//This is a virtual base class.  The responder functions should be overridden
//to handle particular events.  All responder functions return true if the
//event was handled, false if not.  Default implementations generally
//return false.
//
//This class is used in conjunction with the EventManager, which maintains
//a chain of EventResponders.  Events are passed down the chain to find
//something that can execute them.

//Process a command event.
bool EventResponder::processCommand(const EventCommandId &command, Control *control) {
    return m_modal;
}

//Process a key pressed down.
bool EventResponder::processKeyDown(const InputEvent &event) {
    return m_modal;
}

//Process a key released.
bool EventResponder::processKeyUp(const InputEvent &event) {
    return m_modal;
}

//Process a mouse button pressed down.
bool EventResponder::processMouseDown(const InputEvent &event) {
    return m_modal;
}

//Process a mouse button pressed elsewhere, unfocusing this control.
void EventResponder::processUnfocus(const InputEvent &event) {
}

//Process a mouse button released.
bool EventResponder::processMouseUp(const InputEvent &event) {
    return m_modal;
}

//Process a mouse location change.
bool EventResponder::processMouseMove(const InputEvent &event) {
    return m_modal;
}

//Process a mouse location change when at least one mouse button is down.
bool EventResponder::processMouseDrag(const InputEvent &event) {
    return m_modal;
}

//Send a command event into the event chain.
void EventResponder::sendCommand(const EventCommandId &command, Control *control) {
    if (m_commandTarget != NULL) {
        if (m_commandTarget->processCommand(command, control)) {
            return;
        }
    }
    globalEventManager().sendCommand(command, control);
}

//Set a specified target for commands.  Commands aren't forwarded into the
//event chain, they are sent to this specific target.  This can be used, for
//instance, to tie two controls tightly together.
//Use NULL to clear the target and forward commands into the event chain.
void EventResponder::setCommandTarget(EventResponder *responder) {
    m_commandTarget = responder;
}

//Handle all input events.  Don't forward anything down the event chain.
void EventResponder::setModal(bool flag) {
    m_modal = flag;
}

//CONSTRUCTION
EventResponder::EventResponder(void) :
        m_modal(false), m_commandTarget(NULL) {
}

EventResponder::~EventResponder(void) {
    //Make sure this responder is not in the event chain.
    if (hasGlobalEventManager()) {
        globalEventManager().removeResponder(this);
    }
}

