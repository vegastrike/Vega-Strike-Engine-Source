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

#include "cmd/unit_generic.h"
#include "animation.h"
#include "aux_texture.h"
#include "camera.h"
#include "lin_time.h"
#include <stack>
#include "vsfilesystem.h"
#include "vs_globals.h"
#include "point_to_cam.h"
#include "config_xml.h"
#include "xml_support.h"
#include "sprite.h"
#include <algorithm>
using std::vector;
using std::stack;
static vector<Animation *> far_animationdrawqueue;
static vector
<Animation *> animationdrawqueue;
const unsigned char ani_up=1;
const unsigned char ani_close=2;
const unsigned char ani_alpha=4;
const unsigned char ani_repeat=8;
Animation::Animation ()
{
  VSCONSTRUCT2('a')
	height = 0.001F;
	width = 0.001F;
}
void Animation::SetFaceCam(bool face) {
  if (face) {
    options|=ani_up;
  }else {
    options &= (~ani_up);
  }
}

using namespace VSFileSystem;

Animation::Animation (VSFileSystem::VSFile * f, bool Rep,  float priority,enum FILTER ismipmapped,  bool camorient, bool appear_near_by_radius, const GFXColor &c) : mycolor(c)
{
}

Animation::Animation (const char * FileName, bool Rep,  float priority,enum FILTER ismipmapped,  bool camorient, bool appear_near_by_radius, const GFXColor &c) : mycolor(c)
{	
  Identity(local_transformation);
  VSCONSTRUCT2('a')
  //  repeat = Rep;
  options=0;
  if (Rep)
    options|=ani_repeat;
  if (camorient) {
    options |= ani_up;
  }
  if (appear_near_by_radius) {
    options|=ani_close;
  }
  VSFile f;
  VSError err = f.OpenReadOnly( FileName, AnimFile);
  if (err>Ok) {
    //load success already set false
  } else {
    f.Fscanf ( "%f %f", &width, &height);
    if (width>0) {
      options|=ani_alpha;
    }
    width = fabs(width*0.5F);
    height = height*0.5F;
    Load (f,0,ismipmapped);
    f.Close();
  }
  //VSFileSystem::ResetCurrentPath();
}
Animation:: ~Animation () {
  vector <Animation   *>::iterator i;
  while ( (i=std::find(far_animationdrawqueue.begin(),far_animationdrawqueue.end(),this))!=far_animationdrawqueue.end()) {
    far_animationdrawqueue.erase(i);
  }
  while ( (i=std::find(animationdrawqueue.begin(),animationdrawqueue.end(),this))!=animationdrawqueue.end()) {
    animationdrawqueue.erase(i);
  }

  VSDESTRUCT2  
}
void Animation::SetPosition (const QVector &p) {
  local_transformation.p = p;
}
void Animation::SetOrientation(const Vector &p, const Vector &q, const Vector &r)
{	
  VectorAndPositionToMatrix (local_transformation, p,q,r,local_transformation.p);
}
QVector Animation::Position()
{
  return local_transformation.p;
}
void Animation:: SetDimensions(float wid, float hei) {
  width = wid;
  height = hei;
}
void Animation:: GetDimensions(float &wid, float &hei) {
  wid = width;
  hei = height;
}

void Animation::ProcessDrawQueue () {
  GFXBlendMode (SRCALPHA,INVSRCALPHA);
  GFXDisable (LIGHTING);
  GFXEnable(TEXTURE0);
  GFXDisable(TEXTURE1);
  GFXDisable (DEPTHWRITE);	
  ProcessDrawQueue (animationdrawqueue,-FLT_MAX);
}
void Animation::ProcessFarDrawQueue (float farval) {
  //set farshit
  GFXBlendMode (SRCALPHA,INVSRCALPHA);
  GFXDisable (LIGHTING);
  GFXEnable(TEXTURE0);
  GFXDisable(TEXTURE1);

  GFXDisable (DEPTHWRITE);	
  GFXDisable (DEPTHTEST);	

  ProcessDrawQueue (far_animationdrawqueue, farval);
}
void Animation::ProcessDrawQueue (std::vector <Animation *> &animationdrawqueue,float limit) {
  if (g_game.use_animations==0&&g_game.use_textures==0) {
    return;
  }

  unsigned char alphamaps=ani_alpha;
	int i;//NOT UNSIGNED
	for (i=animationdrawqueue.size()-1;i>=0;i--) {
    GFXColorf (animationdrawqueue[i]->mycolor);//fixme, should we need this? we get som egreenie explosions
    Matrix result;
    if (alphamaps!=(animationdrawqueue[i]->options&ani_alpha)) {
      alphamaps = (animationdrawqueue[i]->options&ani_alpha);
      GFXBlendMode ((alphamaps!=0)?SRCALPHA:ONE,(alphamaps!=0)?INVSRCALPHA:ONE);
    }
	if ((animationdrawqueue[i]->Position()-_Universe->AccessCamera()->GetPosition()).Magnitude()-animationdrawqueue[i]->height>limit) {
  	  GFXFogMode(FOG_OFF);
  	  animationdrawqueue[i]->CalculateOrientation(result);
      animationdrawqueue[i]->DrawNow(result);
      animationdrawqueue.erase (animationdrawqueue.begin()+i);
	}
  }
}
void Animation::CalculateOrientation (Matrix & result) {
  Vector camp,camq,camr;
  QVector pos (Position());
  float hei=height;
  float wid=width;
  static float HaloOffset = XMLSupport::parse_float(vs_config->getVariable ("graphics","HaloOffset",".1"));
  ::CalculateOrientation (pos,camp,camq,camr,wid,hei,(options&ani_close)?HaloOffset:0,false,(options&ani_up)?NULL:&local_transformation);
  
  /*
  Camera* TempCam = _Universe->AccessCamera();
  TempCam->GetPQR(camp,camq,camr);
  if (!camup){
    Vector q1 (local_transformation[1],local_transformation[5],local_transformation[9]);
    Vector p1 ((q1.Dot(camq))*camq);
    camq = (q1.Dot(camp))*camp+p1;			
    Vector posit;
    TempCam->GetPosition (posit);
    camr.i = -local_transformation[12]+posit.i;
    camr.j = -local_transformation[13]+posit.j;
    camr.k = -local_transformation[14]+posit.k;
    Normalize (camr);
    ScaledCrossProduct (camq,camr,camp);		
    ScaledCrossProduct (camr,camp,camq); 
    //if the vectors are linearly dependant we're phucked :) fun fun fun
  }
  */
  VectorAndPositionToMatrix (result, camp,camq,camr,pos);    
}
void Animation::DrawNow(const Matrix &final_orientation) {
 
  if (g_game.use_animations==0&&g_game.use_textures==0) {
    
  }else if (!Done()||(options&ani_repeat)) {
    GFXLoadMatrixModel (final_orientation);
    MakeActive();
    GFXBegin (GFXQUAD);
    GFXTexCoord2f (0.00F,1.00F);
    GFXVertex3f (-width,-height,0.00F);  //lower left
    GFXTexCoord2f (1.00F,1.00F);
    GFXVertex3f (width,-height,0.00F);  //upper left
    GFXTexCoord2f (1.00F,0.00F);
    GFXVertex3f (width,height,0.00F);  //upper right
    GFXTexCoord2f (0.00F,0.00F);
    GFXVertex3f (-width,height,0.00F);  //lower right
    GFXEnd ();
   
  }
}
void Animation::DrawAsVSSprite (VSSprite * spr) {
  if (!spr) {
    return;
  }
  if (g_game.use_animations!=0||g_game.use_textures!=0) {
  //  unsigned char alphamaps=ani_alpha;
    GFXPushBlendMode();
    if (options&ani_alpha)
      GFXBlendMode (SRCALPHA,INVSRCALPHA);
    else
      GFXBlendMode (ONE, ZERO);
    MakeActive();
    GFXDisable(CULLFACE);
    GFXBegin(GFXQUAD);
    Vector ll,lr,ur,ul;
    spr->DrawHere(ll,lr,ur,ul);
    GFXTexCoord2f(0, 1);
    GFXVertexf(ll);
    GFXTexCoord2f(1, 1);
    GFXVertexf(lr);
    GFXTexCoord2f(1, 0);
    GFXVertexf(ur);
    GFXTexCoord2f(0, 0);
    GFXVertexf(ul);
    GFXEnd();
    GFXEnable(CULLFACE);
    GFXPopBlendMode();
  }
}
void Animation::DrawNoTransform(bool cross, bool blendoption) {
  if (g_game.use_animations==0&&g_game.use_textures==0) {
    
  }else
  if (!Done()||(options&ani_repeat)) {
    MakeActive();
    if (blendoption){
      if (options&ani_alpha) {
        GFXEnable(DEPTHWRITE);
        GFXBlendMode (SRCALPHA,INVSRCALPHA);
      }else {
        GFXBlendMode(ONE,ONE);
      }
    }
    GFXBegin (GFXQUAD);
    GFXTexCoord2f (0.00F,1.00F);
    GFXVertex3f (-width,-height,0.00F);  //lower left
    GFXTexCoord2f (1.00F,1.00F);
    GFXVertex3f (width,-height,0.00F);  //upper left
    GFXTexCoord2f (1.00F,0.00F);
    GFXVertex3f (width,height,0.00F);  //upper right
    GFXTexCoord2f (0.00F,0.00F);
    GFXVertex3f (-width,height,0.00F);  //lower right
    if (cross) {
    GFXTexCoord2f (0.00F,1.00F);
    GFXVertex3f (-width,0.00F,-height);  //lower left
    GFXTexCoord2f (1.00F,1.00F);
    GFXVertex3f (width,0,-height);  //upper left
    GFXTexCoord2f (1.00F,0.00F);
    GFXVertex3f (width,0,height);  //upper right
    GFXTexCoord2f (0.00F,0.00F);
    GFXVertex3f (-width,0,height);  //lower right
    GFXTexCoord2f (0.00F,1.00F);    
    GFXVertex3f (0,-height,-height);  //lower left
    GFXTexCoord2f (1.00F,1.00F);
    GFXVertex3f (0,height,-height);  //upper left
    GFXTexCoord2f (1.00F,0.00F);
    GFXVertex3f (0,height,height);  //upper right
    GFXTexCoord2f (0.00F,0.00F);
    GFXVertex3f (0,-height,height);  //lower right
    }
    GFXEnd ();
    if (blendoption) {
      if (options&ani_alpha) {
        GFXDisable(DEPTHWRITE);
      }
    }
       
  }
}
void Animation:: Draw() {
  if (g_game.use_animations!=0||g_game.use_textures!=0) {
    static float HaloOffset = XMLSupport::parse_float(vs_config->getVariable ("graphics","HaloOffset",".1"));
    QVector R (_Universe->AccessCamera()->GetR().i,_Universe->AccessCamera()->GetR().j,_Universe->AccessCamera()->GetR().k);
    static float too_far_dist = XMLSupport::parse_float (vs_config->getVariable ("graphics","anim_far_percent",".8"));
    if ((R.Dot (Position()-_Universe->AccessCamera()->GetPosition())-HaloOffset*(height>width?height:width))<too_far_dist*g_game.zfar   ) {
      animationdrawqueue.push_back (this);
    }else {
      far_animationdrawqueue.push_back(this);
    }
  }
}

