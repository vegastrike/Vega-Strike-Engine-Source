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

#include "gfxlib.h"
#include "hashtable_3d.h"
#include "gl_light.h"
#include <queue>
using std::priority_queue;

static int _currentContext=0;
static vector <vector <gfx_light> > _local_lights_dat;
static vector <GFXColor> _ambient_light;
static vector <gfx_light> * _llights=NULL;

//currently stored GL lights!
static OpenGLLights* GLLights=NULL;//{-1,-1,-1,-1,-1,-1,-1,-1};

//table to store local lights, numerical pointers to _llights (eg indices)
static Hashtable3d <LineCollide*, char[20],char[200]> lighttable;

//optimization globals
static float intensity_cutoff=.05;//something that would normally round down
static float optintense=.2;
static float optsat = .95;


GFXLight::GFXLight (const bool enabled,const GFXColor &vect, const GFXColor &diffuse, const GFXColor &specular, const GFXColor &ambient, const GFXColor &attenuate) {
  target = -1;
  options = 0;
  memcpy (this->vect,&vect,sizeof(float)*3);
  if (diffuse.r||diffuse.g||diffuse.g||(diffuse.a!=1)) {
    options |=GFX_DIFFUSE;
    memcpy (this->diffuse,&diffuse,sizeof(float)*4);
  }
  if (specular.r||specular.g||specular.b||(specular.a!=1)) {
    options |=GFX_SPECULAR;
    memcpy (this->specular,&specular,sizeof(float)*4);
  }
  if (ambient.r||ambient.g||ambient.b||(ambient.a!=1)) {
    options |= GFX_AMBIENT;
    memcpy (this->ambient,&ambient,sizeof(float)*4);
  }
  if ((attenuate.r!=1)||attenuate.g||attenuate.b) {
    options |= GFX_ATTENUATED;
    memcpy (this->attenuate,&attenuate,sizeof(float)*3);
    this->vect[3]=1;
  }
  changed = options;
  if (enabled)
    this->enable();
  else
    this->disable();
}
void GFXLight::disable () {options &= (~GFX_LIGHT_ENABLED);}
void GFXLight::enable (){options |= GFX_LIGHT_ENABLED;}

void /*GFXDRVAPI*/ GFXLight::SetProperties(enum LIGHT_TARGET lighttarg, const GFXColor &color) {
  changed |=lighttarg;
  if (lighttarg==ATTENUATE) {
    if (color.r==1&color.g==color.b&&color.b==0) {
      options &=(~lighttarg);
    }else {
      target |= (lighttarg);
    }
  }else {
    if (color.r==color.g&&color.g==color.b&&color.b==0&&color.a==1) {
      options &= (~lighttarg);
    } else {
      options |= (lighttarg);
    }
  }
  if (options&GFX_ATTENUATED) {
    vect[3]=1;
  } else {
    vect[3]=0;
  }
  switch (lighttarg) {
  case DIFFUSE:
    diffuse[0]=color.r;
    diffuse[1]=color.g;
    diffuse[2]=color.b;
    diffuse[3]=color.a;
    break;
  case SPECULAR:
    specular[0]=color.r;
    specular[1]=color.g;
    specular[2]=color.b;
    specular[3]=color.a;
    break;
  case AMBIENT:
    ambient[0]=color.r;
    ambient[1]=color.g;
    ambient[2]=color.b;
    ambient[3]=color.a;
    break;
  case POSITION:
    vect[0]=color.r;
    vect[1]=color.g;
    vect[2]=color.b;
    break;
  case ATTENUATE:
    attenuate[0]=color.r;
    attenuate[1]=color.g;
    attenuate[2]=color.b;
    break;
  }
}

int gfx_light::lightNum() {
  int tmp =  (this-_llights->begin());
  if (tmp<0||tmp>(int)_llights->size())
    return -1;
  return tmp;
}//which number it is in the main scheme of things


void GFXDestroyAllLights () {
  lighttable.Clear();
  if (GLLights)
    free (GLLights);
}
GFXBOOL /*GFXDRVAPI*/ GFXSetCutoff (float ttcutoff) {
  if (ttcutoff<0) 
    return GFXFALSE;
  intensity_cutoff=ttcutoff;
  return GFXTRUE;
}
void /*GFXDRVAPI*/ GFXSetOptimalIntensity (float intensity, float saturate) {
  optintense = intensity;
  optsat = saturate;
}
GFXBOOL /*GFXDRVAPI*/ GFXSetOptimalNumLights (int numLights) {
  if (numLights>GFX_MAX_LIGHTS||numLights<0)
    return GFXFALSE;
  GFX_OPTIMAL_LIGHTS=numLights;
  return GFXTRUE;
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

priority_queue<light_key> lightQ;


GFXBOOL /*GFXDRVAPI*/ GFXSetSeparateSpecularColor(GFXBOOL spec) {
#ifndef WIN32
	if (spec) {
    glLightModeli (GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
  }else {
    glLightModeli (GL_LIGHT_MODEL_COLOR_CONTROL,GL_SINGLE_COLOR);
    return GFXFALSE;
  }
#else
	return GFXFALSE;
#endif
	return GFXTRUE;
}



void /*GFXDRVAPI*/ GFXCreateLightContext (int & con_number) {
  int i;
  static GFXBOOL LightInit=GFXFALSE;
  if (!LightInit) {
    LightInit = GFXTRUE;
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);//don't want lighting coming from infinity....we have to take the hit due to sphere mapping matrix tweaking
    //
    glGetIntegerv(GL_MAX_LIGHTS,&GFX_MAX_LIGHTS);
    if (!GLLights) {
      GLLights= (OpenGLLights *)malloc (sizeof(OpenGLLights)*GFX_MAX_LIGHTS);
      for (i=0;i<GFX_MAX_LIGHTS;i++) {
	GLLights[i].index=-1;
      }
    }
  }
  con_number = _local_lights_dat.size();
  _currentContext= con_number;
  _ambient_light.push_back (GFXColor (0,0,0,1));
  _local_lights_dat.push_back (vector <gfx_light>());
  GFXSetLightContext (con_number);
}

void /*GFXDRVAPI*/ GFXDeleteLightContext(int con_number) {
  _local_lights_dat[con_number]=vector <gfx_light>();
}

void /*GFXDRVAPI*/ GFXSetLightContext (int con_number) {
  _currentContext = con_number;
  _llights = &_local_lights_dat[con_number];
  float tmp[4]={_ambient_light[con_number].r,_ambient_light[con_number].g,_ambient_light[con_number].b,_ambient_light[con_number].a};
  glLightModelfv (GL_LIGHT_MODEL_AMBIENT,tmp);
  unsigned int i;
  for (i=0;i<_llights->size();i++) {
    (*_llights)[i].Target()=-1;
  }
  int GLLindex=0;
  for (int i=0;i<_llights->size()&&GLLindex<GFX_MAX_LIGHTS;i++) {
    if ((*_llights)[i].enabled()&&(!(*_llights)[i].LocalLight())) {
      GLLights[GLLindex].index=-1;//make it clobber completley! no trace of old light.
      (*_llights)[i].ClobberGLLight (GLLindex);
      GLLindex++;
    }
  }
  for (;GLLindex<GFX_MAX_LIGHTS;GLLindex++) {
    GLLights[GLLindex].index=-1;
    GLLights[GLLindex].options=OpenGLLights::GLL_OFF;
    glDisable (GL_LIGHT0+GLLindex);
  }
}

GFXBOOL /*GFXDRVAPI*/ GFXLightContextAmbient (const GFXColor &amb) {
  if (_currentContext >=_ambient_light.size())
    return GFXFALSE;
  (_ambient_light[_currentContext])=amb;
  //  (_ambient_light[_currentContext])[1]=amb.g;
  //  (_ambient_light[_currentContext])[2]=amb.b;
  //  (_ambient_light[_currentContext])[3]=amb.a;
  float tmp[4]={amb.r,amb.g,amb.b,amb.a};
  glLightModelfv (GL_LIGHT_MODEL_AMBIENT,tmp);
  return GFXTRUE;
}

GFXBOOL /*GFXDRVAPI*/ GFXCreateLight (int &light, const GFXLight &templatecopy, const bool global) {
  for (light=0;light<_llights->size();light++) {
    if ((*_llights)[light].Target()==-2)
      break;
  }
  if (light==_llights->size()) {	
    _llights->push_back (gfx_light());
  }
  
  (*_llights)[light].Create(templatecopy,global);
}

void /*GFXDRVAPI*/ GFXDeleteLight (int light) {
  (*_llights)[light].Kill();
}

static int findLocalClobberable() {
  int clobberdisabled =-1;
  for (int i=0;i<GFX_MAX_LIGHTS;i++) {
    if (GLLights[i].index==-1) 
      return i;
    if (!(GLLights[i].options&OpenGLLights::GLL_ON)) {
      clobberdisabled = i;
    }
  }
  return clobberdisabled;
}
static int findGlobalClobberable () {//searches through the GLlights and sees which one is clobberable.  Returns -1 if not.
  int clobberdisabled =-1;
  if ((clobberdisabled=findLocalClobberable())!=-1) {
    return clobberdisabled;
  }
  for (int i=0;i<GFX_MAX_LIGHTS;i++) {
    if (GLLights[i].options&OpenGLLights::GLL_LOCAL) {
      return i;
    }
  }
  return -1;
}


GFXBOOL /*GFXDRVAPI*/ GFXEnableLight (int light) {
  //  assert (_llights);
  //  if (light<0||light>=_llights->size()) 
  //    return FALSE;
  if ((*_llights)[light].Target()==-2) {
    return GFXFALSE;
  }
  (*_llights)[light].Enable();
  return GFXTRUE;
}

GFXBOOL /*GFXDRVAPI*/ GFXDisableLight (int light) {
  //  assert (_llights);
  //  if (light<0||light>=_llights->size()) 
  //    return FALSE;
  if ((*_llights)[light].Target()==-2) {
    return GFXFALSE;
  }
  (*_llights)[light].Disable();
  return GFXTRUE;
}

GFXBOOL /*GFXDRVAPI*/ GFXSetLight(int light, enum LIGHT_TARGET lt, const GFXColor &color) {
  if ((*_llights)[light].Target()==-2) {
    return GFXFALSE;
  }
  (*_llights)[light].ResetProperties(lt,color);
  
  return GFXTRUE;
}

void gfx_light::Create (const GFXLight & temp, bool global) {

}
void gfx_light::Kill() {

}
void gfx_light::ClobberGLLight (const int target) {

}
void gfx_light::ContextSwitchEnableLight (const int target) {

}
void gfx_light::Enable() {

}

void gfx_light::Disable() {

}
void gfx_light::ResetProperties (const enum LIGHT_TARGET, const GFXColor &color) {

}
LineCollide gfx_light::CalculateBounds () {

}
void GFXPickLights (const float *) {

}
#ifdef MIRACLESAVESDAY
inline void SetLocalCompare (Vector x) {//does preprocessing of intensity for relatively small source 
  float dis, dissqr;
  unsigned int i;
  for (i=0;i<_llights->size();i++) {
    if ((*_llights)[i].intensity>0) {
      if ((*_llights)[i].vect[3]!=0) {
	dissqr = 
	  ((*_llights)[i].vect[0]-x.i)*((*_llights)[i].vect[0]-x.i)+
	  ((*_llights)[i].vect[1]-x.j)*((*_llights)[i].vect[1]-x.j)+
	  ((*_llights)[i].vect[2]-x.k)*((*_llights)[i].vect[2]-x.k);
	dis = sqrtf (dissqr);//should use fast sqrt by nvidia
	
	float intensity=(*_llights)[i].intensity/(
						    (*_llights)[i].attenuate[0] +
						    (*_llights)[i].attenuate[1]*dis +
						    (*_llights)[i].attenuate[2]*dissqr);
			       
	if (intensity>intensity_cutoff) {
	  lightQ.push (light_key(i,intensity));
	}	
      }else {
	if ((*_llights)[i].intensity>intensity_cutoff)
	  lightQ.push (light_key(i,(*_llights)[i].intensity));
      }
    }
  }
}




void /*GFXDRVAPI*/ GFXPickLights (const float * transform) {
  //picks 1-5 lights to use
  // glMatrixMode(GL_MODELVIEW);
  //glPushMatrix();
  //float tm [16]={1,0,0,1000,0,1,0,1000,0,0,1,1000,0,0,0,1};
  //glLoadIdentity();
  //  GFXLoadIdentity(MODEL);
  if (!GFXLIGHTING) return ;
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
#ifdef PER_MESH_ATTENUATE
      if ((*_llights)[i].vect[3]) {
	//intensity now has become the attenuation factor
	newQ[i]= tmpvar.number;
	AttenuateQ[i]=tmpvar.intensity_key/(*_llights)[i].intensity;
      }else {
#endif
	newQ[i]= tmpvar.number;
#ifdef PER_MESH_ATTENUATE
	AttenuateQ[i]=0;
      }
#endif
    } else {
      break;
    }
    lightQ.pop();
  }
  unsigned int light;
  for (i=0;i<newQsize;i++) {
    light = newQ[i];
    if ((*_llights)[light].target>=0) {
#ifdef PER_MESH_ATTENUATE
      if (AttenuateQ[i]) 
	EnableExistingAttenuated (light,AttenuateQ[i]);
      else
#endif
	EnableExisting (newQ[i]);
      /***daniel 081901
      AttTmp[0]=(*_llights)[light].vect[0]-loc.i;
      AttTmp[1]=(*_llights)[light].vect[1]-loc.j;
      AttTmp[2]=(*_llights)[light].vect[2]-loc.k;
      VecT[3]=0;
      VecT[0]=AttTmp[0]*transform[0]+AttTmp[1]*transform[1]+AttTmp[2]*transform[2];
      VecT[1]=AttTmp[0]*transform[4]+AttTmp[1]*transform[5]+AttTmp[2]*transform[6];
      VecT[2]=AttTmp[0]*transform[8]+AttTmp[1]*transform[9]+AttTmp[2]*transform[10];
      
      glLightfv (GL_LIGHT0+(*_llights)[light].target, GL_POSITION, VecT);
      ****/
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
    if ((*_llights)[light].target==-1) {
      if (AttenuateQ[i]) 
	ForceEnableAttenuated (light,AttenuateQ[i],tmp,newtarg);
      else
	ForceEnable (light,tmp,newtarg); 
      AttTmp[0]=(*_llights)[light].vect[0]-loc.i;
      AttTmp[1]=(*_llights)[light].vect[1]-loc.j;
      AttTmp[2]=(*_llights)[light].vect[2]-loc.k;
      AttTmp[3]=0;
      VecT[0]=AttTmp[0]*transform[0]+AttTmp[1]*transform[1]+AttTmp[2]*transform[2];
      VecT[1]=AttTmp[0]*transform[4]+AttTmp[1]*transform[5]+AttTmp[2]*transform[6];
      VecT[2]=AttTmp[0]*transform[8]+AttTmp[1]*transform[9]+AttTmp[2]*transform[10];
      glLightfv (GL_LIGHT0+(*_llights)[light].target, GL_POSITION, VecT);

      //      glLightfv (GL_LIGHT0+(*llight_dat)[newQ[i]].target, GL_POSITION, /*wrong*/(*_llights)[light].vect);
    }
  }
  //glPopMatrix();
}

/*
      curlightloc.intensity=(curlight.specular[0]+curlight.specular[1]+curlight.specular[2]+curlight.diffuse[0]+curlight.diffuse[1]+curlight.diffuse[2]+curlight.ambient[0]+curlight.ambient[1]+curlight.ambient[2])*.333333333333333333333333;//calculate new cuttof val for light 'caching'
*/
#endif
