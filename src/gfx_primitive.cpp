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
#include "gfx_primitive.h"
#include "gfx_aux.h"

//extern PFNGLCOLORTABLEEXTPROC glColorTable;
int texcnt = 1;

void Primitive::InitPrimitive()
{
	forcelogos = NULL;
	squadlogos = NULL;

	Identity(orientation);
	Identity(translation);
	Identity(transformation);
	Identity(stackstate);
}

Primitive:: Primitive ()
{
	InitPrimitive();
}

Primitive::~Primitive()
{
}

void Primitive::Draw()
{
  //FIXME VEGASTRIKE	static float rot = 0;
}

Vector &Primitive::Position()
{
	return pos;
}

