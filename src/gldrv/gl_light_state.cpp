#include <assert.h>
//#include <vegastrike.h>
#include "gl_globals.h"
#include "hashtable_3d.h"
#include "gl_light.h"
#define GFX_HARDWARE_LIGHTING
//table to store local lights, numerical pointers to _llights (eg indices)
const float atten0scale = 1;
const float atten1scale = 1./GFX_SCALE;
const float atten2scale = 1./(GFX_SCALE*GFX_SCALE);

int _GLLightsEnabled=0;
Hashtable3d <LineCollideStar, char[20],char[CTACC], char [lighthuge]> lighttable;

GFXLight gfx_light::operator = (const GFXLight &tmp) {
    memcpy (this,&tmp,sizeof (GFXLight));
    return tmp;
}

int gfx_light::lightNum() {
  int tmp =  (this-&_llights->front());
  assert (tmp>=0&&tmp<(int)_llights->size());
  //  assert (&(*_llights)[GLLights[target].index]==this);
  return tmp;
}//which number it is in the main scheme of things


int findLocalClobberable() {
  int clobberdisabled =-1;
  for (int i=0;i<GFX_MAX_LIGHTS;i++) {
    if (GLLights[i].index==-1) 
      return i;
    if (!(GLLights[i].options&OpenGLL::GLL_ON)) {
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
      if (GLLights[i].options&OpenGLL::GLL_LOCAL) {
	  clobberlocal = i;
      }
      if (!(GLLights[i].options&OpenGLL::GLL_ON)) {
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
	if (enabled()) {
	    disable();
	    this->Enable();//upon creation need to call enable script with disabled light
	}
    } else {
	options &=(~GFX_LOCAL_LIGHT);
	foundclobberable = enabled()?findGlobalClobberable ():findLocalClobberable();
	if (foundclobberable!=-1) {
	  _GLLightsEnabled+= (enabled()!=0);
	  ClobberGLLight (foundclobberable);
	}
    }
    return (foundclobberable!=-1)||(!enabled());
}
void gfx_light::Kill() {
    Disable();//first disables it...which _will_ remove it from the light table.
    if (target>=0) {
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
  float v[4] = {vect[0],vect[1],vect[2],1};
  glLightfv (target,GL_POSITION,v);
}  

inline void gfx_light::ContextSwitchClobberLight (const GLenum gltarg, const int original) {
  

  glLightf (gltarg,GL_CONSTANT_ATTENUATION,attenuate[0]*atten0scale);
  glLightf (gltarg,GL_LINEAR_ATTENUATION, attenuate[1]*atten1scale);
  glLightf (gltarg,GL_QUADRATIC_ATTENUATION,attenuate[2]*atten2scale);

  SendGLPosition (gltarg);
  glLightfv (gltarg,GL_DIFFUSE, diffuse);
  glLightfv (gltarg,GL_SPECULAR, specular);
  glLightfv (gltarg,GL_AMBIENT, ambient);
  if (original!=-1) {
    gfx_light * orig = &((*_llights)[GLLights[original].index]);
    orig->target = -1;
    GLLights[original].index=-1;
  }
}

inline void gfx_light::FinesseClobberLight (const GLenum gltarg, const int original) {
  gfx_light * orig = &((*_llights)[GLLights[original].index]);
  if (attenuated()) {
    if (orig->attenuated()) {
	  if (orig->attenuate[0]!=attenuate[0])
	    glLightf (gltarg,GL_CONSTANT_ATTENUATION,attenuate[0]*atten0scale);
	  if  (orig->attenuate[1]!=attenuate[1])
	    glLightf (gltarg,GL_LINEAR_ATTENUATION, attenuate[1]*atten1scale);
	  if  (orig->attenuate[2]!=attenuate[2])	  
	    glLightf (gltarg,GL_QUADRATIC_ATTENUATION,attenuate[2]*atten2scale); 
      } else {
	  glLightf (gltarg,GL_CONSTANT_ATTENUATION,attenuate[0]*atten0scale);
	  glLightf (gltarg,GL_LINEAR_ATTENUATION, attenuate[1]*atten1scale);
	  glLightf (gltarg,GL_QUADRATIC_ATTENUATION,attenuate[2]*atten2scale);
      }
  }
  if (vect[0]!=orig->vect[0]||vect[1]!=orig->vect[1]||vect[2]!=orig->vect[2]||attenuated()!=orig->attenuated()) {
      SendGLPosition(gltarg);
  }
  if (diffuse[0]!=orig->diffuse[0]||diffuse[1]!=orig->diffuse[1]||diffuse[2]!=orig->diffuse[2]||diffuse[3]!=orig->diffuse[3]) {
      glLightfv (gltarg,GL_DIFFUSE,diffuse);
  }
  if (specular[0]!=orig->specular[0]||specular[1]!=orig->specular[1]||specular[2]!=orig->specular[2]||specular[3]!=orig->specular[3]) {
      glLightfv (gltarg,GL_SPECULAR,specular);
  }

  if (ambient[0]!=orig->ambient[0]||ambient[1]!=orig->ambient[1]||ambient[2]!=orig->ambient[2]||ambient[3]!=orig->ambient[3]) {
      glLightfv (gltarg, GL_AMBIENT,ambient);
  }
  orig->target = -1;
  GLLights[original].index=-1;
}

void gfx_light::ClobberGLLight (const int target) {
  this->target = target;
  if (enabled()!=((GLLights[target].options&OpenGLL::GL_ENABLED)!=0))  {
    if (enabled()) {
      glEnable (GL_LIGHT0+target);
      GLLights[target].options|=OpenGLL::GL_ENABLED;
    } else {
      GLLights[target].options&=(~OpenGLL::GL_ENABLED);
      glDisable (GL_LIGHT0+target);
    }
  }
  GLLights[target].options&=(OpenGLL::GL_ENABLED);//turn off options
#ifdef GFX_HARDWARE_LIGHTING
    if (GLLights[target].index==-1) {
#endif
      ContextSwitchClobberLight (GL_LIGHT0+target,GLLights[target].index);
#ifdef GFX_HARDWARE_LIGHTING
    }else {
	FinesseClobberLight (GL_LIGHT0+target,GLLights[target].index);
    }
#endif
    this->target = target;
    //VSFileSystem::Fprintf (stderr,"Target %d had light %d",target, GLLights[target].index);
    GLLights[target].index = lightNum();
    //VSFileSystem::Fprintf (stderr," Clobbered with %d\n",lightNum());
    GLLights[target].options |= OpenGLL::GLL_ON*enabled()+OpenGLL::GLL_LOCAL*LocalLight();
}


void gfx_light::ResetProperties (const enum LIGHT_TARGET light_targ, const GFXColor &color) {
  bool changed=false;
  if (LocalLight()) {
    GFXLight t;
    memcpy (&t,this,sizeof (GFXLight));
    t.SetProperties (light_targ,color);
    changed=RemoveFromTable(false,t);
    memcpy (this,&t,sizeof (GFXLight));
    if (changed)
      AddToTable ();
    if (target>=0)
      TrashFromGLLights();
    return;
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
    SendGLPosition (GL_LIGHT0+target);
    break;
  default:
  case ATTENUATE:
    attenuate[0]=color.r; attenuate[1]=color.g; attenuate[2]=color.b;
    apply_attenuate(attenuated());
    if (target<0)
      break;
    SendGLPosition (GL_LIGHT0+target);
    glLightf (GL_LIGHT0+target,GL_CONSTANT_ATTENUATION, attenuate[0]*atten0scale);
    glLightf (GL_LIGHT0+target,GL_LINEAR_ATTENUATION, attenuate[1]*atten1scale);
    glLightf (GL_LIGHT0+target,GL_QUADRATIC_ATTENUATION, attenuate[2]*atten2scale);
    break;
  }
}

void gfx_light::TrashFromGLLights () {
  assert (target>=0);
  assert ((GLLights[target].options&OpenGLL::GLL_ON)==0);//better be disabled so we know it's not in the table, etc
  assert ((&(*_llights)[GLLights[target].index])==this);
  GLLights[target].index = -1;
  GLLights[target].options= OpenGLL::GLL_LOCAL;
  target = -1;
}
void gfx_light::AddToTable() {
  LineCollideStar tmp;
  bool err;
  LineCollide * coltarg= new LineCollide (CalculateBounds(err));//leak??
  if (err)
    return;
  tmp.lc = coltarg;
  lighttable.Put (coltarg, tmp);
}
bool gfx_light::RemoveFromTable(bool shouldremove, const GFXLight &t) {
  LineCollideStar tmp;
  bool err;
  LineCollide coltarg (CalculateBounds(err));
  if (!shouldremove) {
    bool err2;
    LineCollide coltarg2 (CalculateBounds (err2));
    if (lighttable.hash_int (coltarg2.Mini.i)==lighttable.hash_int (coltarg.Mini.i)&&
	lighttable.hash_int (coltarg2.Mini.j)==lighttable.hash_int (coltarg.Mini.j)&&
	lighttable.hash_int (coltarg2.Mini.k)==lighttable.hash_int (coltarg.Mini.k)&&
	lighttable.hash_int (coltarg2.Maxi.i)==lighttable.hash_int (coltarg.Maxi.i)&&
	lighttable.hash_int (coltarg2.Maxi.j)==lighttable.hash_int (coltarg.Maxi.j)&&
	lighttable.hash_int (coltarg2.Maxi.k)==lighttable.hash_int (coltarg.Maxi.k)) {
      return false;
    }
  }
  if (err)
    return false;
  tmp.lc = &coltarg;
  if (lighttable.Remove ( &coltarg, tmp)) {
    if (tmp.lc)
      delete tmp.lc;
    else
      assert (tmp.lc);
  }
  return true;
}


//unimplemented
void gfx_light::Enable() {
  if (!enabled()) {
    if (LocalLight())
      AddToTable();
    else {
      if (target==-1) {
	int newtarg =  findGlobalClobberable();
	if (newtarg==-1)
	  return;
	_GLLightsEnabled++;
	ClobberGLLight (newtarg);
      }
      glEnable (GL_LIGHT0+this->target);
      GLLights[this->target].options|=OpenGLL::GL_ENABLED|OpenGLL::GLL_ON;
    }
    enable();
  }
}
//unimplemented
void gfx_light::Disable() {
  if (enabled()) {
    disable();
    if (target>=0) {
      if (GLLights[target].options&OpenGLL::GL_ENABLED) {
	_GLLightsEnabled--;
	glDisable (GL_LIGHT0+this->target);
      }
      GLLights[this->target].options&=(~(OpenGLL::GL_ENABLED|OpenGLL::GLL_ON));
    }
    if (LocalLight()&&enabled()) {
      RemoveFromTable();
      if (target>=0)
	TrashFromGLLights();
    }
  }
}


// i =  tot/(A+B*d+C*d*d)    Ai+Bi*d+Ci*d*d = tot   
// d= (-Bi + sqrtf (B*i*B*i - 4*Ci*(Ai-tot)))/ (2Ci)
// d= (-B + sqrtf (B*B + 4*C*(tot/i-A)))/ (2C)

LineCollide gfx_light::CalculateBounds (bool &error) {
  error = false;
  float tot_intensity = ((specular[0]+specular[1]+specular[2])*specular[3]+
			 (diffuse[0]+diffuse[1]+diffuse[2])*diffuse[3]+ 
			 (ambient[0]+ambient[1]+ambient[2])*ambient[3])*.33;
  //  double d = (-(double)attenuate[1]+sqrt((double)attenuate[1]*(double)attenuate[1]+4*(double)attenuate[2]*(((double)tot_intensity/(double)intensity_cutoff) - (double)attenuate[0])))/(2*(double)attenuate[2]);
  //simplistic calculation above causes floating point inaccuracies
  double ffastmathreallysucksd;
  double ffastmathreallysucksq;

  ffastmathreallysucksd = sqrt(tot_intensity/intensity_cutoff-ambient[0]);
  
  ffastmathreallysucksq = sqrt(attenuate[2]+attenuate[1]) ;
  //  VSFileSystem::Fprintf (stderr,"q%lf d%lf",ffastmathreallysucksq,ffastmathreallysucksd);
  if (ffastmathreallysucksq==0||ffastmathreallysucksd<=0)
    error=true;
  ffastmathreallysucksd /=ffastmathreallysucksq;

  QVector st (vect[0]-ffastmathreallysucksd,vect[1]-ffastmathreallysucksd,vect[2]-ffastmathreallysucksd);
  QVector end (vect[0]+ffastmathreallysucksd,vect[1]+ffastmathreallysucksd,vect[2]+ffastmathreallysucksd);
  LineCollide retval(NULL,LineCollide::UNIT,st,end);
  *((int *)(&retval.object)) = lightNum();//put in a lightNum
  return retval;
}
void light_rekey_frame() {
  unpicklights();//picks doubtless changed position
    for (int i=0;i<GFX_MAX_LIGHTS;i++) {
	if (GLLights[i].options & OpenGLL::GL_ENABLED) {
	    if (GLLights[i].index>=0) {
	      if ((*_llights)[GLLights[i].index].Target() == i) {
		(*_llights)[GLLights[i].index].SendGLPosition(GL_LIGHT0+i);//send position transformed by current cam matrix
	      }else {
		unsigned int li=GLLights[i].index;
		if ((*_llights)[li].enabled()&&((*_llights)[li].Target()==-1)) {
		  (*_llights)[li].ClobberGLLight (i);
		}else {
		  glDisable (GL_LIGHT0+i);
		  GLLights[i].index=-1;
		}
	      }
	    }else {
		glDisable (GL_LIGHT0+i);
		GLLights[i].options&=(~OpenGLL::GL_ENABLED);

	    }
	}
    }
}






