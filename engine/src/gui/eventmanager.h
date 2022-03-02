/*
 * Copyright (C) 2001-2022 Daniel Horn, Mike Byron, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __EVENTMANAGER_H__
#define __EVENTMANAGER_H__

#include "eventresponder.h"

#include <vector>

//See cpp file for detailed descriptions of classes, functions, etc.

/* The EventManager class contains the basic event loop and the code
 * to support the EventResponder chain.  There should be only one
 * instance of this class running in an application.
 * You can get a pointer to it by using the static globalEventManager()
 * function.
 */
class EventManager {
public:
//Use this instead of deleting, because other events that happen at the
//same time may delete the same object.
    static void addToDeleteQueue(EventResponder *controlToDelete);
//STATIC: Initialize the event manager.  This starts the event loop, etc.
//This may be called more than once -- it does nothing after the
//first call.
    static void initializeEventManager(void);

//Add a new event responder to the top of the chain.
//This responder will get events *first*.
    void pushResponder(EventResponder *responder);

//Remove an event responder from the chain.
//"all" = True means get rid of all instances of the responder in the chain.
//False means get rid of the topmost one only.
    void removeResponder(EventResponder *responder, bool top = false);

//Send a command through the responder chain.
    void sendCommand(const EventCommandId &id, Control *control);

//Get the current mouse position.
    Point mouseLoc(void) {
        return m_mouseLoc;
    }

protected:
//CONSTRUCTION
//Constructor isn't public.  Use initializeEventManager.
    EventManager(void);
    virtual ~EventManager(void);

//INTERNAL IMPLEMENTATION
//Send an input event through the responder chain.
    void sendInputEvent(const InputEvent &event);

//DATA
    std::vector<EventResponder *> m_responders; //Stack of responders for events.
    Point m_mouseLoc;                           //Current mouse point.

//HACKS FOR WORKING WITH CURRENT EVENT SYSTEM
    void checkForShutDownEventManager(void);   //Called to revert to old event management.
    void takeOverEventManagement(void);        //Called to grab event management from old system.
public:

    static void ProcessMouseClick(int button, int state, int x, int y);
    static void ProcessMouseActive(int x, int y);
    static void ProcessMousePassive(int x, int y);
};

//Get the global instance of the event manager
EventManager &globalEventManager(void);
bool hasGlobalEventManager(void);

#endif   //__EVENTMANAGER_H__

