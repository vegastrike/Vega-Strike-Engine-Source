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
#include <iostream.h>

#include "gfx_aux_texture.h"
#include "gfx_sprite.h"
#include "gfx_transform.h"
#include "gfxlib.h"
#include "vegastrike.h"
#include <assert.h>
static float *mview = NULL;

Sprite::Sprite(char *file, bool trackzoom):Mesh(), track_zoom(trackzoom)
{
  local_transformation.position = Vector(0,0,1.0001);

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


void Sprite::UpdateHudMatrix() {
  assert(0);
}

void Sprite::Draw(const Transformation &dtrans, const Matrix m)
{
  Matrix tmatrix;

  Vector camp,camq,camr;
  _GFX->AccessCamera()->GetPQR(camp,camq,camr);
  VectorAndPositionToMatrix (tmatrix,camp,camq,camr,
			     _GFX->AccessCamera()->GetPosition() +
			     camp * local_transformation.position.i +
			     camq * local_transformation.position.j +
			     camr * local_transformation.position.k);
  GFXLoadMatrix(MODEL, tmatrix);
  
	if(surface!=NULL)
	{

		GFXDisable(LIGHTING);
		GFXDisable(DEPTHWRITE);
		GFXDisable(DEPTHTEST);
		GFXPushBlendMode();
		GFXColor4f (1,1,1,1);
		//GFXBlendMode(SRCALPHA, INVSRCALPHA);
		GFXBlendMode(ONE, ONE);
		GFXEnable(TEXTURE0);
		GFXDisable(TEXTURE1);
		surface->MakeActive();


		//GFXVertex(Vector(0.00F, 0.00F, 1.00F), Vector(0.00F, 0.00F, 0.00F), 0.00F, 0.00F),
		//GFXVertex(Vector(xsize, 0.00F, 1.00F), Vector(0.00F, 0.00F, 0.00F), 1.00F, 0.00F),
		//GFXVertex(Vector(xsize, ysize, 1.00F), Vector(0.00F, 0.00F, 0.00F), 1.00F, 1.00F),
		//GFXVertex(Vector(0.00F, ysize, 1.00F), Vector(0.00F, 0.00F, 0.00F), 0.00F, 1.00F)};

		GFXColor(1.00f, 1.00f, 1.00f, 1.00f);
		GFXBegin(QUADS);
		
		GFXTexCoord2f(0.00f, 1.00f);
		GFXVertex3f(left, top, 0.00f);
		GFXTexCoord2f(1.00f, 1.00f);
		GFXVertex3f(right, top, 0.00f);
		GFXTexCoord2f(1.00f, 0.00f);
		GFXVertex3f(right, bottom, 0.00f);
		GFXTexCoord2f(0.00f, 0.00f);
		GFXVertex3f(left, bottom, 0.00f);

		GFXEnd();
		GFXEnable(LIGHTING);
		GFXEnable(DEPTHWRITE);
		GFXEnable(DEPTHTEST);

	}
}

void Sprite::SetPosition(const float &x1, const float &y1)
{
	local_transformation.position.i = x1;
	local_transformation.position.j = y1;
}

void Sprite::GetPosition(float &x1, float &y1)
{
	x1 = local_transformation.position.i;
	y1 = local_transformation.position.j;
}
void Sprite::SetSize (float x1, float y1) {
  right = left+x1;
  bottom = top+y1;
}
void Sprite::GetSize (float &x1,float &y1) {
  x1 = right-left;
  y1 = bottom - top;
}

void Sprite::SetRotation(const float &rot)
{
  local_transformation.orientation *= Quaternion::from_axis_angle(Vector(0,1,0), rot - rotation);
  rotation = rot;
}

void Sprite::GetRotation(float &rot)
{
  rot = rotation;
}
