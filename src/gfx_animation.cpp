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

//#include "wrapgfx.h"
#include "gfx_animation.h"
#include "gfx_camera.h"
//#include "glob.h"
//#include "win.h"
#include "lin_time.h"
Animation::Animation ():Primitive()
{
	cumtime = 0;
	numframes = 0;
	timeperframe = 0.001F;
	height = 0.001F;
	width = 0.001F;
	Decal = NULL;
}

Animation::Animation (char * FileName):Primitive()
{
	cumtime = 0;
	char temp [256];
	char tempalp [256];
	FILE * fp = fopen (FileName, "r+b");
	if (!fp)
		; // do something 
	fscanf (fp,"%d",&numframes);
	fscanf (fp, "%f",&timeperframe);
	fprintf (stderr,"timeperframe: %f",timeperframe);
	//	fread (&numframes, sizeof (short), 1, fp); 
	Decal = new Texture* [numframes];
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
	  Decal[i] = new Texture (temp,tempalp);
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
	//if(!copied)
	//{
		for (int i=0; i< numframes; i++)
			delete Decal[i];
		delete [] Decal;
	//}
}

void Animation:: SetDimensions(float wid, float hei) {
  width = wid;
  height = hei;
}

void Animation:: Draw()
{
  float eee;
  if (cumtime==0&&GetElapsedTime()>=timeperframe)
    cumtime +=timeperframe;
  else
    cumtime += GetElapsedTime();
		int framenum = (int)(cumtime/timeperframe);
		if (framenum<numframes)
		{
			GFXDisable (LIGHTING);
			//glMatrixMode(GL_MODELVIEW);
			Vector p1,q1,r1;
			Vector camp,camq,camr;
			Camera* TempCam = _GFX ->AccessCamera();
			
			_GFX->AccessCamera()->GetPQR(camp,camq,camr);
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
			static float ShipMat [16];
			VectorToMatrix (ShipMat,p1,q1,r1);
			Translate(translation, pos.i, pos.j, pos.k);
			MultMatrix(transformation, translation, ShipMat);

			GFXLoadMatrix (MODEL, transformation);
			//glDisable(GL_TEXTURE_2D);
			//Decal[framenum]->Transfer();//frame stuff needs to be done
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
			//glEnable(GL_TEXTURE_2D);
			GFXEnable (LIGHTING);
		}
		else
			framenum = 0;

}
