#include "gl_light.h"
#include "hashtable_3d.h"
#include "cmd_collide.h"


//table to store local lights, numerical pointers to _llights (eg indices)
Hashtable3d <LineCollide*, char[20],char[200]> lighttable;

GFXLight gfx_light::operator = (const GFXLight &tmp) {
    memcpy (this,&tmp,sizeof (GFXLight));
    return tmp;
}

int gfx_light::lightNum() {
  int tmp =  (this-_llights->begin());
  if (tmp<0||tmp>(int)_llights->size())
    return -1;
  return tmp;
}//which number it is in the main scheme of things


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
  int clobberlocal=-1;
  for (int i=0;i<GFX_MAX_LIGHTS;i++) {
      if (GLLights[i].index==-1) {
	  return i;
      }
      if (GLLights[i].options&OpenGLLights::GLL_LOCAL) {
	  clobberlocal = i;
      }
      if (!(GLLights[i].options&OpenGLLights::GLL_ON)) {
	  if (clobberlocal==i||clobberdisabled==-1) {
	      clobberdisabled = i;
	  }
      }
  }
  return (clobberdisabled==-1)?clobberlocal:clobberdisabled;
}
bool gfx_light::Create (const GFXLight & temp, bool global) {
    int foundclobberable=0;
    *this = temp;
    if (!global) {
	options |=GFX_LOCAL_LIGHT;
    } else {
	options &=(~GFX_LOCAL_LIGHT);
	foundclobberable = findGlobalClobberable ();
	if (foundclobberable!=-1) {
	    ClobberGLLight (foundclobberable);
	}
    }
    return foundclobberable!=-1;
}
void gfx_light::Kill() {
    Disable();//first disables it...which _will_ remove it from the light table.
    if (target!=-1) {
      TrashFromGLLights();//then if not already done, trash from GLlights;
    }
    target=-2;
    options = 0;
}

/** ClobberGLLight ****
 ** most OpenGL implementation dirty lighting if any info is changed at all
 ** having two different lights with the same stats and pos is unlikely at best
 */

void gfx_light::SendGLPosition (const GLenum target) {
  int tmp = options;
  w = (float)(attenuated()!=0);
  glLightfv (target,GL_POSITION,vect);
  options = tmp;
}
void gfx_light::ClobberGLLight (const int target) {
  GLenum gltarg = GL_LIGHT0+target;
  if (attenuated()) {
    glLightf (gltarg,GL_CONSTANT_ATTENUATION,attenuate[0]);
    glLightf (gltarg,GL_LINEAR_ATTENUATION, attenuate[1]);
    glLightf (gltarg,GL_QUADRATIC_ATTENUATION,attenuate[2]);
  }
  SendGLPosition (gltarg);
  glLightfv (gltarg,GL_DIFFUSE, diffuse);
  glLightfv (gltarg,GL_SPECULAR, specular);
  glLightfv (gltarg,GL_AMBIENT, ambient);
}


void gfx_light::ResetProperties (const enum LIGHT_TARGET light_targ, const GFXColor &color) {
  if (LocalLight()) {
    RemoveFromTable();
    TrashFromGLLights();
  }
  switch (light_targ) {
  case DIFFUSE:
    diffuse[0]= color.r;diffuse[1]=color.g;diffuse[2]=color.b;diffuse[3]=color.a;
    if (target<0)
      break;
    glLightfv (GL_LIGHT0+target,GL_DIFFUSE,diffuse);
    break;
  case SPECULAR:
    specular[0]= color.r;specular[1]=color.g;specular[2]=color.b;specular[3]=color.a;    
    if (target<0)
      break;
    glLightfv (GL_LIGHT0+target,GL_SPECULAR,specular);
    break;
  case AMBIENT:
    ambient[0]= color.r;ambient[1]=color.g;ambient[2]=color.b;ambient[3]=color.a;    
    if (target<0)
      break;
    glLightfv (GL_LIGHT0+target,GL_AMBIENT,ambient);    
    break;
  case POSITION:
    vect[0]=color.r;vect[1]=color.g;vect[2]=color.b;
    if (target<0)
      break;
    glLightfv (GL_LIGHT0+target,GL_POSITION,vect);
    break;
  case ATTENUATE:
    attenuate[0]=color.r; attenuate[1]=color.g; attenuate[2]=color.b;
    apply_attenuate(attenuated());
    if (target<0)
      break;
    SendGLPosition (GL_LIGHT0+target);
    glLightf (GL_LIGHT0+target,GL_CONSTANT_ATTENUATION, attenuate[0]);
    glLightf (GL_LIGHT0+target,GL_LINEAR_ATTENUATION, attenuate[1]);
    glLightf (GL_LIGHT0+target,GL_QUADRATIC_ATTENUATION, attenuate[2]);
    break;
  }
}

void gfx_light::TrashFromGLLights () {
  assert (target>0);
  assert ((GLLights[target].options&OpenGLLights::GLL_ON)==0);//better be disabled so we know it's not in the table, etc
  assert ((&(*_llights)[GLLights[target].index])==this);
  GLLights[target].index = -1;
  GLLights[target].options= OpenGLLights::GLL_LOCAL;
  
}

void gfx_light::RemoveFromTable() {

}


//unimplemented
void gfx_light::Enable() {

}
//unimplemented
void gfx_light::Disable() {

}

LineCollide gfx_light::CalculateBounds () {

}







