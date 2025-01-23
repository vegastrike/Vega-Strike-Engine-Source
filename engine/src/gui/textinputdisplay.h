/*
 * textinputdisplay.h
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
#ifndef VEGA_STRIKE_ENGINE_GUI_TEXT_INPUT_DISPLAY_H
#define VEGA_STRIKE_ENGINE_GUI_TEXT_INPUT_DISPLAY_H

#include "staticdisplay.h"
#include <vector>

//See cpp file for detailed descriptions of classes, functions, etc.

//The TextInputDisplay class is used to get text input from the user.

class TextInputDisplay : public StaticDisplay {
    std::vector<unsigned int> local_keyboard_queue;
    std::vector<unsigned int> *keyboard_queue;
    char *disallowed;
    bool isFocused;
    char passwordChar;
public:
    TextInputDisplay(std::vector<unsigned int> *keyboard_input_queue, const char *disallowed);
//OVERRIDES - used for focusing input.
    virtual void processUnfocus(const InputEvent &event);
    virtual bool processMouseDown(const InputEvent &event);
    virtual bool processKeypress(unsigned int pressedKey);

    void setPassword(char passchar) {
        passwordChar = passchar;
    }

    virtual ~TextInputDisplay();
    virtual void draw();
};

#endif   //VEGA_STRIKE_ENGINE_GUI_TEXT_INPUT_DISPLAY_H
