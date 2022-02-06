/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
#include <queue>
#include <list>
using std::queue;
using std::list;

#include "in_kb.h"
#include "in_mouse.h"
#include "in_joystick.h"
#include "in_handler.h"

extern KBSTATE keyState[LAST_MODIFIER][KEYMAP_SIZE];

queue<InputListener *> activationreqqueue;
list<InputListener *> listeners;
InputListener *activelistener;

void AddListener(InputListener *il)
{
    il->keystate = keyState;
    il->mousex = &mousex;
    il->mousey = &mousey;
    listeners.push_back(il);
}

void ActivateListener(InputListener *il)
{
    activationreqqueue.push(il);
}

void RemoveListener(InputListener *il)
{
    listeners.remove(il);
}

void ProcessInput(size_t whichplayer)
{
    ProcessKB(whichplayer);
    ProcessMouse();
    for (int i = 0; i < MAX_JOYSTICKS; i++) {
        if (joystick[i]->player == whichplayer) {
            ProcessJoystick(i);
        }
    }
}

void InitInput()
{
    InitKB();
    InitMouse();
    InitJoystick();
}

void DeInitInput()
{
    DeInitJoystick();
}

