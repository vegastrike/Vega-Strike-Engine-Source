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
#include "gfx_aux_texture.h"
#include "gfx_mesh.h"
#include "quaternion.h"

class Sprite {
        Transformation local_transformation;
	float xcenter;
	float ycenter;
	float width;
	float height;
	
	float left, right, top, bottom;

	float rotation;

	Texture *surface;
	//GFXVertexList *vlist;

	//BOOL changed;
	bool track_zoom;

public:
	Sprite(char *file, bool trackZoom = false);
	~Sprite();

	void UpdateHudMatrix();
	void Draw(const Transformation &quat = identity_transformation, const Matrix m = identity_matrix);

	void Rotate(const float &rad){ rotation += rad;};

	void SetPosition(const float &x1, const float &y1);
	void GetPosition(float &x1, float &y1);
        void SetSize (float s1, float s2);
        void GetSize (float &x1, float &y1);
	void SetRotation(const float &rot);
	void GetRotation(float &rot);
	//float &Rotation(){return rotation;};
};

#endif
