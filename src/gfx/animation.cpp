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


#include "animation.h"
#include "aux_texture.h"
#include "camera.h"
#include "lin_time.h"
#include <stack>
#include "vs_path.h"
using std::stack;

static stack<Animation *> animationdrawqueue;

Animation::Animation ()
{
	height = 0.001F;
	width = 0.001F;
}

Animation::Animation (const char * FileName, bool Rep,  float priority,enum FILTER ismipmapped,  bool camorient)
{	
  vschdir ("animations");
  vschdir (FileName);
  repeat = Rep;
  camup = camorient;
  FILE * fp = fopen (FileName, "r");
  if (!fp)
    ; // do something 
  else {
    fscanf (fp, "%f %f", &width, &height);
    alphamaps=(width>0);
    width = fabs(width*0.5F);
    height = height*0.5F;
    Load (fp,0,ismipmapped);
    fclose (fp);
  }
  vscdup();
  vscdup();
}
Animation:: ~Animation () {
  
}
void Animation::SetPosition (const float x,const float y, const float z) {
  local_transformation [12] = x;
  local_transformation [13] = y;
  local_transformation [14] = z;
}

void Animation::SetPosition (const Vector &k) {
  local_transformation [12] = k.i;
  local_transformation [13] = k.j;
  local_transformation [14] = k.k;  
}

void Animation::SetOrientation(const Vector &p, const Vector &q, const Vector &r)
{	
  VectorAndPositionToMatrix (local_transformation, p,q,r,Vector (local_transformation[12],local_transformation[13], local_transformation[14]));
}

Vector Animation::Position()
{
  return Vector(local_transformation[12], local_transformation[13], local_transformation[14]);
}

void Animation:: SetDimensions(float wid, float hei) {
  width = wid;
  height = hei;
}
void Animation:: SetDimensions(float &wid, float &hei) {
  wid = width;
  hei = height;
}

void Animation::ProcessDrawQueue () {
  bool alphamaps=true;
  GFXBlendMode (SRCALPHA,INVSRCALPHA);
  GFXDisable (LIGHTING);
  GFXEnable(TEXTURE0);
  GFXDisable(TEXTURE1);
  GFXDisable (DEPTHWRITE);
  GFXColor4f (1,1,1,1);//fixme, should we need this? we get som egreenie explosions
  while (!animationdrawqueue.empty()) {
    Matrix result;
    if (alphamaps!=animationdrawqueue.top()->alphamaps) {
      alphamaps = !alphamaps;
      GFXBlendMode (alphamaps?SRCALPHA:ONE,alphamaps?INVSRCALPHA:ONE);
    }
    animationdrawqueue.top()->CalculateOrientation(result);
    animationdrawqueue.top()->DrawNow(result);

    animationdrawqueue.pop();
  }
}

void Animation::CalculateOrientation (Matrix & result) {
  Vector camp,camq,camr;
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
  VectorAndPositionToMatrix (result, camp,camq,camr,Position());    
}

void Animation::DrawNow(const Matrix &final_orientation) {
 
  if (!Done()) {
    GFXLoadMatrix (MODEL, final_orientation);
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
void Animation::DrawNoTransform() {

  if (!Done()) {
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
    
    GFXEnd ();
   
  }
}

void Animation:: Draw(const Transformation &, const float *) {
  animationdrawqueue.push (this);
}

