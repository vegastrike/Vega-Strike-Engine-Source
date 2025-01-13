/*
 * eventresponder.h
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
#ifndef VEGA_STRIKE_ENGINE_GUI_EVENTRESPONDER_H
#define VEGA_STRIKE_ENGINE_GUI_EVENTRESPONDER_H

//See cpp file for detailed descriptions of classes, functions, etc.

#include "guidefs.h"
#include <string>

//The type of a command
typedef std::string EventCommandId;

//Forward reference
class Control;

//The EventResponder class is a virtual base class that allows objects
//to intercept and respond to input and command events. This class is
//used in conjunction with the EventManager.

class EventResponder {
public:
//PROCESS COMMAND
//Process a command event.
    virtual bool processCommand(const EventCommandId &command, Control *control);

//PROCESS KEYBOARD
//Process a key pressed down.
    virtual bool processKeyDown(const InputEvent &event);

//Process a key released.
    virtual bool processKeyUp(const InputEvent &event);

//PROCESS MOUSE
//Process a mouse button pressed down.
    virtual bool processMouseDown(const InputEvent &event);

//Called for every click except those landing on the control.
    virtual void processUnfocus(const InputEvent &event);

//Process a mouse button released.
    virtual bool processMouseUp(const InputEvent &event);

//Process a mouse location change.
    virtual bool processMouseMove(const InputEvent &event);

//Process a mouse location change when at least one mouse button is down.
    virtual bool processMouseDrag(const InputEvent &event);

//CODE-GENERATED EVENTS.
//Send a command event into the event chain.
    virtual void sendCommand(const EventCommandId &command, Control *control);

//Set a specified target for commands.  Commands aren't forwarded into the
//event chain, they are sent to this specific target.  This can be used, for
//instance, to tie two controls tightly together.
//Use NULL to clear the target and forward commands into the event chain.
    virtual void setCommandTarget(EventResponder *responder);

//BEHAVIOR
//Handle all input events.  Don't forward anything down the event chain.
    virtual void setModal(bool modal);

//CONSTRUCTION
public:
    EventResponder(void);
    virtual ~EventResponder(void);

//VARIABLES
protected:
    bool m_modal;                   //true = This window "traps" all events -- events don't go to other windows.
    EventResponder *m_commandTarget; //Forward events to this particular responder, not the event chain.
};

#endif   //VEGA_STRIKE_ENGINE_GUI_EVENTRESPONDER_H
