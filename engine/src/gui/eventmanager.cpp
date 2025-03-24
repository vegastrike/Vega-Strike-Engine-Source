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


#include "src/vegastrike.h"

#include "eventmanager.h"
#include "src/vs_logging.h"

#include <algorithm>

/* The EventManager class contains the basic event loop and the code
 * to support the EventResponder chain.  There should be only one
 * instance of this class running in an application.
 * You can get a pointer to it by using the static globalEventManager()
 * function.
 */
extern void ModifyMouseSensitivity(int &x, int &y);
//This is the one, unique event manager.
static EventManager *globalEventManagerPtr = NULL;

//STATIC: Get the global instance of the event manager
EventManager &globalEventManager(void) {
    if (globalEventManagerPtr == NULL) {
        EventManager::initializeEventManager();
        assert(globalEventManagerPtr != NULL);         //Make sure we got a manager!
    }
    return *globalEventManagerPtr;
}

bool hasGlobalEventManager(void) {
    return globalEventManagerPtr != NULL;
}

//STATIC: Initialize the event manager.  This starts the event loop, etc.
//This may be called more than once -- it does nothing after the
//first call.
void EventManager::initializeEventManager(void) {
    globalEventManagerPtr = new EventManager();
    globalEventManagerPtr->takeOverEventManagement();              //FIXME -- EVENT HACK
}

static std::vector<EventResponder *> deleteQueue;

void EventManager::addToDeleteQueue(EventResponder *controlToDelete) {
    if (controlToDelete == NULL || find(deleteQueue.begin(), deleteQueue.end(), controlToDelete) != deleteQueue.end()) {
        bool DUPLICATE_DELETE_OF_OBJECT = true;
        VS_LOG_AND_FLUSH(fatal, (boost::format("\nERROR: duplicate delete of object %1$x.\n\n") % controlToDelete));
        printf("Attach a debugger now!");
        while (DUPLICATE_DELETE_OF_OBJECT) {
        }
    } else {
        deleteQueue.push_back(controlToDelete);
    }
}

static void clearDeleteQueue() {
    while (deleteQueue.size()) {
        std::vector<EventResponder *> queue(deleteQueue);
        deleteQueue.clear();
        for (size_t i = 0; i < queue.size(); i++) {
            delete queue[i];
        }
    }
}

//Add a new event responder to the top of the chain.
//This responder will get events *first*.
void EventManager::pushResponder(EventResponder *responder) {
    m_responders.push_back(responder);
}

//Remove an event responder from the chain.
void EventManager::removeResponder(EventResponder *responder, //The responder to get rid of.
        bool top //True = only topmost, False = all.
) {
    bool found;                 //Whether we found one
    do {
        found = false;
        //Start at the top, so that we only get the top if we only want the top.
        //Can't use reverse_iterator easily -- erase() needs iterator.
        for (int i = m_responders.size() - 1; i >= 0; i--) {
            if (m_responders[i] == responder) {
                m_responders.erase(m_responders.begin() + i);
                found = true;
                break;
            }
        }
        //Do the loop again if we found one and we want all of them.
    } while (found && !top);
//FIXME -- Calling this here causes the loop in processEvent to be b0rked by a deletion. We now must checkForShutdown at a later time...which is below    checkForShutDownEventManager();     // FIXME mbyron -- EVENT HACK.
}

//Send a command through the responder chain.
void EventManager::sendCommand(const EventCommandId &id, Control *control) {
    vector<EventResponder *>::reverse_iterator iter;
    //Loop through the event chain, starting at the end.
    for (iter = m_responders.rbegin(); iter != m_responders.rend(); iter++) {
        if ((*iter)->processCommand(id, control)) {
            //Somebody handled it!
            break;
        }
    }
}

//Send an input event through the responder chain.
void EventManager::sendInputEvent(const InputEvent &event) {
    //Record the mouse position.
    //This is used (at least) to render the cursor.
    switch (event.type) {
        case MOUSE_DOWN_EVENT:
        case MOUSE_UP_EVENT:
        case MOUSE_MOVE_EVENT:
        case MOUSE_DRAG_EVENT:
            m_mouseLoc = event.loc;
            break;
        default:
            m_mouseLoc = event.loc;
            break;
    }
    //Loop through the event chain, starting at the end.
    //WARNING:  The functions in this loop can change the responders list.
    //Iterate through the list carefully!
    for (size_t i = m_responders.size(); i > 0; i--) {
        bool result = false;
        if (i < m_responders.size() + 1) {
            //Check this in case responders get deleted.
            switch (event.type) {
                case KEY_DOWN_EVENT:
                    result = m_responders[i - 1]->processKeyDown(event);
                    break;
                case KEY_UP_EVENT:
                    result = m_responders[i - 1]->processKeyUp(event);
                    break;
                case MOUSE_DOWN_EVENT:
                    result = m_responders[i - 1]->processMouseDown(event);
                    break;
                case MOUSE_UP_EVENT:
                    result = m_responders[i - 1]->processMouseUp(event);
                    break;
                case MOUSE_MOVE_EVENT:
                    result = m_responders[i - 1]->processMouseMove(event);
                    break;
                case MOUSE_DRAG_EVENT:
                    result = m_responders[i - 1]->processMouseDrag(event);
                    break;
                default:
                    //Event responder dispatch doesn't handle this type of input event!
                    assert(false);
                    break;
            }
        }
        if (result) {
            //Somebody handled it!
            break;
        }
    }
    checkForShutDownEventManager();
}

//Constructor isn't public.  Use initializeEventManager.
EventManager::EventManager(void) :
        m_mouseLoc(0.0, 0.0) {
}

//Destructor.
EventManager::~EventManager(void) {
}

///////////// HACKS FOR WORKING WITH CURRENT EVENT SYSTEM  //////////////////
#include "cmd/base.h"
#include "gldrv/winsys.h"

extern void InitCallbacks(void);

//Called to revert to old event management.
void EventManager::checkForShutDownEventManager(void) {
    if (m_responders.empty() && globalEventManagerPtr != NULL) {
        //There are no more responders.  We assume no more of our windows, and reset mouse callbacks.
        //If we don't have a global event manager, we already did this.
        InitCallbacks();

        //Get rid of global event manager object until we need it again.
        delete globalEventManagerPtr;
        globalEventManagerPtr = NULL;
    }
}

//Map mouse coord to Vegastrike 2d coord.
static float MouseXTo2dX(int x) {
    //2*(coord+.5)/res + 1.
    //Puts origin in the middle of the screen, going -1 -> 1 left to right.
    //Add .5 to put mouse in middle of pixel, rather than left side.
    //Multiply by 2 first to reduce division error in the multiply.
    //Do everything in double to minimize calc error and because it's faster.
    //Result in float to round-off at the end.  Gets prettier numbers. :-)
    return (2.0 * ((double) x + 0.5)) / g_game.x_resolution - 1.0;
}

static float MouseYTo2dY(int y) {
    //See explanation of x.
    //This is a bit different from x because the mouse coords increase top-
    //to-bottom, and the drawing surface y increases bottom-to-top.
    //So we need to reflect the mouse coords around the y origin.
    return 1.0 - (2.0 * ((double) y + 0.5)) / g_game.y_resolution;
}

void EventManager::ProcessMouseClick(int button, int state, int x, int y) {
    ModifyMouseSensitivity(x, y);
    //Make sure we are working with the same "button" constants.
    assert(LEFT_MOUSE_BUTTON == WS_LEFT_BUTTON);
    assert(RIGHT_MOUSE_BUTTON == WS_RIGHT_BUTTON);
    assert(MIDDLE_MOUSE_BUTTON == WS_MIDDLE_BUTTON);
    assert(WHEELUP_MOUSE_BUTTON == WS_WHEEL_UP);
    assert(WHEELDOWN_MOUSE_BUTTON == WS_WHEEL_DOWN);
    if (state == WS_MOUSE_DOWN) {
        InputEvent event(MOUSE_DOWN_EVENT, button, 0, Point(MouseXTo2dX(x), MouseYTo2dY(y)));
        globalEventManager().sendInputEvent((event));
    } else {
        InputEvent event(MOUSE_UP_EVENT, button, 0, Point(MouseXTo2dX(x), MouseYTo2dY(y)));
        globalEventManager().sendInputEvent((event));
    }
    clearDeleteQueue();
}

void EventManager::ProcessMouseActive(int x, int y) {
    ModifyMouseSensitivity(x, y);
    //FIXME mbyron -- Should provide info about which buttons are down.
    InputEvent event(MOUSE_DRAG_EVENT, 0, 0, Point(MouseXTo2dX(x), MouseYTo2dY(y)));
    globalEventManager().sendInputEvent((event));
    clearDeleteQueue();
}

void EventManager::ProcessMousePassive(int x, int y) {
    ModifyMouseSensitivity(x, y);
    InputEvent event(MOUSE_MOVE_EVENT, 0, 0, Point(MouseXTo2dX(x), MouseYTo2dY(y)));
    globalEventManager().sendInputEvent((event));
    clearDeleteQueue();
}

//Called to grab event management from old system.
void EventManager::takeOverEventManagement(void) {
    winsys_set_mouse_func(EventManager::ProcessMouseClick);
    winsys_set_motion_func(EventManager::ProcessMouseActive);
    winsys_set_passive_motion_func(EventManager::ProcessMousePassive);
}

