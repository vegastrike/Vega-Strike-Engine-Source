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
#include <queue>
#include <list>
using std::queue;
using std::list;

#include "in_kb.h"
#include "in_mouse.h"
#include "in_joystick.h"

extern KBSTATE keyState[LAST_MODIFIER][KEYMAP_SIZE];


void ProcessInput( size_t whichplayer )
{
    ProcessKB( whichplayer );
    ProcessMouse();
    for (int i = 0; i < MAX_JOYSTICKS; i++)
        if (joystick[i]->player == whichplayer)
            ProcessJoystick( i );
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

