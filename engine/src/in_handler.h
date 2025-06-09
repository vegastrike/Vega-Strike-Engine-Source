/*
 * in_handler.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: Alan Shieh
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
#ifndef VEGA_STRIKE_ENGINE_INHANDLER_H
#define VEGA_STRIKE_ENGINE_INHANDLER_H

#include "src/in_kb.h"
//#include "cmd_unit.h"
#include <map>
#include <string>

class Unit;

typedef void (*INDISPATCH)(int x, int y);

class InputListener {
public:
    int *mousex, *mousey;

    Unit *parent;

    InputListener(Unit *parent) {
        this->parent = parent;
        //int a;
        //mousex = mousey = 0;
        //ZeroMemory(keystate, sizeof(keystate));
    }

    virtual void MoveMouse(int x, int y) {
        //mousex = x;
        //mousey = y;
    }

    virtual void MoveMouse(int *x, int *y, int num) {
    }

    virtual void KeyDown(int key) {
        //keystate[key] = 1;
    }

    virtual void KeyDown(int *key, int num) {
    }

    virtual void KeyUp(int key) {
        //keystate[key] = 0;
    }

    virtual void KeyUp(int *key, int num) {
    }

    virtual void Activate() {
    }

    virtual void Deactivate() {
    }

//InHandler &Dispatch(); // Drives the input dispatcher, for switching input states
//InHandler &Revert(); // return to the previous state
};

//const InputListener End;
#endif //VEGA_STRIKE_ENGINE_INHANDLER_H

