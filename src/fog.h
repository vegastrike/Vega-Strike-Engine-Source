/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel
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
 * This code taken from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _FOG_H_
#define _FOG_H_

#include "vegastrike.h"

typedef struct {
    bool_t is_on;
    GLint mode;
    GLfloat density;
    GLfloat start;
    GLfloat end;
    GLfloat colour[4];
} fog_t;

void reset_fog();
void setup_fog();
void disable_fog();
bool_t is_fog_on();
GLfloat* get_fog_colour();
  //void register_fog_callbacks( Tcl_Interp *ip );

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
