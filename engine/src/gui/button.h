/*
 * button.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file; David Ranger specifically
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
#ifndef VEGA_STRIKE_ENGINE_GUI_BUTTON_H
#define VEGA_STRIKE_ENGINE_GUI_BUTTON_H

//Version: 1.0 - Initial Release (Feb 18, 2002)

/* This class is designed to be self sufficient.
 * The only external functions it requires that aren't provided by system libs are in glut_support.h
 */

// See https://github.com/vegastrike/Vega-Strike-Engine-Source/pull/851#discussion_r1589254766
#if defined(__APPLE__) && defined(__MACH__)
    #include <gl.h>
    #include <glut.h>
#else
#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif //tells VCC not to generate min/max macros
    #include <windows.h>
#else
    #include <gl.h>
#endif
    #include <glut.h>
#endif

#include "glut_support.h"

class Button {
public:
    Button(float x, float y, float wid, float hei, const char *name);
    ~Button(void);

    void Refresh(void);

    int MouseClick(int button, int state, float x, float);
    int MouseMove(float x, float y);
    int MouseMoveClick(float x, float y);
    int DoMouse(int type, float x, float y, int button, int state);
    void ModifyName(const char *newname);
private:
//Stores the location and label of the button
    float xcoord;
    float ycoord;
    float width;
    float height;
    char *label;

//Flag that says wether or not to highlight the button
    int highlight;

    int Inside(float x, float y);
};

#endif    //VEGA_STRIKE_ENGINE_GUI_BUTTON_H
