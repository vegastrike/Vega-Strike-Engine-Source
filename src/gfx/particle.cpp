#include "particle.h"
#include "gfxlib.h"
#include "lin_time.h"
#include "vs_globals.h"
#include "cmd/unit_generic.h"
#include "config_xml.h"
#include "camera.h"
#include "aux_texture.h"
using namespace std;
//#define USE_POINTS
ParticleTrail particleTrail(1000);
void ParticleTrail::ChangeMax (unsigned int max ) {
  this->maxparticles = max;
}
static bool colorOK(Vector &col, const double time) {
  static float adj = XMLSupport::parse_float (vs_config->getVariable("graphics",
								       "sparklefade",
								       
								       ".2"));
  col=col-Vector (adj*time,adj*time,adj*time);
  if (col.i<=0) {
    col.i=0;}
  if (col.j<=0) {
    col.j=0;}
  if (col.k<=0) {
    col.k=0;}
  return !(col.i==0&&col.j==0&&col.k==0);
}

bool ParticlePoint::Draw(const Vector & vel,const double time, Vector p, Vector q) {
  loc+=(vel*time).Cast();
  GFXColor4f(col.i,col.j,col.k,1);
#ifdef USE_POINTS
  GFXVertexf(loc);
#else
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
#endif
  return colorOK(col,time);
}
void ParticleTrail::DrawAndUpdate (){
  Vector P,Q;
  {
	  Vector R;
	  _Universe->AccessCamera()->GetPQR(P,Q,R);
  	  static float particlesize = XMLSupport::parse_float (vs_config->getVariable("graphics","sparksize","1"));
	  P*=particlesize;
	  Q*=particlesize;
  }
  list<Vector>::iterator v=particleVel.begin();
  list<ParticlePoint>::iterator p=particle.begin();
#ifdef USE_POINTS
  GFXDisable(TEXTURE0);
  GFXDisable(CULLFACE);
  static float psiz=XMLSupport::parse_float (vs_config->getVariable ("graphics","sparkesize","1.5"));
  
  GFXPointSize(psiz);
  static bool psmooth=XMLSupport::parse_bool (vs_config->getVariable ("graphics","sparkesmooth","false"));  
  glEnable(psmooth);
#else
  GFXEnable(TEXTURE0);
  GFXDisable(TEXTURE1);
  GFXDisable(DEPTHWRITE);
  GFXDisable(CULLFACE);
  static string s = vs_config->getVariable("graphics","sparkletexture","supernova.bmp");
  static Texture * t = new Texture (s.c_str());
  
  t->MakeActive();
#endif
  GFXDisable(LIGHTING);
  GFXLoadIdentity(MODEL);
  static bool pblend=XMLSupport::parse_bool (vs_config->getVariable ("graphics","sparkeblend","false"));
  //GFXBlendMode(ONE,ZERO);
#ifdef USE_POINTS
  if (pblend)
	  GFXBlendMode(SRCALPHA,INVSRCALPHA);
  else
	  GFXBlendMode(ONE,ZERO);
  GFXBegin (GFXPOINT);
#else
  GFXBlendMode(ONE,ONE);
  GFXBegin (GFXQUAD);
#endif
  
  double mytime= GetElapsedTime();
  while (p!=particle.end()) {
    if (!(*p).Draw(*v,mytime,P,Q)) {
      p =particle.erase(p);
      v = particleVel.erase(v);
    }else {
      ++p;
      ++v;
    }
  }
  GFXEnd();
#ifdef USE_POINTS  
  glDisable(GL_POINT_SMOOTH);
  GFXPointSize(1);
#else
  GFXDisable(DEPTHWRITE);
  GFXDisable(CULLFACE);
#endif

}

void ParticleTrail::AddParticle (const ParticlePoint &P, const Vector &V,float size) {
	
  particle.push_back (P);
  particle.back().size=size;
  particleVel.push_back (V);
  if (particle.size()>maxparticles) {
    PopParticle();
  }
}
void ParticleTrail::PopParticle() {
  if (!particle.empty()) {
    particle.pop_front();
    particleVel.pop_front();
  }
}
