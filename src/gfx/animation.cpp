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

Animation::Animation (char * FileName, bool Rep,  float priority,enum FILTER ismipmapped,  bool camorient)
{	
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
	width = width*0.5F;
	//fread (&tmp, sizeof (float),1,fp);
	height = height*0.5F;
	for (int i=0; i<numframes;i++) //load all textures
	{
	  fscanf (fp,"%s %s", temp, tempalp);
	  Decal[i] = new Texture (temp,tempalp, 0,ismipmapped, TEXTURE2D, TEXTURE_2D,alp,zeroval);
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
}
Animation:: ~Animation ()
{
  for (int i=0; i< numframes; i++)
    delete Decal[i];
  delete [] Decal;
  
}
void Animation::SetPosition (float x,float y, float z) {
  local_transformation.position = Vector (x,y,z);
}
void Animation::SetPosition (const Vector &k) {
  local_transformation.position = k;
}
void Animation::SetOrientation(const Vector &p, const Vector &q, const Vector &r)
{	
  local_transformation.orientation = Quaternion::from_vectors(p,q,r);
}

Vector &Animation::Position()
{
	return local_transformation.position;
}

void Animation:: SetDimensions(float wid, float hei) {
  width = wid;
  height = hei;
}

void Animation::ProcessDrawQueue () {
  GFXBlendMode (SRCALPHA,INVSRCALPHA);
  GFXDisable (LIGHTING);
  GFXEnable(TEXTURE0);
  GFXDisable(TEXTURE1);
  GFXDisable (DEPTHWRITE);
  while (!animationdrawqueue.empty()) {
    animationdrawqueue.top()->DrawNow();
    animationdrawqueue.pop();
  }
}

void Animation::DrawNow() {
  Matrix orientation;
  Vector pos = local_transformation.position;

  local_transformation.orientation.to_matrix(orientation);
  int framenum = (int)(cumtime/timeperframe);
  if (!Done()) {

    Vector p1,q1,r1;
    Vector camp,camq,camr;
    Camera* TempCam = _Universe->AccessCamera();
    _Universe->AccessCamera()->GetPQR(camp,camq,camr);
    if (camup)  {
      p1=camp; q1 =  camq ; r1=camr;
    }else{
      q1.i = orientation[1];
      q1.j = orientation[5];
      q1.k = orientation[9];
      
      p1 = (q1.Dot(camq))*camq;
      q1 = (q1.Dot(camp))*camp+p1;			
      Vector posit;
      TempCam->GetPosition (posit);
      r1.i = -pos.i+posit.i;
      r1.j = -pos.j+posit.j;
      r1.k = -pos.k+posit.k;
      Normalize (r1);

      ScaledCrossProduct (q1,r1,p1);		
      ScaledCrossProduct (r1,p1,q1); 
      //if the vectors are linearly dependant we're phucked :) fun fun fun
    }
    static float ShipMat [16];
    Matrix translation, transformation;
    VectorToMatrix (ShipMat,p1,q1,r1);
    Translate(translation, pos.i, pos.j, pos.k);
    MultMatrix(transformation, translation, ShipMat);
    
    GFXLoadMatrix (MODEL, transformation);

    Decal[framenum]->MakeActive();
    GFXBegin (QUADS);
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
  
  if (GetElapsedTime()>=timeperframe)
    cumtime +=timeperframe;
  else
    cumtime += GetElapsedTime();
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
