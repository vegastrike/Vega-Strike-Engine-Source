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
#ifndef SPRITE_H
#define SPRITE_H

#include "gfxlib.h"

#include "quaternion.h"

namespace VSFileSystem
{
	class VSFile;
};

struct Texture;
class Sprite {
	float xcenter;
	float ycenter;
	float widtho2;
	float heighto2;
	float maxs,maxt;
	float rotation;

	Texture *surface;

public:
	Sprite (const char *file, enum FILTER texturefilter=BILINEAR, GFXBOOL force=GFXFALSE);
	~Sprite();
	bool LoadSuccess() {return surface!=NULL;}
	void Draw();
	void DrawHere (Vector &ll, Vector &lr, Vector &ur, Vector &ul);
	void Rotate(const float &rad){ rotation += rad;};
	void SetST (const float s, const float t);
	void SetPosition(const float &x1, const float &y1);
	void GetPosition(float &x1, float &y1);
        void SetSize (float s1, float s2);
        void GetSize (float &x1, float &y1);
	void SetRotation(const float &rot);
	void GetRotation(float &rot);
	void ReadTexture( VSFileSystem::VSFile * f);
	//float &Rotation(){return rotation;};
};

#endif
