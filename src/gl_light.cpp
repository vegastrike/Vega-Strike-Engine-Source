/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn & Alan Shieh
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
#include <GL/gl.h>
#include "gfxlib.h"
//#include "vegastrike.h"
#include <list>
#include <queue>
#define GFX_DIFFUSE 1
#define GFX_SPECULAR 2
#define GFX_AMBIENT 4
#define GFX_SPOTLIGHT 8;
#define GFX_ATTENUATED 32;


#define GFX_MAX_LIGHTS 8
int GFX_OPTIMAL_LIGHTS=4;
BOOL GFXLIGHTING=FALSE;
//for OpenGL

struct gfx_light_loc {///yay 32 bytes! 1 cache line
  float vect[4];//last is w for positional, otherwise 3 for spec
  float attenuate[3];
  float intensity;///<=0 if disabled
  //  bool enabled;
  gfx_light_loc() {
    vect[0]=vect[1]=vect[2]=vect[3]=0;
    attenuate[0]=attenuate[1]=attenuate[2]=0;
  }
};

struct gfx_light_data {
  int target;//-1 == not in array //-2 == DEAD
  bool enabled;
  float spot[3];
  float exp;
  float diffuse [4];
  int options;//diffuse? spec? amb? spot? atten?
  int changed;//what changed this round
  float specular[4];
  float ambient[4];
  gfx_light_data() {
    diffuse[0]=diffuse[1]=diffuse[2]=0;//openGL defaults
    specular[0]=specular[1]=specular[2]=0;//openGL defaults
    ambient[0]=ambient[1]=ambient[2]=changed=options=0;
    spot[0]=spot[1]=spot[2]=0;
    exp =diffuse[3]=specular[3]=ambient[3]=1;
    enabled=false;
    target =-1;//physical GL light its saved in
  }
};





static int _currentContext=0;

int GLLights[GFX_MAX_LIGHTS]={-1,-1,-1,-1,-1,-1,-1,-1};
int GLLightState[GFX_MAX_LIGHTS]={0};//0 off 1 on 2=processed

vector <vector <gfx_light_loc> > _local_lights_loc;
vector <vector <gfx_light_data> > _local_lights_dat;
vector <GFXColor> _ambient_light;
vector <gfx_light_loc> * _llights_loc=NULL;
vector <gfx_light_data> * _llights_dat=NULL;

float intensity_cutoff=.05;//something that would normally round down
float optintense=.2;
float optsat = .95;
BOOL /*GFXDRVAPI*/ GFXSetCutoff (float ttcutoff) {
  if (ttcutoff<0) 
    return FALSE;
  intensity_cutoff=ttcutoff;
  return TRUE;
}
BOOL /*GFXDRVAPI*/ GFXSetOptimalIntensity (float intensity, float saturate) {
  optintense = intensity;
  optsat = saturate;
  return TRUE;
}
BOOL /*GFXDRVAPI*/ GFXSetOptimalNumLights (int numLights) {
  if (numLights>GFX_MAX_LIGHTS||numLights<0)
    return FALSE;
  GFX_OPTIMAL_LIGHTS=numLights;
  return TRUE;
}


struct light_key {
  int number;
  float intensity_key;
  light_key () {
    intensity_key = number=0;
  }
  light_key (int num, float inte) {
    number = num;
    intensity_key = inte;
  }
};
static bool operator < (light_key tmp1,light_key tmp2) {return tmp1.intensity_key<tmp2.intensity_key;}

priority_queue <light_key> lightQ;
inline void SetLocalCompare (Vector x) {//does preprocessing of intensity for relatively small source 
  float dis, dissqr;
  unsigned int i;
  for (i=0;i<_llights_loc->size();i++) {
    if ((*_llights_loc)[i].intensity>0) {
      if ((*_llights_loc)[i].vect[3]!=0) {
	dissqr = 
	  ((*_llights_loc)[i].vect[0]-x.i)*((*_llights_loc)[i].vect[0]-x.i)+
	  ((*_llights_loc)[i].vect[1]-x.j)*((*_llights_loc)[i].vect[1]-x.j)+
	  ((*_llights_loc)[i].vect[2]-x.k)*((*_llights_loc)[i].vect[2]-x.k);
	dis = sqrtf (dissqr);//should use fast sqrt by nvidia
	
	float intensity=(*_llights_loc)[i].intensity/(
						    (*_llights_loc)[i].attenuate[0] +
						    (*_llights_loc)[i].attenuate[1]*dis +
						    (*_llights_loc)[i].attenuate[2]*dissqr);
			       
	if (intensity>intensity_cutoff) {
	  lightQ.push (light_key(i,intensity));
	}	
      }else {
	if ((*_llights_loc)[i].intensity>intensity_cutoff)
	  lightQ.push (light_key(i,(*_llights_loc)[i].intensity));
      }
    }
  }
}

BOOL /*GFXDRVAPI*/ GFXSetSeparateSpecularColor(BOOL spec) {
  if (spec) {
    glLightModeli (GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
  }else {
    glLightModeli (GL_LIGHT_MODEL_COLOR_CONTROL,GL_SINGLE_COLOR);
  }
  return TRUE;
}
BOOL /*GFXDRVAPI*/ GFXCreateLightContext (int & con_number) {
  static BOOL LightInit=FALSE;
  if (!LightInit) {
    LightInit = TRUE;
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);//don't want lighting coming from infinity....we have to take the hit due to sphere mapping matrix tweaking
    //
  }
  con_number = _local_lights_loc.size();
  _currentContext= con_number;
  _ambient_light.push_back (GFXColor (0,0,0,1));
  _local_lights_loc.push_back (vector <gfx_light_loc>());
  _local_lights_dat.push_back (vector <gfx_light_data>());
  _llights_loc = &_local_lights_loc[con_number];
  _llights_dat = &_local_lights_dat[con_number];
  
  return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXDeleteLightContext(int con_number) {

  _local_lights_loc[con_number]=vector <gfx_light_loc>();
  _local_lights_dat[con_number]=vector <gfx_light_data>();
  return TRUE;
}
BOOL /*GFXDRVAPI*/ GFXSetLightContext (int con_number) {
  _currentContext = con_number;
  _llights_loc = &_local_lights_loc[con_number];
  _llights_dat = &_local_lights_dat[con_number];
  float tmp[4]={_ambient_light[con_number].r,_ambient_light[con_number].g,_ambient_light[con_number].b,_ambient_light[con_number].a};
  glLightModelfv (GL_LIGHT_MODEL_AMBIENT,tmp);
  for (unsigned int i=0;i<_llights_dat->size();i++) {
    (*_llights_dat)[i].target=-1;
  }
  for (unsigned int i=0;i<GFX_MAX_LIGHTS;i++) {
    GLLights[i]=-1;
    GLLightState[i]=0;
  }
  return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXLightContextAmbient (const GFXColor &amb) {
  if (_currentContext >=_ambient_light.size())
    return FALSE;
  (_ambient_light[_currentContext])=amb;
  //  (_ambient_light[_currentContext])[1]=amb.g;
  //  (_ambient_light[_currentContext])[2]=amb.b;
  //  (_ambient_light[_currentContext])[3]=amb.a;
  float tmp[4]={amb.r,amb.g,amb.b,amb.a};
  glLightModelfv (GL_LIGHT_MODEL_AMBIENT,tmp);
  return TRUE;
}


BOOL /*GFXDRVAPI*/ GFXCreateLight (int &light) {
  for (light=0;light<_llights_dat->size();light++) {
    if ((*_llights_dat)[light].target==-2)
      break;
  }
  if (light==_llights_dat->size()) {	
    _llights_dat->push_back (gfx_light_data());
    _llights_loc->push_back (gfx_light_loc());
  }
  return TRUE;
}
BOOL /*GFXDRVAPI*/ GFXDeleteLight (int light) {
  if ((*_llights_dat)[light].target>=0) {
    GLLights[(*_llights_dat)[light].target]=-1;//ensure only valid lights are in the state
  }
  (*_llights_dat)[light].target=-2;
  (*_llights_dat)[light].enabled=false;
  (*_llights_loc)[light].intensity=-1;
  return TRUE;
}



BOOL /*GFXDRVAPI*/ GFXEnableLight (int light) {
  //  assert (_llights);
  //  if (light<0||light>=_llights->size()) 
  //    return FALSE;
  if ((*_llights_dat)[light].target==-2) {
    return FALSE;
  }
  (*_llights_dat)[light].enabled=true;
  (*_llights_loc)[light].intensity = fabs ((*_llights_loc)[light].intensity);
  return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXDisableLight (int light) {
  //  assert (_llights);
  //  if (light<0||light>=_llights->size()) 
  //    return FALSE;
  if ((*_llights_dat)[light].target==-2) {
    return FALSE;
  }
  (*_llights_dat)[light].enabled=false;
  (*_llights_loc)[light].intensity =-fabs ((*_llights_loc)[light].intensity);
  return TRUE;
}

void EnableExisting (unsigned int);
void EnableExistingAttenuated (unsigned int,float);
inline void DisableExisting(int);
void ForceEnable (unsigned int, unsigned int &, unsigned int &);
void ForceEnableAttenuated (unsigned int, float, unsigned int &, unsigned int&);


int newQsize=0;
int newQ [GFX_MAX_LIGHTS];
float AttenuateQ[GFX_MAX_LIGHTS];
static float AttTmp[4];
static float VecT[4];

BOOL /*GFXDRVAPI*/ GFXPickLights (const float * transform) {
  //picks 1-5 lights to use
  // glMatrixMode(GL_MODELVIEW);
  //glPushMatrix();
  //float tm [16]={1,0,0,1000,0,1,0,1000,0,0,1,1000,0,0,0,1};
  //glLoadIdentity();
  //  GFXLoadIdentity(MODEL);
  if (!GFXLIGHTING) return TRUE;
  Vector loc (transform[12],transform[13],transform[14]);
  SetLocalCompare (loc);
  newQsize=0;
  unsigned int i;
  light_key tmpvar;
  unsigned int lightQsize = lightQ.size();
  for (i=0;i<lightQsize;i++) {
    tmpvar = lightQ.top();
    if (i<GFX_MAX_LIGHTS) {
      //do more stuff with intensity before hacking it to hell
      if (i>GFX_OPTIMAL_LIGHTS) {
	if ((tmpvar.intensity_key-optintense)*(GFX_MAX_LIGHTS-GFX_OPTIMAL_LIGHTS) < (i - GFX_OPTIMAL_LIGHTS)*(optsat-optintense))
	  break;//no new lights
      }else {
	if (i==GFX_OPTIMAL_LIGHTS-2&&tmpvar.intensity_key<.25*optintense)  
	  break;//no new lights
	if (i==GFX_OPTIMAL_LIGHTS-1&&tmpvar.intensity_key<.5*optintense)
	  break;//no new lights
	if (i==GFX_OPTIMAL_LIGHTS&&tmpvar.intensity_key < optintense)
	  break;
      }
      newQsize++;
      if ((*_llights_loc)[i].vect[3]) {
	//intensity now has become the attenuation factor
	newQ[i]= tmpvar.number;
	AttenuateQ[i]=tmpvar.intensity_key/(*_llights_loc)[i].intensity;
      }else {
	newQ[i]= tmpvar.number;
	AttenuateQ[i]=0;
      }

    } else {
      break;
    }
    lightQ.pop();
  }
  unsigned int light;
  for (i=0;i<newQsize;i++) {
    light = newQ[i];
    if ((*_llights_dat)[light].target>=0) {
      if (AttenuateQ[i]) 
	EnableExistingAttenuated (light,AttenuateQ[i]);
      else
	EnableExisting (newQ[i]);
      AttTmp[0]=(*_llights_loc)[light].vect[0]-loc.i;
      AttTmp[1]=(*_llights_loc)[light].vect[1]-loc.j;
      AttTmp[2]=(*_llights_loc)[light].vect[2]-loc.k;
      VecT[3]=0;
      VecT[0]=AttTmp[0]*transform[0]+AttTmp[1]*transform[1]+AttTmp[2]*transform[2];
      VecT[1]=AttTmp[0]*transform[4]+AttTmp[1]*transform[5]+AttTmp[2]*transform[6];
      VecT[2]=AttTmp[0]*transform[8]+AttTmp[1]*transform[9]+AttTmp[2]*transform[10];
      glLightfv (GL_LIGHT0+(*_llights_dat)[light].target, GL_POSITION, VecT);
    }
  }
  for (i=0;i<GFX_MAX_LIGHTS;i++) {
    DisableExisting(i);
  }

  // go through newQ and tag all existing lights for enablement
  // disable rest of lights
  // ForceActivate the rest.
  unsigned int tmp=0;//where to start looking for disabled lights
  unsigned int newtarg=0;
  for (i=0;i<newQsize;i++) {
    light = newQ[i];
    if ((*_llights_dat)[light].target==-1) {
      if (AttenuateQ[i]) 
	ForceEnableAttenuated (light,AttenuateQ[i],tmp,newtarg);
      else
	ForceEnable (light,tmp,newtarg); 
      AttTmp[0]=(*_llights_loc)[light].vect[0]-loc.i;
      AttTmp[1]=(*_llights_loc)[light].vect[1]-loc.j;
      AttTmp[2]=(*_llights_loc)[light].vect[2]-loc.k;
      AttTmp[3]=0;
      VecT[0]=AttTmp[0]*transform[0]+AttTmp[1]*transform[1]+AttTmp[2]*transform[2];
      VecT[1]=AttTmp[0]*transform[4]+AttTmp[1]*transform[5]+AttTmp[2]*transform[6];
      VecT[2]=AttTmp[0]*transform[8]+AttTmp[1]*transform[9]+AttTmp[2]*transform[10];
      glLightfv (GL_LIGHT0+(*_llights_dat)[light].target, GL_POSITION, VecT);

      //      glLightfv (GL_LIGHT0+(*llight_dat)[newQ[i]].target, GL_POSITION, /*wrong*/(*_llights_loc)[light].vect);
    }
  }
  //glPopMatrix();
  return TRUE;
}


void DisableExisting (int gl_light) {
  if (GLLightState[gl_light]==1) {
    GLLightState[gl_light]=0;
    glDisable (GL_LIGHT0+gl_light);
  } else {
    if (GLLightState[gl_light]==2) {
      GLLightState[gl_light]=1;
    }
  }
}

void ForceEnable (unsigned int light, unsigned int &tmp, unsigned int &newtarg) {
  for (;tmp<GFX_MAX_LIGHTS;tmp++) {
    if (GLLightState[tmp]==0) {
      if (GLLights[tmp]==-1) {
	//copy all state nfo
	glLightfv (GL_LIGHT0+tmp,GL_DIFFUSE, (*_llights_dat)[light].diffuse);
	glLightfv (GL_LIGHT0+tmp, GL_SPECULAR, (*_llights_dat)[light].specular);
	glLightfv (GL_LIGHT0+tmp, GL_AMBIENT, (*_llights_dat)[light].ambient);
	//POS//	glLightfv (GL_LIGHT0+tmp, GL_POSITION, (*_llights_loc)[light].vect);
      	glLightfv (GL_LIGHT0+tmp, GL_SPOT_DIRECTION, (*_llights_dat)[light].spot);
      	glLightf (GL_LIGHT0+tmp,GL_SPOT_EXPONENT, (*_llights_dat)[light].exp);
       	////glLightf (GL_LIGHT0+tmp, GL_CONSTANT_ATTENUATION, (*_llights_loc)[light].attenuate[0]);
       	////glLightf (GL_LIGHT0+tmp, GL_LINEAR_ATTENUATION, (*_llights_loc)[light].attenuate[1]);
       	////glLightf (GL_LIGHT0+tmp, GL_QUADRATIC_ATTENUATION, (*_llights_loc)[light].attenuate[2]);//they appear to be broken...anyhow they are of great cost...possibly better to just do a per-model attenuation
	(*_llights_dat)[light].target=tmp;
	(*_llights_dat)[light].changed=0;
	GLLights[tmp]=light;//set pointer and backpointer
	GLLightState[tmp]=1;
	glEnable (GL_LIGHT0+tmp);
	tmp++;
	return;
      } 
    }
  }
  for (;newtarg<GFX_MAX_LIGHTS;newtarg++) {
    if (GLLightState[newtarg]==0) {      
      //go through minimizing state changes
      gfx_light_data * olddat = &(*_llights_dat)[GLLights[newtarg]];
      gfx_light_data * newdat = &(*_llights_dat)[light];
      int chg = olddat->changed;
      if (olddat->diffuse[0]!=newdat->diffuse[0]||olddat->diffuse[1]!=newdat->diffuse[1]||olddat->diffuse[2]!=newdat->diffuse[2]||olddat->diffuse[3]!=newdat->diffuse[3]||chg&DIFFUSE) {
	glLightfv (GL_LIGHT0+newtarg,GL_DIFFUSE, (*_llights_dat)[light].diffuse);
      }
      if (olddat->specular[0]!=newdat->specular[0]||olddat->specular[1]!=newdat->specular[1]||olddat->specular[2]!=newdat->specular[2]||olddat->specular[3]!=newdat->specular[3]||chg&SPECULAR) {
	glLightfv (GL_LIGHT0+newtarg, GL_SPECULAR, (*_llights_dat)[light].specular);
      }	
      if (olddat->ambient[0]!=newdat->ambient[0]||olddat->ambient[1]!=newdat->ambient[1]||olddat->ambient[2]!=newdat->ambient[2]||olddat->ambient[3]!=newdat->ambient[3]||chg&AMBIENT) {
	glLightfv (GL_LIGHT0+newtarg, GL_AMBIENT, (*_llights_dat)[light].ambient);
      }
      
      //POS//glLightfv (GL_LIGHT0+newtarg, GL_POSITION, (*_llights_loc)[light].vect);

      if (olddat->spot[0]!=newdat->spot[0]||olddat->spot[1]!=newdat->spot[1]||olddat->spot[2]!=newdat->spot[2]||olddat->exp!=newdat->exp||chg&DIRECTION) {
	glLightfv (GL_LIGHT0+newtarg, GL_SPOT_DIRECTION, (*_llights_dat)[light].spot);
	glLightf (GL_LIGHT0+newtarg, GL_SPOT_EXPONENT, (*_llights_dat)[light].exp);
      }
      ////      glLightf (GL_LIGHT0+newtarg, GL_CONSTANT_ATTENUATION, (*_llights_loc)[light].attenuate[0]);
      ////      glLightf (GL_LIGHT0+newtarg, GL_LINEAR_ATTENUATION, (*_llights_loc)[light].attenuate[1]);
      ////      glLightf (GL_LIGHT0+newtarg, GL_QUADRATIC_ATTENUATION, (*_llights_loc)[light].attenuate[2]); //for the most part ignored by OpenGL, so not a big cost...I hope :-D
      olddat->target=-1;
      (*_llights_dat)[light].target=newtarg;
      (*_llights_dat)[light].changed=0;
      GLLights[newtarg]=light;//set pointer and backpointer
      GLLightState[newtarg]=1;
      glEnable (GL_LIGHT0+newtarg);
      return;
    }
  }
}

static const float ZeroTmp[4]={0,0,0,1};

void SubrEnableAttenuated (unsigned int, float, unsigned int);//does the replacing 
void ForceEnableAttenuated (unsigned int light, float AttenFactor, unsigned int &tmp, unsigned int &newtarg) {


  for (;tmp<GFX_MAX_LIGHTS;tmp++) {
    if (GLLightState[tmp]==0) {
      if (GLLights[tmp]==-1) {
	SubrEnableAttenuated (light,AttenFactor, tmp);
	return;
      }
    }
  }
  for (;tmp<GFX_MAX_LIGHTS;tmp++) {
    if (GLLightState[tmp]==0) {
      SubrEnableAttenuated (light,AttenFactor, tmp);
      return;      
    }
  }
}

void SubrEnableAttenuated (unsigned int light, float AttenFactor, unsigned int tmp) {
  //copy all state nfo
  int opt = (*_llights_dat)[light].options;
  if (opt&DIFFUSE) {
    AttTmp[0]=(*_llights_dat)[light].diffuse[0]*AttenFactor;
    AttTmp[1]=(*_llights_dat)[light].diffuse[1]*AttenFactor;
    AttTmp[2]=(*_llights_dat)[light].diffuse[2]*AttenFactor;
    AttTmp[3]=(*_llights_dat)[light].diffuse[3];//don't attenuate the alpha :-D
    glLightfv (GL_LIGHT0+tmp,GL_DIFFUSE, AttTmp);
  } else 
    glLightfv (GL_LIGHT0+tmp,GL_DIFFUSE, ZeroTmp);
  if (opt&SPECULAR) {
    AttTmp[0]=(*_llights_dat)[light].specular[0]*AttenFactor;
    AttTmp[1]=(*_llights_dat)[light].specular[1]*AttenFactor;
    AttTmp[2]=(*_llights_dat)[light].specular[2]*AttenFactor;
    AttTmp[3]=(*_llights_dat)[light].specular[3];//don't attenuate the alpha :-D
    glLightfv (GL_LIGHT0+tmp, GL_SPECULAR, AttTmp);
  } else {
    glLightfv (GL_LIGHT0+tmp, GL_SPECULAR, ZeroTmp);
  }
  if (opt&AMBIENT) {
    AttTmp[0]=(*_llights_dat)[light].specular[0]*AttenFactor;
    AttTmp[1]=(*_llights_dat)[light].specular[1]*AttenFactor;
    AttTmp[2]=(*_llights_dat)[light].specular[2]*AttenFactor;
    AttTmp[3]=(*_llights_dat)[light].specular[3];//don't attenuate the alpha :-D
    glLightfv (GL_LIGHT0+tmp, GL_AMBIENT, (*_llights_dat)[light].ambient);
  } else {
    glLightfv (GL_LIGHT0+tmp, GL_AMBIENT, ZeroTmp);
  }
  //POS//(*_llights_loc)[light].vect[3]=1;
  //POS//glLightfv (GL_LIGHT0+tmp, GL_POSITION, (*_llights_loc)[light].vect);
  //POS//(*_llights_loc)[light].vect[3]=1;
  glLightfv (GL_LIGHT0+tmp, GL_SPOT_DIRECTION, (*_llights_dat)[light].spot);
  glLightf (GL_LIGHT0+tmp,GL_SPOT_EXPONENT, (*_llights_dat)[light].exp);
  ////glLightf (GL_LIGHT0+tmp, GL_CONSTANT_ATTENUATION, (*_llights_loc)[light].attenuate[0]);
  ////glLightf (GL_LIGHT0+tmp, GL_LINEAR_ATTENUATION, (*_llights_loc)[light].attenuate[1]);
  ////glLightf (GL_LIGHT0+tmp, GL_QUADRATIC_ATTENUATION, (*_llights_loc)[light].attenuate[2]);//they appear to be broken...anyhow they are of great cost...possibly better to just do a per-model attenuation
  (*_llights_dat)[light].target=tmp;
  (*_llights_dat)[light].changed=0;
  GLLights[tmp]=light;//set pointer and backpointer
  GLLightState[tmp]=1;
  glEnable (GL_LIGHT0+tmp);
  tmp++;
} 




void EnableExisting (unsigned int light) {
  int chg = (*_llights_dat)[light].changed;
  int gl_targ = (*_llights_dat)[light].target;
  if (chg) {
    if (chg&DIFFUSE) {
      glLightfv (GL_LIGHT0+gl_targ,GL_DIFFUSE, (*_llights_dat)[light].diffuse);
    }
    if (chg&SPECULAR) {
      glLightfv (GL_LIGHT0+gl_targ, GL_SPECULAR, (*_llights_dat)[light].specular);
    }
    if (chg&AMBIENT) {
      glLightfv (GL_LIGHT0+gl_targ, GL_AMBIENT, (*_llights_dat)[light].ambient);
    }
    //POS//    if (chg&POSITION) {
    //POS//      glLightfv (GL_LIGHT0+gl_targ, GL_POSITION, (*_llights_loc)[light].vect);
    //POS//    }
    if (chg&DIRECTION) {
      glLightfv (GL_LIGHT0+gl_targ, GL_SPOT_DIRECTION, (*_llights_dat)[light].spot);
      glLightf 	(GL_LIGHT0+gl_targ, GL_SPOT_EXPONENT,  (*_llights_dat)[light].exp);
    }
    //    if (chg&ATTENUATE) {
    //      glLightf (GL_LIGHT0+gl_targ, GL_CONSTANT_ATTENUATION, (*_llights_loc)[light].attenuate[0]);
    //      glLightf (GL_LIGHT0+gl_targ, GL_LINEAR_ATTENUATION, (*_llights_loc)[light].attenuate[1]);
    //      glLightf (GL_LIGHT0+gl_targ, GL_QUADRATIC_ATTENUATION, (*_llights_loc)[light].attenuate[2]);
    //}
    glEnable (GL_LIGHT0+gl_targ);
    GLLightState[gl_targ]=2;
    (*_llights_dat)[light].changed=0;
    return;
  }
  if (GLLightState[gl_targ]==0) {
    glEnable (GL_LIGHT0+gl_targ);//if nothing changes not a whole lot to do  dont' enable if on
  }
  GLLightState[gl_targ]=2;//don't enable then disable
}

void EnableExistingAttenuated (unsigned int light, float AttenFactor) {
  int opt = (*_llights_dat)[light].options;
  int chg = (*_llights_dat)[light].changed;
  int gl_targ = (*_llights_dat)[light].target;
  if (opt&DIFFUSE) {
    AttTmp[0]=(*_llights_dat)[light].diffuse[0]*AttenFactor;
    AttTmp[1]=(*_llights_dat)[light].diffuse[1]*AttenFactor;
    AttTmp[2]=(*_llights_dat)[light].diffuse[2]*AttenFactor;
    AttTmp[3]=(*_llights_dat)[light].diffuse[3];//don't attenuate the alpha :-D
    glLightfv (GL_LIGHT0+gl_targ,GL_DIFFUSE, AttTmp);
  } else {
    if (chg&DIFFUSE) 
      glLightfv (GL_LIGHT0+gl_targ,GL_DIFFUSE, ZeroTmp);
  }	
  if (opt&SPECULAR) {
    AttTmp[0]=(*_llights_dat)[light].specular[0]*AttenFactor;
    AttTmp[1]=(*_llights_dat)[light].specular[1]*AttenFactor;
    AttTmp[2]=(*_llights_dat)[light].specular[2]*AttenFactor;
    AttTmp[3]=(*_llights_dat)[light].specular[3];//don't attenuate the alpha :-D
    glLightfv (GL_LIGHT0+gl_targ, GL_SPECULAR, AttTmp);
  } else {
    if (chg&SPECULAR)
    glLightfv (GL_LIGHT0+gl_targ, GL_SPECULAR, ZeroTmp);
  }
  if (opt&AMBIENT) {
    AttTmp[0]=(*_llights_dat)[light].specular[0]*AttenFactor;
    AttTmp[1]=(*_llights_dat)[light].specular[1]*AttenFactor;
    AttTmp[2]=(*_llights_dat)[light].specular[2]*AttenFactor;
    AttTmp[3]=(*_llights_dat)[light].specular[3];//don't attenuate the alpha :-D
    glLightfv (GL_LIGHT0+gl_targ, GL_AMBIENT, (*_llights_dat)[light].ambient);
  } else {
    if (chg&AMBIENT)
      glLightfv (GL_LIGHT0+gl_targ, GL_AMBIENT, ZeroTmp);
  }
  //POS//  if (chg&POSITION) {
  //POS//    (*_llights_loc)[light].vect[3]=1;
  //POS//  glLightfv (GL_LIGHT0+gl_targ, GL_POSITION, (*_llights_loc)[light].vect);
  //POS//  (*_llights_loc)[light].vect[3]=1;
  
  if (chg&DIRECTION) {
    glLightfv (GL_LIGHT0+gl_targ, GL_SPOT_DIRECTION, (*_llights_dat)[light].spot);
    glLightf 	(GL_LIGHT0+gl_targ, GL_SPOT_EXPONENT,  (*_llights_dat)[light].exp);
  }
    //    if (chg&ATTENUATE) {
    //      glLightf (GL_LIGHT0+gl_targ, GL_CONSTANT_ATTENUATION, (*_llights_loc)[light].attenuate[0]);
    //      glLightf (GL_LIGHT0+gl_targ, GL_LINEAR_ATTENUATION, (*_llights_loc)[light].attenuate[1]);
    //      glLightf (GL_LIGHT0+gl_targ, GL_QUADRATIC_ATTENUATION, (*_llights_loc)[light].attenuate[2]);
    //    }
  glEnable (GL_LIGHT0+gl_targ);
  GLLightState[gl_targ]=2;
  (*_llights_dat)[light].changed=0;
  return;
}

//minor differences between d3d and gl direction and position, will be dealt with if need be

BOOL /*GFXDRVAPI*/ GFXSetLightProperties(gfx_light_data &curlight,gfx_light_loc &curlightloc, enum LIGHT_TARGET lighttarg, const GFXColor &color) {
  curlight.changed |=lighttarg;
  if (color.r==color.g&&color.g==color.b&&color.b==0) {
    curlight.options &= (~lighttarg);
  } else {
    curlight.options |= (lighttarg);
  }
  switch (lighttarg) {
  case DIFFUSE:
    curlight.diffuse[0]=color.r;
    curlight.diffuse[1]=color.g;
    curlight.diffuse[2]=color.b;
    curlight.diffuse[3]=color.a;
    break;
  case SPECULAR:
    curlight.specular[0]=color.r;
    curlight.specular[1]=color.g;
    curlight.specular[2]=color.b;
    curlight.specular[3]=color.a;
    break;
  case AMBIENT:
    curlight.ambient[0]=color.r;
    curlight.ambient[1]=color.g;
    curlight.ambient[2]=color.b;
    curlight.ambient[3]=color.a;
    break;
  case POSITION:
    curlightloc.vect[0]=color.r;
    curlightloc.vect[1]=color.g;
    curlightloc.vect[2]=color.b;
    curlightloc.vect[3]=0; //assume set attenuate AFTER position set
    //////????    curlight.options&=(~(POSITION));
  case DIRECTION:
    curlight.spot[0]=color.r;
    curlight.spot[1]=color.g;
    curlight.spot[2]=color.b;
    curlight.exp = color.a;///exponent!!!!!!
    /////??????    curlight.options&=(~(DIRECTION));//are position and direction mutually exclusive?
    break;
  case ATTENUATE:
    curlightloc.attenuate[0]=color.r;
    curlightloc.attenuate[1]=color.g;
    curlightloc.attenuate[2]=color.b;
    curlightloc.vect[3]=1;
    break;
  }
  if (curlight.target==-2) {
    curlightloc.intensity=-1;
    return FALSE;
  }else {
    if (lighttarg==AMBIENT||lighttarg==SPECULAR||lighttarg==DIFFUSE) {
      curlightloc.intensity=(curlight.specular[0]+curlight.specular[1]+curlight.specular[2]+curlight.diffuse[0]+curlight.diffuse[1]+curlight.diffuse[2]+curlight.ambient[0]+curlight.ambient[1]+curlight.ambient[2])*.333333333333333333333333;//calculate new cuttof val for light 'caching'
      if (!curlight.enabled)
	curlightloc.intensity=-curlightloc.intensity;
    }
  }
  return TRUE;
}

BOOL /*GFXDRVAPI*/ GFXSetLight(int light, enum LIGHT_TARGET lightarg, const GFXColor & color) {
  return GFXSetLightProperties ((*_llights_dat)[light],(*_llights_loc)[light], lightarg ,color);
}
BOOL /*GFXDRVAPI*/ GFXSetPower (int light, float power) {
  if (light < _llights_dat->size()&&light >=0) {
    (*_llights_dat)[light].exp = power;
  } else
    return FALSE;
  return TRUE;
}

