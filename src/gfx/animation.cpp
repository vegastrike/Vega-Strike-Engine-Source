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
	cumtime = 0;
	numframes = 0;
	timeperframe = 0.001F;
	height = 0.001F;
	width = 0.001F;
	Decal = NULL;
}

Animation::Animation (const char * FileName, bool Rep,  float priority,enum FILTER ismipmapped,  bool camorient)
{	
  vschdir ("animations");
  vschdir (FileName);
  repeat = Rep;
	cumtime = 0;
	camup = camorient;
	char temp [256];
	char tempalp [256];
	float alp;
	int zeroval;
	FILE * fp = fopen (FileName, "r");
	if (!fp)
		; // do something 
	fscanf (fp,"%d %f",&numframes,&timeperframe);
	//	fread (&numframes, sizeof (short), 1, fp); 
	Decal = new Texture* [numframes];
	fscanf (fp,"%f %d",&alp,&zeroval);
	//	fread (&timeperframe,sizeof (float),1,fp);
	float tmp;
	//fread (&tmp, sizeof (float),1,fp);
	fscanf (fp, "%f %f", &width, &height);
	alphamaps = width>0;
	width = fabs(width*0.5F);
	//fread (&tmp, sizeof (float),1,fp);
	height = height*0.5F;
	for (int i=0; i<numframes;i++) //load all textures
	{
	  if (alphamaps) {
	    fscanf (fp,"%s %s", temp, tempalp);
	    Decal[i] = new Texture (temp,tempalp, 0,ismipmapped, TEXTURE2D, TEXTURE_2D,alp,zeroval);
	  } else {
	    fscanf (fp, "%s", temp);
	    Decal[i] = new Texture (temp, 0,ismipmapped, TEXTURE2D, TEXTURE_2D);
	  }
	  Decal[i]->Prioritize (priority);//standard animation priority
	  /*int j;
	    for (j=0; ;j++)
	    {
	    fread (&temp[j],sizeof (char), 1, fp);
	    if (!temp[j]) //ahh we have come upon a NULL
	    {
	    break;
	    }
	    }
	    for (j=0; ;j++)
	    {
	    fread (&tempalp[j],sizeof (char), 1, fp);
	    if (!tempalp[j]) //ahh we have come upon a NULL
	    {
	    Decal[i] = new Texture (temp,tempalp);
	    break;
	    }
	    }*/
	}
	fclose (fp);
	vscdup();
	vscdup();
}
Animation:: ~Animation ()
{
  for (int i=0; i< numframes; i++)
    delete Decal[i];
  delete [] Decal;
  
}
void Animation::SetPosition (float x,float y, float z) {
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

//FIXME:: possibly not portable
Vector Animation::Position()
{
  return Vector(local_transformation[12], local_transformation[13], local_transformation[14]);
}

void Animation:: SetDimensions(float wid, float hei) {
  width = wid;
  height = hei;
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
    animationdrawqueue.top()->UpdateTime (GetElapsedTime());
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
 
  int framenum = (int)(cumtime/timeperframe);
  if (!Done()) {
    GFXLoadMatrix (MODEL, final_orientation);
    Decal[framenum]->MakeActive();
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
 int framenum = (int)(cumtime/timeperframe);
  if (!Done()) {
    Decal[framenum]->MakeActive();
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
void Animation::UpdateTime( float elapsedtime)
{  
  int framenum = (int)(cumtime/timeperframe);
  if (elapsedtime>=timeperframe)
    cumtime +=timeperframe;
  else
    cumtime += elapsedtime;
  if (repeat&&framenum>=numframes)
    cumtime =0;
}    

void Animation:: Draw(const Transformation &, const float *) {
  animationdrawqueue.push (this);
}

bool Animation::Done () {
  int framenum = (int)(cumtime/timeperframe);
  return (framenum>=numframes);
}
