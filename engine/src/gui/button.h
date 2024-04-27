/*
 * button.h
 *
 * Copyright (C) 2001-2024 Daniel Horn, David Ranger, pyramid3d,
 * Stephen G. Tuggy, Benjamen R. Meyer, and other Vega Strike contributors.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_ENGINE_GUI_BUTTON_H
#define VEGA_STRIKE_ENGINE_GUI_BUTTON_H

//Version: 1.0 - Initial Release (Feb 18, 2002)

/* This class is designed to be self sufficient.
 * The only external functions it requires that aren't provided by system libs are in glut_support.h
 */

#if !defined(_WIN32)
#define GL_GLEXT_PROTOTYPES 1
#endif

#if defined(__APPLE__) && defined(__MACH__)
    #include <OpenGL/gl.h>
    #include <GL/glext.h>
    #include <GLUT/glut.h>
#else
#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif //tells VCC not to generate min/max macros
    #include <windows.h>
#else
    #include <GL/gl.h>
#endif
    #include <GL/glut.h>
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
