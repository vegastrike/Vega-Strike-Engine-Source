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
//#include <gl/gl.h>
//#include <gl/glu.h>
//#include "glob.h"
#include "wrapgfx.h"
#include "gfx_camera.h"
#include "gfx_aux_texture.h"
#include "gfx_background.h"
#include "gfxlib.h"
#include "vegastrike.h"
	const float size = 10;
Background::Background(char *file)
{
	char temp[80]; 
	up = left = down = front=right=back=NULL;
 
	up = new Texture(strcat(temp, "_up.bmp"), 0);
	SphereBackground = NULL;

	if (!up->LoadSuccess()) {
	strcpy(temp, file);
	  strcpy (temp,file);
	  SphereBackground = new SphereMesh (20,8,8,strcat (temp,"_sphere.bmp"),true,true);
	  delete up;
	  up = NULL;
	}else {
	//up->Clamp();
	//up->Filter();

	strcpy(temp, file);
	left = new Texture(strcat(temp, "_left.bmp"), 0);
	//left->Clamp();
	//left->Filter();

	strcpy(temp, file);
	front = new Texture(strcat(temp, "_front.bmp"), 0);
	//front->Clamp();
	//front->Filter();

	strcpy(temp, file);
	right = new Texture(strcat(temp, "_right.bmp"), 0);
	//right->Clamp();
	//right->Filter();

	strcpy(temp, file);
	back = new Texture(strcat(temp, "_back.bmp"), 0);
	//back->Clamp();
	//back->Filter();

	strcpy(temp, file);
	down = new Texture(strcat(temp, "_down.bmp"), 0);
	//down->Clamp();
	//down->Filter();
	}
}

Background::~Background()
{
  if (up) 
    delete up;
  if (left) 
    delete left;
  if (front)
    delete front;
  if (right)
    delete right;
  if (back)
    delete back;
  if (down)
    delete down;
  if (SphereBackground)
    delete SphereBackground;
}

void Background::Draw()
{
  if (SphereBackground) {
    SphereBackground->Draw();
    return;
  }   
	GFXDisable(LIGHTING);
	GFXDisable(DEPTHWRITE);
	GFXTextureAddressMode(CLAMP);
	//glMatrixMode(GL_MODELVIEW);
	GFXLoadIdentity(MODEL);

	GFXSelectTexcoordSet(0, 0);
	/***********************?????????
	//Matrix oldproject;
	//GFXGetMatrix(VIEW, oldproject);
	//glPushMatrix();
	//gluPerspective (90,1.33,0.01,1500); //set perspective to 78 degree FOV
	********************************/
	_GFX ->AccessCamera()->UpdateGLCenter();

	
	/*up*/
	GFXColor4f(1.00F, 1.00F, 1.00F, 1.00F);

	up->MakeActive();
	GFXBegin(QUADS);
	GFXTexCoord2f(0.998F, 0.002F);
	GFXVertex3f(-size, size, size);
	
	GFXTexCoord2f(0.002F, 0.002F);
	GFXVertex3f(-size, size, -size);

	GFXTexCoord2f(0.002F, 0.998F);
	GFXVertex3f(size, size, -size);

	GFXTexCoord2f(0.998F, 0.998F);
	GFXVertex3f(size, size, size);
	GFXEnd();

	/*Left*/
	left->MakeActive();
	GFXBegin(QUADS);
	GFXTexCoord2f(0.998F, 0.002F);
	GFXVertex3f(-size, size, -size);
	
	GFXTexCoord2f(0.002F, 0.002F);
	GFXVertex3f(-size, size, size);

	GFXTexCoord2f(0.002F, 0.998F);
	GFXVertex3f(-size, -size, size);

	GFXTexCoord2f(0.998F, 0.998F);
	GFXVertex3f(-size, -size, -size);
	GFXEnd();
	
	///*Front
	front->MakeActive();
	GFXBegin(QUADS);
	GFXTexCoord2f(0.998F, 0.002F);
	GFXVertex3f(-size, size, size);
	
	GFXTexCoord2f(0.002F, 0.002F);
	GFXVertex3f(size, size, size);

	GFXTexCoord2f(0.002F, 0.998F);
	GFXVertex3f(size, -size, size);

	GFXTexCoord2f(0.998F, 0.998F);
	GFXVertex3f(-size, -size, size);

	GFXEnd();
	
	///*Right
	right->MakeActive();
	GFXBegin(QUADS);
	GFXTexCoord2f(0.998F, 0.002F);
	GFXVertex3f(size, size, size);
	
	GFXTexCoord2f(0.002F, 0.002F);
	GFXVertex3f(size, size, -size);

	GFXTexCoord2f(0.002F, 0.998F);
	GFXVertex3f(size, -size, -size);

	GFXTexCoord2f(0.998F, 0.998F);
	GFXVertex3f(size, -size, size);
	GFXEnd();
	
	///*Back
	back->MakeActive();
	GFXBegin(QUADS);
	GFXTexCoord2f(0.998F, 0.002F);
	GFXVertex3f(size, size, -size);
	
	GFXTexCoord2f(0.002F, 0.002F);
	GFXVertex3f(-size, size, -size);
	
	GFXTexCoord2f(0.002F, 0.998F);
	GFXVertex3f(-size, -size, -size);
	
	GFXTexCoord2f(0.998F, 0.998F);
	GFXVertex3f(size, -size, -size);
			
	GFXEnd();
	
//	/*down
	down->MakeActive();
	GFXBegin(QUADS);
	GFXTexCoord2f(0.002F, 0.998F);
	GFXVertex3f(-size, -size, size);
	
	GFXTexCoord2f(0.998F, 0.998F);
	GFXVertex3f(size, -size, size);
	
	GFXTexCoord2f(0.998F, 0.002F);
	GFXVertex3f(size, -size, -size);
	
	GFXTexCoord2f(0.002F, 0.002F);
	GFXVertex3f(-size, -size, -size);
			
	GFXEnd();//*/

	_GFX ->AccessCamera()->UpdateGFX(false);
	GFXEnable(DEPTHWRITE);
	GFXEnable(LIGHTING);
}
