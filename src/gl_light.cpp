/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn & Alan Shieh
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
#include <GL/gl.h>
#include "gfxlib.h"
#include "vegastrike.h"

BOOL /*GFXDRVAPI*/ GFXEnableLight (int light)
{
	glEnable(GL_LIGHT0+light);
	return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXDisableLight(int light)
{
	glDisable(GL_LIGHT0+light);
	return TRUE;
}

//minor differences between d3d and gl direction and position, will be dealt with if need be
BOOL /*GFXDRVAPI*/ GFXSetLightPosition(int light, const Vector &position_)
{
	float position[4];
	position[0] = position_.i;
	position[1] = position_.j;
	position[2] = position_.k;
	position[3] = 1;
	glLightfv(GL_LIGHT0+light, GL_POSITION, position);
	return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXSetLightDirection(int light, const Vector &direction)
{
	float position[4];
	position[0] = direction.i;
	position[1] = direction.j;
	position[2] = direction.k;
	position[3] = 0;
	glLightfv(GL_LIGHT0+light, GL_POSITION, position);
	return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXSetLightColor(int light, const GFXColor &color)
{
	float args[4];
	args[0] = args[1] = args[2] = args[3] = 0.0;
	
	//glLightfv(GL_LIGHT0+num, GL_DIFFUSE, args);
	glLightfv(GL_LIGHT0+light, GL_SPECULAR, args);
	glLightfv(GL_LIGHT0+light, GL_AMBIENT, args);

	args[0] = color.r;
	args[1] = color.g;
	args[2] = color.b;
	args[3] = color.a;

	glLightfv(GL_LIGHT0+light, GL_DIFFUSE, args);
	return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXSetLightColor(int light, const float color[4])
{
	GFXSetLightColor(light, GFXColor(color[0], color[1], color[2], color[3]));
	return TRUE;
}
