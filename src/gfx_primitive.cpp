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
}

Primitive:: Primitive ()
{
	InitPrimitive();
}

Primitive::~Primitive()
{
}

void Primitive::SetPosition (float x,float y, float z) {
  local_transformation.position = Vector (x,y,z);
}
void Primitive::SetPosition (const Vector &k) {
  local_transformation.position = k;
}
void Primitive::SetOrientation(const Vector &p, const Vector &q, const Vector &r)
{	
  local_transformation.orientation = Quaternion::from_vectors(p,q,r);
}

Vector &Primitive::Position()
{
	return local_transformation.position;
}

