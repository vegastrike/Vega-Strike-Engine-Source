/**
 * in_joystick.h
 *
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) Alexander Rawass <alexannika@users.sourceforge.net>
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

/*
 *  Joystick support written by Alexander Rawass <alexannika@users.sourceforge.net>
 */
#include "in_kb_data.h"
#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#if defined (HAVE_SDL)
#include <SDL/SDL.h>
#endif

#include "vegastrike.h"
//#include "glob.h"
//#include "dbg.h"
#include "in_kb.h"
#define NUMJBUTTONS 32

class JoyStick;

//typedef void (*JoyHandler) (int);

extern void ProcessJoystick();
extern void InitJoystick();
extern void DeInitJoystick();

const int MAX_JOYSTICKS      = 16;
const int MOUSE_JOYSTICK     = MAX_JOYSTICKS-1;
const int MAX_BUTTONS        = 48;
const int MAX_DIGITAL_HATSWITCHES = 4;
const int MAX_DIGITAL_VALUES = 9;

enum
{
    VS_HAT_CENTERED=0, VS_HAT_LEFT, VS_HAT_RIGHT, VS_HAT_DOWN, VS_HAT_UP, VS_HAT_RIGHTUP, VS_HAT_RIGHTDOWN, VS_HAT_LEFTUP,
    VS_HAT_LEFTDOWN
};

extern JoyStick *joystick[MAX_JOYSTICKS];

class JoyStick
{
    bool mouse;
    void InitMouse( int i );
    void GetMouse( float &x, float &y, float &z, int &buttons );
public:
//initializes the joystick
    JoyStick( int );
//engine calls GetJoyStick to get coordinates and buttons
    void GetJoyStick( float &x, float &y, float &z, int &buttons );
    bool isAvailable( void );
    bool is_around( float axe, float hswitch );
    int NumButtons();

#if defined (HAVE_SDL)
    SDL_Joystick *joy;
#else
    void   *otherdata; //bad form to have an ifdef in a struct
#endif
    int     nr_of_axes, nr_of_buttons, nr_of_hats;
    int     hat_margin;
    size_t  player;
#define MAX_AXES 32
    bool    axis_inverse[MAX_AXES];
    int     axis_axis[MAX_AXES];
    float   joy_axis[MAX_AXES];
    JoyStick();
#if defined (IRIX)        //could be POSIX type uchar_t?
    uchar_t digital_hat[MAX_DIGITAL_HATSWITCHES];
#else
    unsigned char digital_hat[MAX_DIGITAL_HATSWITCHES];
#endif

    bool  debug_digital_hatswitch;

    int   joy_buttons;
    bool  joy_available;
    float joy_xmin, joy_xmax, joy_ymin, joy_ymax, joy_zmin, joy_zmax;
    float joy_x, joy_y, joy_z;
    float deadzone;
}
;

extern JoyStick *joystick[MAX_JOYSTICKS];
typedef void (*JoyHandler)( KBSTATE, float x, float y, int mod );
void BindJoyKey( int key, int joystick, KBHandler handler, const KBData &data );
void UnbindJoyKey( int joystick, int key );

void UnbindHatswitchKey( int hatswitch, int val_index );
void BindHatswitchKey( int hatswitch, int val_index, KBHandler handler, const KBData &data );

void BindDigitalHatswitchKey( int joystick, int hatswitch, int dir_index, KBHandler handler, const KBData &data );
void UnbindDigitalHatswitchKey( int joystick, int hatswitch, int dir_index );

#endif //_JOYSTICK_H_

