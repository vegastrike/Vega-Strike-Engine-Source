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
#include "vegastrike.h"
#include "camera.h"
#include "aux_texture.h"
#include "background.h"
#include "gfxlib.h"
#include "aux_texture.h"
#include "sphere.h"
#include "vs_globals.h"
#include "config_xml.h"
#include <float.h>
	const float size = 100;
Background::Background(const char *file, int numstars, float spread):Enabled (true) {
	char * temp=new char [strlen(file)+25];
	GFXVertex * tmpvertex = new GFXVertex [numstars];
	memset (tmpvertex,0,sizeof (GFXVertex)*numstars);	
	for (int j=0;j<numstars;j++) {
	  tmpvertex[j].x = -.5*spread+rand()*1.2*((float)spread/RAND_MAX);
	  tmpvertex[j].y = -.5*spread+rand()*1.2*((float)spread/RAND_MAX);
	  tmpvertex[j].z = -.5*spread+rand()*1.2*((float)spread/RAND_MAX);
	}
	stars= new GFXVertexList (GFXPOINT,numstars,tmpvertex, numstars, false,0);
	up = left = down = front=right=back=NULL;
	strcpy(temp, file);
	static int max_cube_size =XMLSupport::parse_int (vs_config->getVariable("graphics","max_cubemap_size","1024"));
	up = new Texture(strcat(temp, "_up.bmp"),0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE,max_cube_size);
	SphereBackground = NULL;
	
	if (!up->LoadSuccess()) {
	strcpy(temp, file);
	  strcpy (temp,file);
	  SphereBackground = new SphereMesh (20,8,8,strcat (temp,"_sphere.bmp"),NULL,true);
	  //SphereBackground->Pitch(PI*.5);//that's the way prophecy's textures are set up
	  //SphereBackground->SetOrientation(Vector(1,0,0),
	  //			   Vector(0,0,-1),
	  //			   Vector(0,1,0));//that's the way prophecy's textures are set up
	  delete up;
	  up = NULL;
	}else {
	//up->Clamp();
	//up->Filter();

	strcpy(temp, file);
	left = new Texture(strcat(temp, "_left.bmp"),0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE,max_cube_size );
	//left->Clamp();
	//left->Filter();

	strcpy(temp, file);
	front = new Texture(strcat(temp, "_front.bmp"),0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE,max_cube_size );
	//front->Clamp();
	//front->Filter();

	strcpy(temp, file);
	right = new Texture(strcat(temp, "_right.bmp"),0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE,max_cube_size);
	//right->Clamp();
	//right->Filter();

	strcpy(temp, file);
	back = new Texture(strcat(temp, "_back.bmp"),0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE,max_cube_size);
	//back->Clamp();
	//back->Filter();

	strcpy(temp, file);
	down = new Texture(strcat(temp, "_down.bmp"),0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE,max_cube_size);
	//down->Clamp();
	//down->Filter();
	delete [] temp;
	}
}
void Background::EnableBG(bool tf) {
  Enabled = tf;
}
Background::~Background()
{
  delete stars;
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
  GFXClear (Enabled?GFXFALSE:GFXTRUE);
  if (Enabled) {
    GFXBlendMode (ONE,ZERO);
    GFXDisable(LIGHTING);
    GFXDisable(DEPTHWRITE);
    GFXDisable(DEPTHTEST);
    GFXEnable (TEXTURE0);
    GFXDisable (TEXTURE1);
    GFXColor (1,1,1,1);
    if (SphereBackground) {
      SphereBackground->DrawNow(FLT_MAX,true);
      //    Mesh::ProcessUndrawnMeshes();//background must be processed...dumb but necessary--otherwise might collide with other mehses
    } else {
      GFXCenterCamera(true);
      GFXLoadMatrixModel (identity_matrix);
      //GFXLoadIdentity(MODEL);
	  //	  GFXTranslate (MODEL,_Universe->AccessCamera()->GetPosition()); 

      GFXTextureAddressMode(CLAMP);
      //glMatrixMode(GL_MODELVIEW);
      
      GFXSelectTexcoordSet(0, 0);
      /***********************?????????
			      //Matrix oldproject;
			      //GFXGetMatrix(VIEW, oldproject);
			      //glPushMatrix();
			      //gluPerspective (90,1.33,0.01,1500); //set perspective to 78 degree FOV
      ********************************/
      //_Universe->AccessCamera()->UpdateGLCenter();
      
      
      /*up*/
      GFXColor4f(1.00F, 1.00F, 1.00F, 1.00F);
      
      up->MakeActive();
      GFXBegin(GFXQUAD);
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
      GFXBegin(GFXQUAD);
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
      GFXBegin(GFXQUAD);
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
      GFXBegin(GFXQUAD);
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
      GFXBegin(GFXQUAD);
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
      GFXBegin(GFXQUAD);
      GFXTexCoord2f(0.002F, 0.998F);
      GFXVertex3f(-size, -size, size);
      
      GFXTexCoord2f(0.998F, 0.998F);
      GFXVertex3f(size, -size, size);
      
      GFXTexCoord2f(0.998F, 0.002F);
      GFXVertex3f(size, -size, -size);
      
      GFXTexCoord2f(0.002F, 0.002F);
      GFXVertex3f(-size, -size, -size);
      
      GFXEnd();//*/
      GFXCenterCamera(false);
    }
  }
  //  GFXLoadIdentity(MODEL);
  //GFXTranslate (MODEL,_Universe->AccessCamera()->GetPosition()); 
  GFXCenterCamera (true);
  GFXEnable(DEPTHWRITE);
  GFXDisable (TEXTURE0);
  GFXDisable (LIGHTING);
  GFXColor (1,1,1,1);
  GFXDisable (TEXTURE1);

  stars->DrawOnce();
  GFXEnable (DEPTHTEST);
  GFXCenterCamera(false);
  //  _Universe->AccessCamera()->UpdateGFX(false);

}

