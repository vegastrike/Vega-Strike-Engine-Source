/***************************************************************************
*                           button.h  -  description
*                           --------------------------
*                           begin                : January 10, 2002
*                           copyright            : (C) 2002 by David Ranger
*                           email                : ussreliant@users.sourceforge.net
*                           copyright            : (C) 2023 Stephen G. Tuggy
*                           email                : sgt@stephengtuggy.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   any later version.                                                    *
*                                                                         *
***************************************************************************/

#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

//Version: 1.0 - Initial Release (Feb 18, 2002)

/* This class is designed to be self sufficient.
 * The only external functions it requires that aren't provided by system libs are in glut_support.h
 */

#if defined(__APPLE__) && defined(__MACH__)
    #include <OpenGL/gl.h>
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

class Button
{
public: Button( float x, float y, float wid, float hei, const char *name );
    ~Button( void );

    void Refresh( void );

    int MouseClick( int button, int state, float x, float );
    int MouseMove( float x, float y );
    int MouseMoveClick( float x, float y );
    int DoMouse( int type, float x, float y, int button, int state );
    void ModifyName( const char *newname );
private:
//Stores the location and label of the button
    float xcoord;
    float ycoord;
    float width;
    float height;
    char *label;

//Flag that says wether or not to highlight the button
    int   highlight;

    int Inside( float x, float y );
};

#endif    //GUI_BUTTON_H

