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
#ifndef LOGO_H_
#define LOGO_H_

#include "gfxlib.h"

//struct glVertex;
struct Texture;
class Vector;
class Logo{
	int numlogos;
	//glVertex **LogoCorner;

	GFXVertexList **vlists;
	Texture* Decal;
public:
	Logo(int numberlogos,Vector* center, Vector* normal, float* sizes, float* rotations, float offset, Texture * Dec, Vector *Ref);
	Logo(const Logo &rval){*this = rval;}
	~Logo ();

	void SetDecal(Texture *decal)
	{
		Decal = decal;
	}
	void Draw();

};
#endif
