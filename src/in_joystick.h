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
#include "in_handler.h"

extern void ProcessJoystick();
extern void InitJoystick();

class JoyStick {
    public:
    // initializes the joystick
    JoyStick(int);
    // engine calls GetJoyStick to get coordinates and buttons
    void GetJoyStick(float &x,float &y,int &buttons);
    bool isAvailable(void);
    bool is_around(float axe, float hswitch);
#if defined(HAVE_SDL)
    SDL_Joystick *joy;
#endif
    int nr_of_axes,nr_of_buttons;
    int hat_margin;

    int joy_buttons;
    bool joy_available;
    float joy_xmin,joy_xmax,joy_ymin,joy_ymax;
    float joy_x,joy_y;
    float  deadzone;
}
;
extern int num_joystick;
extern JoyStick **joystick;
#endif // _JOYSTICK_H_

