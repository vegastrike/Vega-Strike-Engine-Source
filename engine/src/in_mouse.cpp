/*
 * in_mouse.cpp
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
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

#include "src/vegastrike.h"
#include "src/in_handler.h"
#include "src/in_mouse.h"
#include <deque>
#include "root_generic/vs_globals.h"
#include "src/config_xml.h"
#include "src/in_joystick.h"
#include "gldrv/winsys.h"
#include "root_generic/options.h"
#include "src/universe.h"

using std::deque;
#define NUM_BUTTONS 15

/** Gets the button number of the function used to draw the mouse*/
int getMouseDrawFunc() {
    return NUM_BUTTONS;
}

KBSTATE MouseState[NUM_BUTTONS + 1] = {RELEASE};
static MouseHandler mouseBindings[NUM_BUTTONS + 1];

int mousex = 0;
int mousey = 0;

void GetMouseXY(int &mx, int &my) {
    mx = mousex;
    my = mousey;
}

int getMouseButtonStatus() {
    int ret = 0;
    for (int i = 0; i < NUM_BUTTONS; i++) {
        ret |= (MouseState[i] == PRESS || MouseState[i] == DOWN) ? (1 << i) : 0;
    }
    return ret;
}

struct MouseEvent {
    enum EventType { CLICK, DRAG, MOTION } type;
    int button;
    int state;
    int mod;
    int x;
    int y;

    MouseEvent(EventType type, int button, int state, int mod, int x, int y)
            : type(type), button(button), state(state), mod(mod), x(x), y(y) {
    }
};

static deque<MouseEvent> eventQueue;

void mouseClickQueue(int button, int state, int x, int y) {
    int mod = 0;
    eventQueue.push_back(MouseEvent(MouseEvent::CLICK, button, state, mod, x, y));
}

int delx = 0;
int dely = 0;

void AddDelta(int dx, int dy) {
    delx += dx;
    dely += dy;
}

int warpallowage = 2;

void DealWithWarp(int x, int y) {
    if (configuration()->joystick.warp_mouse) {
        if (joystick[MOUSE_JOYSTICK]->player < _Universe->numPlayers()) {
            if (x < configuration()->joystick.warp_mouse_zone || y < configuration()->joystick.warp_mouse_zone
                    || x > configuration()->graphics.resolution_x - configuration()->joystick.warp_mouse_zone || y
                    > configuration()->graphics.resolution_y - configuration()->joystick.warp_mouse_zone) {

                int delx = -x + configuration()->graphics.resolution_x / 2;
                int dely = -y + configuration()->graphics.resolution_y / 2;
                mousex += delx;
                mousey += dely;
                deque<MouseEvent>::iterator i;
                for (i = eventQueue.begin(); i != eventQueue.end(); i++) {
                    i->x += delx;
                    i->y += dely;
                }
                if (warpallowage-- >= 0) {
                    winsys_warp_pointer(configuration()->graphics.resolution_x / 2, configuration()->graphics.resolution_y / 2);
                }
            }
        }
    }
}

void mouseDragQueue(int x, int y) {
    eventQueue.push_back(MouseEvent(MouseEvent::DRAG, -1, -1, -1, x, y));
    DealWithWarp(x, y);
}

void mouseMotionQueue(int x, int y) {
    eventQueue.push_back(MouseEvent(MouseEvent::MOTION, -1, -1, -1, x, y));
    DealWithWarp(x, y);
}

int lookupMouseButton(int b) {
    static int adj = 0;
    if (b + adj < WS_LEFT_BUTTON) {
        adj = WS_LEFT_BUTTON - b;
    }
    b += adj;
    switch (b) {
        case WS_LEFT_BUTTON:
            return 0;

        case WS_RIGHT_BUTTON:
            return 2;

        case WS_MIDDLE_BUTTON:
            return 1;

        case WS_WHEEL_UP:
            return 3;

        case WS_WHEEL_DOWN:
            return 4;

        default:
            return ((b - WS_LEFT_BUTTON) >= NUM_BUTTONS) ? NUM_BUTTONS - 1 : b - WS_LEFT_BUTTON;
    }
    return 0;
}

void mouseClick0(int button, int state, int mod, int x, int y) {
    button = lookupMouseButton(button);
    if (button >= NUM_BUTTONS) {
        return;
    }
    AddDelta(x - mousex, y - mousey);
    mousex = x;
    mousey = y;
    mouseBindings[button](state == WS_MOUSE_DOWN ? PRESS : RELEASE, x, y, 0, 0, mod);
    MouseState[button] = (state == WS_MOUSE_DOWN) ? DOWN : UP;
}

void SetDelta(int dx, int dy) {
    delx = dx;
    dely = dy;
}

void GetMouseDelta(int &dx, int &dy) {
    dx = delx;
    dy = dely;
    delx = dely = 0;
}

void mouseDrag(int x, int y) {
    for (int i = 0; i < NUM_BUTTONS + 1; i++) {
        mouseBindings[i](MouseState[i], x, y, x - mousex, y - mousey, 0);
    }
    AddDelta(x - mousex, y - mousey);
    mousex = x;
    mousey = y;
}

void mouseMotion(int x, int y) {
    for (int i = 0; i < NUM_BUTTONS + 1; i++) {
        mouseBindings[i](MouseState[i], x, y, x - mousex, y - mousey, 0);
    }
    AddDelta(x - mousex, y - mousey);
    mousex = x;
    mousey = y;
}

static void DefaultMouseHandler(KBSTATE, int x, int y, int delx, int dely, int mod) {
}

void UnbindMouse(int key) {
    mouseBindings[key] = DefaultMouseHandler;
}

void BindKey(int key, MouseHandler handler) {
    mouseBindings[key] = handler;
    handler(RESET, mousex, mousey, 0, 0, 0);
}

void RestoreMouse() {
    winsys_set_mouse_func(mouseClickQueue);
    winsys_set_motion_func(mouseDragQueue);
    winsys_set_passive_motion_func(mouseMotionQueue);
}

void InitMouse() {
    for (int a = 0; a < NUM_BUTTONS + 1; a++) {
        UnbindMouse(a);
    }
    RestoreMouse();
}

void ProcessMouse() {
    warpallowage = 2;
    while (eventQueue.size()) {
        MouseEvent e = eventQueue.front();
        switch (e.type) {
            case MouseEvent::CLICK:
                mouseClick0(e.button, e.state, e.mod, e.x, e.y);
                break;
            case MouseEvent::DRAG:
                mouseDrag(e.x, e.y);
                break;
            case MouseEvent::MOTION:
                mouseMotion(e.x, e.y);
                break;
        }
        eventQueue.pop_front();
    }
}

