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

#ifndef _MOUSE_H_
#define _MOUSE_H_
#include "in.h"

extern int mousex, mousey;
int getMouseButtonStatus ();//returns button status that are bitwise anded (i.e. 1,3 down  the number looks like 0x1|(0x1<<2)
void InitMouse();
void RestoreMouse();
void ProcessMouse();
void BindKey(int key,MouseHandler handler);
void UnbindMouse (int key);
int getMouseDrawFunc();
void GetMouseDelta (int &dx, int &dy);
int lookupMouseButton (int winsys_button);//returns 0 for left click, 1 for middle, 2 for right. ....
#endif



