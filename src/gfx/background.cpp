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
#include "star.h"
#include "background.h"
#include "gfxlib.h"
#include "aux_texture.h"
#include "sphere.h"
#include "vs_globals.h"
#include "../gldrv/gl_globals.h"
#include "config_xml.h"
#include <float.h>
	const float size = 100;
Background::Background(const char *file, int numstars, float spread,std::string filename):Enabled (true),stars(NULL) {
	string temp;
        static string starspritetextures = vs_config->getVariable("graphics","far_stars_sprite_texture","");
        static float starspritesize = XMLSupport::parse_float(vs_config->getVariable("graphics","far_stars_sprite_size","2"));

        if(starspritetextures.length()==0) {
          stars= new PointStarVlist(numstars,200/*spread*/,XMLSupport::parse_bool (vs_config->getVariable("graphics","use_star_coords","true"))?filename:"");
        }else {
          stars= new SpriteStarVlist(numstars,200/*spread*/,XMLSupport::parse_bool (vs_config->getVariable("graphics","use_star_coords","true"))?filename:"",starspritetextures,starspritesize);
        }
	up = left = down = front=right=back=NULL;
	static int max_cube_size =XMLSupport::parse_int (vs_config->getVariable("graphics","max_cubemap_size","1024"));
	temp = string(file)+"_up.image";
	up = new Texture(temp.c_str(),0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE,max_cube_size);
	SphereBackground = NULL;
	
	if (!up->LoadSuccess()) {
	  temp = string(file)+"_sphere.image";
	  SphereBackground = new SphereMesh (20,8,8,temp.c_str(),NULL,true);
	  //SphereBackground->Pitch(PI*.5);//that's the way prophecy's textures are set up
	  //SphereBackground->SetOrientation(Vector(1,0,0),
	  //			   Vector(0,0,-1),
	  //			   Vector(0,1,0));//that's the way prophecy's textures are set up
	  delete up;
	  up = NULL;
	}else {
	//up->Clamp();
	//up->Filter();

	temp = string(file)+"_left.image";
	left = new Texture(temp.c_str(),0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE,max_cube_size );
	//left->Clamp();
	//left->Filter();

	temp = string(file)+"_front.image";
	front = new Texture(temp.c_str(),0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE,max_cube_size );
	//front->Clamp();
	//front->Filter();

	temp = string(file)+"_right.image";
	right = new Texture(temp.c_str(),0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE,max_cube_size);
	//right->Clamp();
	//right->Filter();

	temp = string(file)+"_back.image";
	back = new Texture(temp.c_str(),0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE,max_cube_size);
	//back->Clamp();
	//back->Filter();

	temp = string(file)+"_down.image";
	down = new Texture(temp.c_str(),0,MIPMAP,TEXTURE2D,TEXTURE_2D,GFXTRUE,max_cube_size);
	//down->Clamp();
	//down->Filter();
	}
}
void Background::EnableBG(bool tf) {
  Enabled = tf;
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
  if (stars)
    delete stars;
}
Background::BackgroundClone Background::Cache() {
  BackgroundClone ret;
  ret.backups[0]=up?up->Clone():NULL;
  ret.backups[1]=down?down->Clone():NULL;
  ret.backups[2]=left?left->Clone():NULL;
  ret.backups[3]=right?right->Clone():NULL;
  ret.backups[4]=front?front->Clone():NULL;
  ret.backups[5]=back?back->Clone():NULL;
  ret.backups[6]=NULL;
  if (SphereBackground) {
    for (int i=0;i<7&&i<SphereBackground->numTextures();++i) {
      ret.backups[(i+6)%7]=SphereBackground->texture(i)->Clone();
    }
  }
  return ret;
}
void Background::BackgroundClone::FreeClone() {
    for (int i=0;i<7;++i) {
      if (backups[i]) {
        delete backups[i];
        backups[i]=NULL;
      }
    }
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

      //GFXTextureWrap(0,GFXCLAMPTEXTURE);
      //glMatrixMode(GL_MODELVIEW);
      
      /***********************?????????
			      //Matrix oldproject;
			      //GFXGetMatrix(VIEW, oldproject);
			      //glPushMatrix();
			      //gluPerspective (90,1.33,0.01,1500); //set perspective to 78 degree FOV
      ********************************/
      //_Universe->AccessCamera()->UpdateGLCenter();
      
      static struct skybox_rendering_record {
          Texture *tex;
          float vertices[4][3]; //will be *= size
          unsigned char tcoord[4][4]; //S-T-S-T: 0 >= min, 1 => max
      } skybox_rendering_sequence[6] = {
          {   // up
              NULL,
              { {-1,+1,+1},{-1,+1,-1},{+1,+1,-1},{+1,+1,+1} },
              { {1,0,1,0},{0,0,0,0},{0,1,0,1},{1,1,1,1} }
          },
          {   // left
              NULL,
              { {-1,+1,-1},{-1,+1,+1},{-1,-1,+1},{-1,-1,-1} },
              { {1,0,1,0},{0,0,0,0},{0,1,0,1},{1,1,1,1} }
          },
          {   // front
              NULL,
              { {-1,+1,+1},{+1,+1,+1},{+1,-1,+1},{-1,-1,+1} },
              { {1,0,1,0},{0,0,0,0},{0,1,0,1},{1,1,1,1} }
          },
          {   // right
              NULL,
              { {+1,+1,+1},{+1,+1,-1},{+1,-1,-1},{+1,-1,+1} },
              { {1,0,1,0},{0,0,0,0},{0,1,0,1},{1,1,1,1} }
          },
          {   // back
              NULL,
              { {+1,+1,-1},{-1,+1,-1},{-1,-1,-1},{+1,-1,-1} },
              { {1,0,1,0},{0,0,0,0},{0,1,0,1},{1,1,1,1} }
          },
          {   // down
              NULL,
              { {-1,-1,+1},{+1,-1,+1},{+1,-1,-1},{-1,-1,-1} },
              { {0,1,0,1},{1,1,1,1},{1,0,1,0},{0,0,0,0} }
          }
      };
      skybox_rendering_sequence[0].tex = up;
      skybox_rendering_sequence[1].tex = left;
      skybox_rendering_sequence[2].tex = front;
      skybox_rendering_sequence[3].tex = right;
      skybox_rendering_sequence[4].tex = back;
      skybox_rendering_sequence[5].tex = down;

      for (int skr=0; skr<sizeof(skybox_rendering_sequence)/sizeof(skybox_rendering_sequence[0]); skr++) {
          Texture *tex=skybox_rendering_sequence[skr].tex;
          int lyr;
          int numlayers=tex->numLayers();
          bool multitex=(numlayers>1);
          int numpasses=tex->numPasses();
          float ms=tex->mintcoord.i,Ms=tex->maxtcoord.i;
          float mt=tex->mintcoord.j,Mt=tex->maxtcoord.j;
          if (!gl_options.ext_clamp_to_edge) {
              ms += 1.0/tex->boundSizeX; Ms -= 1.0/tex->boundSizeX;
              mt += 1.0/tex->boundSizeY; Mt -= 1.0/tex->boundSizeY;
          }
          float stca[]={ms,Ms},ttca[]={mt,Mt};

          GFXColor4f(1.00F, 1.00F, 1.00F, 1.00F);
          for (lyr=0; (lyr<gl_options.Multitexture)||(lyr<numlayers); lyr++) {
              GFXToggleTexture((lyr<numlayers),lyr);
              if (lyr<numlayers) GFXTextureCoordGenMode(lyr,NO_GEN,NULL,NULL);
          }
          for (int pass=0; pass<numpasses; pass++) if (tex->SetupPass(pass,0,ONE,ZERO)) {
              tex->MakeActive(0,pass);
              GFXActiveTexture(0);
              GFXTextureAddressMode(CLAMP);
              GFXTextureEnv(0,GFXMODULATETEXTURE);

              float s1,t1,s2,t2;

              GFXBegin(GFXQUAD);
              s1 = stca[skybox_rendering_sequence[skr].tcoord[0][0]&1];
              t1 = ttca[skybox_rendering_sequence[skr].tcoord[0][1]&1];
              s2 = stca[skybox_rendering_sequence[skr].tcoord[0][2]&1];
              t2 = ttca[skybox_rendering_sequence[skr].tcoord[0][3]&1];
              if (!multitex) GFXTexCoord2f(s1, t1); else GFXTexCoord4f(s1, t1, s2, t2);
              GFXVertex3f(skybox_rendering_sequence[skr].vertices[0][0]*size,
                          skybox_rendering_sequence[skr].vertices[0][1]*size,
                          skybox_rendering_sequence[skr].vertices[0][2]*size);
              s1 = stca[skybox_rendering_sequence[skr].tcoord[1][0]&1];
              t1 = ttca[skybox_rendering_sequence[skr].tcoord[1][1]&1];
              s2 = stca[skybox_rendering_sequence[skr].tcoord[1][2]&1];
              t2 = ttca[skybox_rendering_sequence[skr].tcoord[1][3]&1];
              if (!multitex) GFXTexCoord2f(s1, t1); else GFXTexCoord4f(s1, t1, s2, t2);
              GFXVertex3f(skybox_rendering_sequence[skr].vertices[1][0]*size,
                          skybox_rendering_sequence[skr].vertices[1][1]*size,
                          skybox_rendering_sequence[skr].vertices[1][2]*size);
              s1 = stca[skybox_rendering_sequence[skr].tcoord[2][0]&1];
              t1 = ttca[skybox_rendering_sequence[skr].tcoord[2][1]&1];
              s2 = stca[skybox_rendering_sequence[skr].tcoord[2][2]&1];
              t2 = ttca[skybox_rendering_sequence[skr].tcoord[2][3]&1];
              if (!multitex) GFXTexCoord2f(s1, t1); else GFXTexCoord4f(s1, t1, s2, t2);
              GFXVertex3f(skybox_rendering_sequence[skr].vertices[2][0]*size,
                          skybox_rendering_sequence[skr].vertices[2][1]*size,
                          skybox_rendering_sequence[skr].vertices[2][2]*size);
              s1 = stca[skybox_rendering_sequence[skr].tcoord[3][0]&1];
              t1 = ttca[skybox_rendering_sequence[skr].tcoord[3][1]&1];
              s2 = stca[skybox_rendering_sequence[skr].tcoord[3][2]&1];
              t2 = ttca[skybox_rendering_sequence[skr].tcoord[3][3]&1];
              if (!multitex) GFXTexCoord2f(s1, t1); else GFXTexCoord4f(s1, t1, s2, t2);      
              GFXVertex3f(skybox_rendering_sequence[skr].vertices[3][0]*size,
                          skybox_rendering_sequence[skr].vertices[3][1]*size,
                          skybox_rendering_sequence[skr].vertices[3][2]*size);
              GFXEnd();
          }
          for (lyr=0; lyr<numlayers; lyr++) 
              GFXToggleTexture(false,lyr);
          tex->SetupPass(-1,0,ONE,ZERO);
      }

      GFXActiveTexture(0);
      GFXTextureAddressMode(WRAP);
      GFXCenterCamera(false);
    }
  }
  //  GFXLoadIdentity(MODEL);
  //GFXTranslate (MODEL,_Universe->AccessCamera()->GetPosition()); 
  GFXCenterCamera (true);
//  GFXEnable(DEPTHWRITE);
  GFXDisable (TEXTURE0);
  GFXDisable (LIGHTING);
  GFXColor (1,1,1,1);
  GFXDisable (TEXTURE1);
  GFXDisable(DEPTHWRITE);
  GFXBlendMode(ONE,ONE);
  static float background_velocity_scale = XMLSupport::parse_float (vs_config->getVariable("graphics","background_star_streak_velocity_scale","0"));
  stars->DrawAll(QVector(0,0,0), _Universe->AccessCamera()->GetVelocity().Scale(background_velocity_scale),_Universe->AccessCamera()->GetAngularVelocity(),true,true);
  GFXBlendMode(ONE,ZERO);
  GFXEnable (DEPTHTEST);
  GFXEnable(DEPTHWRITE);
  GFXCenterCamera(false);
  //  _Universe->AccessCamera()->UpdateGFX(false);

}

