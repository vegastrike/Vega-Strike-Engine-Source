/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
  Joystick support written by Alexander Rawass <alexannika@users.sourceforge.net>
*/

#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#if defined(HAVE_SDL)
#include <SDL/SDL.h>
#endif

#include "vegastrike.h"
//#include "glob.h"
//#include "dbg.h"
#include "in_kb.h"
#define NUMJBUTTONS 16

class JoyStick;

//typedef void (*JoyHandler) (int);


extern void ProcessJoystick();
extern void InitJoystick();
extern void DeInitJoystick();

extern void BindButton(int button,KBHandler handler);
extern void UnbindButton(int button) ;

const int MAX_JOYSTICKS=10;
const int MAX_BUTTONS=32;
/*
static KBHandler joyBindings[MAX_BUTTONS];
extern KBSTATE buttonState[MAX_BUTTONS];
*/

extern JoyStick *joystick[MAX_JOYSTICKS];

class JoyStick {
    public:
    // initializes the joystick
    JoyStick(int);
    // engine calls GetJoyStick to get coordinates and buttons
    void GetJoyStick(float &x,float &y, float &z, int &buttons);
    bool isAvailable(void);
    bool is_around(float axe, float hswitch);
    int NumButtons();

#if defined(HAVE_SDL)
    SDL_Joystick *joy;
#else
    void *otherdata;//bad form to have an ifdef in a struct
#endif
    int nr_of_axes,nr_of_buttons;
    int hat_margin;

    int joy_buttons;
    bool joy_available;
    float joy_xmin,joy_xmax,joy_ymin,joy_ymax, joy_zmin, joy_zmax;
    float joy_x,joy_y,joy_z;
    float  deadzone;
}
;


extern JoyStick *joystick[MAX_JOYSTICKS];
typedef void (*JoyHandler)(KBSTATE,float x, float y, int mod);
void BindJoyKey (int key, int joystick, KBHandler handler);
void UnbindJoyKey (int joystick, int key);

#endif // _JOYSTICK_H_

