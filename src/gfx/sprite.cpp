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
#include <iostream>
#include "vsfilesystem.h"
#include "cmd/unit_generic.h"
#include "aux_texture.h"
#include "sprite.h"
#include "matrix.h"
#include "gfxlib.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include <assert.h>
#include <math.h>
#ifndef M_PI_2
# define M_PI_2		1.57079632679489661923	/* pi/2 */
#endif
static float *mview = NULL;

using namespace VSFileSystem;

Sprite::Sprite(const char *file, enum FILTER texturefilter,GFXBOOL force) {
  VSCONSTRUCT2('S')
  xcenter = 0;
  ycenter = 0;
  widtho2 = 0;
  heighto2 = 0;
  rotation = 0;
  surface = NULL;
  maxs = maxt =0;
  VSFile f;
  VSError err = Unspecified;
  if (file[0]!='\0') {
	err = f.OpenReadOnly( file, SpriteFile);
  }
  if (err<=Ok) {
    char texture[64]={0};
    char texturea[64]={0};
    f.Fscanf( "%63s %63s", texture, texturea);
    f.Fscanf( "%f %f", &widtho2, &heighto2);
    f.Fscanf( "%f %f", &xcenter, &ycenter);
    
    widtho2/=2;
    heighto2/=-2;
    surface=NULL;
    if (g_game.use_sprites||force==GFXTRUE) {
      if (texturea[0]=='0') {
		surface = new Texture(texture,0,texturefilter,TEXTURE2D,TEXTURE_2D,GFXTRUE);    
      } else {
		surface = new Texture(texture,texturea,0,texturefilter,TEXTURE2D,TEXTURE_2D,1,0,GFXTRUE);    
      }
      
      if (!surface->LoadSuccess()) {
		delete surface;
		surface = NULL;
      }
    }
    // Finally close file
    f.Close();
  }else {
    widtho2 = heighto2 = 0;
    xcenter = ycenter = 0;
  }
}	

void	Sprite::ReadTexture( VSFileSystem::VSFile * f)
{
	if( !f->Valid())
	{
    	widtho2 = heighto2 = 0;
    	xcenter = ycenter = 0;
		cerr<<"Sprite::ReadTexture error : VSFile not valid"<<endl;
		return;
	}
	surface = new Texture( f);
}

Sprite::~Sprite()
{
  VSDESTRUCT2
  if(surface!=NULL)
    delete surface;
}

void Sprite::SetST (const float s, const float t) {
  maxs = s;
  maxt = t;
}
void Sprite::DrawHere (Vector &ll, Vector &lr, Vector &ur, Vector &ul) {
    if (rotation) {
      const float cw = widtho2*cos(rotation);
      const float sw = widtho2*sin(rotation);
      const float ch = heighto2*cos(M_PI_2+rotation);
      const float sh = heighto2*sin(M_PI_2+rotation);
      const float wnew = cw+ch;
      const float hnew = sw+sh;
      ll=Vector(xcenter-wnew, ycenter+hnew, 0.00f);
      lr=Vector(xcenter+wnew, ycenter+hnew, 0.00f);
      ur=Vector(xcenter+wnew, ycenter-hnew, 0.00f);
      ul=Vector(xcenter-wnew, ycenter-hnew, 0.00f);
    } else {
      ll=Vector(xcenter-widtho2, ycenter+heighto2, 0.00f);
      lr=Vector(xcenter+widtho2, ycenter+heighto2, 0.00f);
      ur=Vector(xcenter+widtho2, ycenter-heighto2, 0.00f);
      ul=Vector(xcenter-widtho2, ycenter-heighto2, 0.00f);
    }
}
void Sprite::Draw()
{
  if (surface){//don't do anything if no surface
    surface->MakeActive();
    GFXDisable (CULLFACE);
    GFXBegin(GFXQUAD);
    Vector ll,lr,ur,ul;
    DrawHere (ll,lr,ur,ul);
    GFXTexCoord2f(maxs, 1);
    GFXVertexf(ll);
    GFXTexCoord2f(1, 1);
    GFXVertexf(lr);
    GFXTexCoord2f(1, maxt);
    GFXVertexf(ur);
    GFXTexCoord2f(maxs, maxt);
    GFXVertexf(ul);
    GFXEnd();
    GFXEnable (CULLFACE);
  }
}

void Sprite::SetPosition(const float &x1, const float &y1) {
  xcenter =x1;
  ycenter = y1;
}

void Sprite::GetPosition(float &x1, float &y1)
{
	x1 = xcenter;
	y1 = ycenter;
}
void Sprite::SetSize (float x1, float y1) {
  widtho2 = x1/2;
  heighto2 = y1/2;
}
void Sprite::GetSize (float &x1,float &y1) {
  x1 = widtho2*2;
  y1 = heighto2*2;
}

void Sprite::SetRotation(const float &rot) {
  rotation = rot;
}

void Sprite::GetRotation(float &rot) {
  rot = rotation;
}

