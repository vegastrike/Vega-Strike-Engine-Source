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
#include <stdlib.h>

#include "gfx_aux_texture.h"
#include "gfx_sprite.h"
#include "gfx_transform.h"
#include "gfxlib.h"

static float *mview = NULL;

Sprite::Sprite(char *file):Mesh()
{
  pos.i = 0;
  pos.j = 0;
  pos.k = 1.0;
  Mesh::SetPosition();

  xcenter = 0;
  ycenter = 0;
  width = 0;
  height = 0;
  rotation = 0;
  surface = NULL;
  //changed = TRUE;
  //vlist = NULL;
  
  FILE *fp = fopen(file, "r");
  char texture[64];
  fscanf(fp, "%s", texture);
  fscanf(fp, "%f %f", &width, &height);
  fscanf(fp, "%f %f", &xcenter, &ycenter);
  fclose(fp);
  left = -xcenter;
  right = width - xcenter;
  top = -ycenter;
  bottom = height - ycenter;
  
  surface = new Texture(texture);
  /*
    GFXVertex vertices[4] = { 
    GFXVertex(Vector(0.00F, 0.00F, 1.00F), Vector(0.00F, 0.00F, 0.00F), 0.00F, 0.00F),
    GFXVertex(Vector(xsize, 0.00F, 1.00F), Vector(0.00F, 0.00F, 0.00F), 1.00F, 0.00F),
    GFXVertex(Vector(xsize, ysize, 1.00F), Vector(0.00F, 0.00F, 0.00F), 1.00F, 1.00F),
    GFXVertex(Vector(0.00F, ysize, 1.00F), Vector(0.00F, 0.00F, 0.00F), 0.00F, 1.00F)};
    vlist = new GFXVertexList(4, vertices);
  */
}	

Sprite::~Sprite()
{
	if(surface!=NULL)
		delete surface;
}

void Sprite::Draw()
{
	if(surface!=NULL)
	{
	  //GFXPushBlendMode();
	  //	  GFXBlendMode(ONE, ONE);
		GFXDisable(LIGHTING);
		GFXDisable(DEPTHWRITE);
		GFXDisable(DEPTHTEST);

		UpdateHudMatrix();
		GFXEnable(TEXTURE0);
		surface->MakeActive();

		//GFXVertex(Vector(0.00F, 0.00F, 1.00F), Vector(0.00F, 0.00F, 0.00F), 0.00F, 0.00F),
		//GFXVertex(Vector(xsize, 0.00F, 1.00F), Vector(0.00F, 0.00F, 0.00F), 1.00F, 0.00F),
		//GFXVertex(Vector(xsize, ysize, 1.00F), Vector(0.00F, 0.00F, 0.00F), 1.00F, 1.00F),
		//GFXVertex(Vector(0.00F, ysize, 1.00F), Vector(0.00F, 0.00F, 0.00F), 0.00F, 1.00F)};

		GFXColor(1.00f, 1.00f, 1.00f, 1.00f);
		GFXBegin(QUADS);
		
		GFXTexCoord2f(0.00f, 0.00f);
		GFXVertex3f(left, bottom, 0.00f);
		GFXTexCoord2f(1.00f, 0.00f);
		GFXVertex3f(right, bottom, 0.00f);
		GFXTexCoord2f(1.00f, 0.00f);
		GFXVertex3f(right, top, 0.00f);
		GFXTexCoord2f(0.00f, 0.00f);
		GFXVertex3f(left, top, 0.00f);

		GFXEnd();
		GFXEnable(LIGHTING);
		GFXEnable(DEPTHWRITE);
		GFXEnable(DEPTHTEST);
		//		GFXPopBlendMode();
	}
}

void Sprite::SetPosition(const float &x1, const float &y1)
{
	pos.i = x1;
	pos.j = y1;
}

void Sprite::GetPosition(float &x1, float &y1)
{
	x1 = pos.i;
	y1 = pos.j;
}

void Sprite::SetRotation(const float &rot)
{
	Roll(rot-rotation);
	rotation = rot;
}

void Sprite::GetRotation(float &rot)
{
	rot = rotation;
}
