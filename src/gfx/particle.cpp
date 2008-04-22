#include "particle.h"
#include "gfxlib.h"
#include "lin_time.h"
#include "vs_globals.h"
#include "cmd/unit_generic.h"
#include "config_xml.h"
#include "camera.h"
#include "aux_texture.h"
#include "gldrv/gl_globals.h"
using namespace std;

ParticleTrail particleTrail(500);
void ParticleTrail::ChangeMax (unsigned int max ) {
  this->maxparticles = max;
}

bool ParticlePoint::Draw(const Vector & vel,const double time, Vector p, Vector q) {
  static float pgrow=XMLSupport::parse_float (vs_config->getVariable ("graphics","sparkegrowrate","200.0")); // 200x size when disappearing
  static float adj = XMLSupport::parse_float (vs_config->getVariable("graphics","sparklefade","0.1"));
  static float trans=XMLSupport::parse_float (vs_config->getVariable("graphics","sparklealpha","2.5")); // NOTE: It's the base transparency, before surface attenuation, so it needn't be within the [0-1] range.
  static bool use_points = XMLSupport::parse_bool(vs_config->getVariable("graphics","point_sparkles","false"));


  float size = this->size*(pgrow*(1-col.a)+col.a);
  float maxsize = (this->size >size)?this->size:size;
  float minsize = (this->size<=size)?this->size:size;
  //Squared, surface-linked decay - looks nicer, more real for emmisive gasses
  //NOTE: maxsize/minsize allows for inverted growth (shrinkage) while still fading correctly. Cheers!
  GFXColorf(col*(col.a*trans*(minsize/((maxsize>0)?maxsize:1.f)))); 

  {
  QVector loc= this->loc-_Universe->AccessCamera()->GetPosition();
  if (use_points) {
    GFXVertexf(loc);
  } else {
  #if 0
    q*=size;
    p*=size;  
    GFXTexCoord2f(0,0);
    GFXVertex3d(loc.i+p.i+q.i,loc.j+p.j+q.j,loc.k+p.k+q.k);
    GFXTexCoord2f(0,1);  
    GFXVertex3d(loc.i+p.i-q.i,loc.j+p.j-q.j,loc.k+p.k-q.k);
    GFXTexCoord2f(1,1);  
    GFXVertex3d(loc.i-p.i-q.i,loc.j-p.j-q.j,loc.k-p.k-q.k);
    GFXTexCoord2f(1,0);  
    GFXVertex3d(loc.i-p.i+q.i,loc.j-p.j+q.j,loc.k-p.k+q.k);
#else
    GFXTexCoord2f(0,0);
    GFXVertex3d(loc.i+size,loc.j+size,loc.k);
    GFXTexCoord2f(0,1);  
    GFXVertex3d(loc.i+size,loc.j-size,loc.k);
    GFXTexCoord2f(1,1);  
    GFXVertex3d(loc.i-size,loc.j-size,loc.k);
    GFXTexCoord2f(1,0);  
    GFXVertex3d(loc.i-size,loc.j+size,loc.k);
    
    GFXTexCoord2f(0,0);
    GFXVertex3d(loc.i,loc.j+size,loc.k+size);
    GFXTexCoord2f(0,1);  
    GFXVertex3d(loc.i,loc.j-size,loc.k+size);
    GFXTexCoord2f(1,1);  
    GFXVertex3d(loc.i,loc.j-size,loc.k-size);
    GFXTexCoord2f(1,0);  
    GFXVertex3d(loc.i,loc.j+size,loc.k-size);
    
    GFXTexCoord2f(0,0);
    GFXVertex3d(loc.i+size,loc.j,loc.k+size);
    GFXTexCoord2f(0,1);  
    GFXVertex3d(loc.i+size,loc.j,loc.k-size);
    GFXTexCoord2f(1,1);  
    GFXVertex3d(loc.i-size,loc.j,loc.k-size);
    GFXTexCoord2f(1,0);  
    GFXVertex3d(loc.i-size,loc.j,loc.k+size);

#endif
#if 0
    GFXEnd();
    GFXBegin(GFXPOINT);
    GFXVertexf(loc);
    GFXEnd();
    GFXBegin(GFXQUAD);
#endif
  }
  }

  loc+=(vel*time).Cast();
  col = (col-GFXColor(adj*time,adj*time,adj*time,adj*time)).clamp();
  return (col.a != 0);
}
void ParticleTrail::DrawAndUpdate (){
  static bool use_points = XMLSupport::parse_bool(vs_config->getVariable("graphics","point_sparkles","false"));
  Vector P,Q;
  {
	  Vector R;
	  _Universe->AccessCamera()->GetPQR(P,Q,R);
  	  static float particlesize = XMLSupport::parse_float (vs_config->getVariable("graphics","sparksize","1"));
	  P*=particlesize;
	  Q*=particlesize;
  }

  vector<Vector>::iterator v=particleVel.begin();
  vector<ParticlePoint>::iterator p=particle.begin();
  if (use_points) {
    GFXDisable(TEXTURE0);
    GFXDisable(CULLFACE);
    static float psiz=XMLSupport::parse_float (vs_config->getVariable ("graphics","sparkesize","1.5"));
    
    GFXPointSize(psiz);
    
    static bool psmooth=XMLSupport::parse_bool (vs_config->getVariable ("graphics","sparkesmooth","false"));
    if (psmooth && gl_options.smooth_points) {
      glEnable(GL_POINT_SMOOTH);
    }
  } else {
    GFXEnable(TEXTURE0);
    GFXDisable(TEXTURE1);
    GFXDisable(DEPTHWRITE);
    GFXDisable(CULLFACE);
    static string s = vs_config->getVariable("graphics","sparkletexture","supernova.bmp");
    static Texture * t = new Texture (s.c_str());
    
    t->MakeActive();
  }
  GFXDisable(LIGHTING);
  GFXLoadIdentity(MODEL);
  GFXTranslateModel(_Universe->AccessCamera()->GetPosition());
  static bool pblend=XMLSupport::parse_bool (vs_config->getVariable ("graphics","sparkeblend","false"));
  //GFXBlendMode(ONE,ZERO);
  if (use_points) {
    if (pblend)
      GFXBlendMode(SRCALPHA,INVSRCALPHA);
    else
      GFXBlendMode(ONE,ZERO);
    GFXBegin (GFXPOINT);
  } else {
    GFXBlendMode(ONE,ONE);
    GFXBegin (GFXQUAD);
  }
  
  double mytime= GetElapsedTime();
  while (p!=particle.end()) {
    if (!(*p).Draw(*v,mytime,P,Q)) {
      vector<Vector>::iterator vlast=particleVel.end();
      vector<ParticlePoint>::iterator plast=particle.end();
      --vlast;--plast;
      if (p!=plast) {
        *v=*vlast;
        *p=*plast;
      }
      size_t index=p-particle.begin();
      size_t indexv=v-particleVel.begin();
      particle.pop_back();
      particleVel.pop_back();
      p=particle.begin()+index;
      v=particleVel.begin()+indexv;//continue where we left off
    }else {
      ++p;
      ++v;
    }
  }
  GFXEnd();
  if (use_points) {  
    glDisable (GL_POINT_SMOOTH);
    GFXPointSize(1);
  } else {
    GFXDisable(DEPTHWRITE);
    GFXDisable(CULLFACE);
  }
  GFXLoadIdentity(MODEL);
}

void ParticleTrail::AddParticle (const ParticlePoint &P, const Vector &V,float size) {
  if (particle.size()>maxparticles) {
    vector<Vector>::iterator vel=particleVel.begin();
    vector<ParticlePoint>::iterator p=particle.begin();
    size_t off=((size_t)rand())%particle.size();
    vel+=off;
    p+=off;
    *p=P;
    (*p).size=size;
    *vel=V;
  }else {
    particle.push_back (P);
    particle.back().size=size;
    particleVel.push_back (V);
  }
}
