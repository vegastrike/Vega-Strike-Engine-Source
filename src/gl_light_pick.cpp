#include "gl_light.h"
#include <queue>
using std::priority_queue;
#include "hashtable_3d.h"
#include "cmd_collide.h"

//optimization globals
float intensity_cutoff=.05;//something that would normally round down
float optintense=.2;
float optsat = .95;


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

static priority_queue<light_key> lightQ;


void GFXPickLights (const Vector & center) {

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


