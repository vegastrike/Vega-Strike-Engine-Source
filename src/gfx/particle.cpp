#include "particle.h"
#include "gfxlib.h"
#include "lin_time.h"
#include "vs_globals.h"
#include "config_xml.h"
using namespace std;
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

bool ParticlePoint::Draw(const Vector & vel,const double time) {
  loc+=(vel*time).Cast();
  GFXColor4f(col.i,col.j,col.k,1);
  GFXVertexf(loc);
  return colorOK(col,time);
}
void ParticleTrail::DrawAndUpdate (){
  list<Vector>::iterator v=particleVel.begin();
  list<ParticlePoint>::iterator p=particle.begin();
  GFXDisable(TEXTURE0);
  GFXDisable(LIGHTING);
  GFXLoadIdentity(MODEL);
  static bool psmooth=XMLSupport::parse_bool (vs_config->getVariable ("graphics","sparkesmooth","false"));  
  glEnable(psmooth);
  static bool pblend=XMLSupport::parse_bool (vs_config->getVariable ("graphics","sparkeblend","false"));
  if (pblend)
	  GFXBlendMode(SRCALPHA,INVSRCALPHA);
  else
	  GFXBlendMode(ONE,ZERO);
  //GFXBlendMode(ONE,ZERO);
  static float psiz=XMLSupport::parse_float (vs_config->getVariable ("graphics","sparkesize","1.5"));
  GFXPointSize(psiz);
  GFXBegin (GFXPOINT);
  double mytime= GetElapsedTime();
  while (p!=particle.end()) {
    if (!(*p).Draw(*v,mytime)) {
      p =particle.erase(p);
      v = particleVel.erase(v);
    }else {
      ++p;
      ++v;
    }
  }
  GFXEnd();
  glDisable(GL_POINT_SMOOTH);

  GFXPointSize(1);
}

void ParticleTrail::AddParticle (const ParticlePoint &P, const Vector &V) {
  particle.push_back (P);
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
