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

#include "gfxlib.h"
#include "gl_globals.h"
//#include "gfx_transform_vector.h"

static GFXMaterial materialinfo[MAX_NUM_MATERIAL];

BOOL /*GFXDRVAPI*/ GFXSetMaterial(int number, const GFXMaterial &material)
{
	materialinfo[number] = material;
	return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXGetMaterial(int number, GFXMaterial &material)
{
	material = materialinfo[number];
	return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXSelectMaterial(int number)
{
	float matvect[4];
	matvect[0] = materialinfo[number].ar;
	matvect[1] = materialinfo[number].ag;
	matvect[2] = materialinfo[number].ab;
	matvect[3] = materialinfo[number].aa;
	glMaterialfv(GL_FRONT, GL_AMBIENT, matvect);

	matvect[0] = materialinfo[number].dr;
	matvect[1] = materialinfo[number].dg;
	matvect[2] = materialinfo[number].db;
	matvect[3] = materialinfo[number].da;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, matvect);

	matvect[0] = materialinfo[number].sr;
	matvect[1] = materialinfo[number].sg;
	matvect[2] = materialinfo[number].sb;
	matvect[3] = materialinfo[number].sa;
	glMaterialfv(GL_FRONT, GL_SPECULAR, matvect);

	matvect[0] = materialinfo[number].er;
	matvect[1] = materialinfo[number].eg;
	matvect[2] = materialinfo[number].eb;
	matvect[3] = materialinfo[number].ea;
	glMaterialfv(GL_FRONT, GL_EMISSION, matvect);

	glMaterialfv(GL_FRONT, GL_SHININESS, &materialinfo[number].power);
	return TRUE;
}
