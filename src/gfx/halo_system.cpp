#include <vector>
#include <string>
#include "vec.h"
#include "matrix.h"
#include "halo_system.h"
#include "universe.h"
#include <stdlib.h>
#include <stdio.h>
#include "vegastrike.h"
#include "mesh.h"
#include "vs_globals.h"
#include "xml_support.h"
#include "config_xml.h"
#include "gfx/particle.h"
#include "lin_time.h"
#include "animation.h"
#include "car_assist.h"
void DoParticles (QVector pos, float percent, const Vector & velocity, float radial_size,int faction) {
  percent = 1-percent;
  int i=rand();
  static float scale = XMLSupport::parse_float (vs_config->getVariable("graphics",
								       "sparklescale",
								       
								       "8"));
  static float sspeed = XMLSupport::parse_float (vs_config->getVariable("graphics",
								       "sparklespeed",
								       
								       ".5"));
  static float flare = XMLSupport::parse_float (vs_config->getVariable("graphics",
								       "sparkleflare",
								       
								       ".5"));
  static float spread = XMLSupport::parse_float (vs_config->getVariable("graphics",
								       "sparklespread",
								       
								       ".04"));
  if (i<(RAND_MAX*percent)*(GetElapsedTime()*scale)) {
      ParticlePoint pp;
      float r1 = rand()/((float)RAND_MAX*.5)-1;
      float r2 = rand()/((float)RAND_MAX*.5)-1;      
      QVector rand(r1,r2,0);
      pp.loc = pos+rand*radial_size*flare;
      const float * col = FactionUtil::GetSparkColor(faction);
      pp.col.i=col[0];
      pp.col.j=col[1];
      pp.col.k=col[2];
      particleTrail.AddParticle(pp,rand*velocity.Magnitude()*spread+velocity*sspeed);
    }
}
  

void LaunchOneParticle (const Matrix &mat,const Vector &vel,unsigned int seed, Mesh * mush, float hull,int faction) {
	if (mush){
		static bool ignoreGetBlendDst = XMLSupport::parse_bool (vs_config->getVariable ("graphics","sparkleoffglow","true"));
	if (mush->getBlendDst()!=ONE||ignoreGetBlendDst) {		
		unsigned int numvert = mush->numVertices();
		if (numvert) {
			unsigned int whichvert = seed%numvert;
			QVector v (mush->GetVertex(whichvert).Cast());
			v=Transform(mat,v);
			DoParticles (v,hull,vel,0,faction);
		}
	}
	}
}



HaloSystem::HaloSystem() {
  VSCONSTRUCT2('h')
  mesh=NULL;
  activation=0;
}

MyIndHalo::MyIndHalo(const QVector & loc, const Vector & size) {
    this->loc = loc;
    this->size=size;
}

unsigned int HaloSystem::AddHalo (const char * filename, const QVector & loc, const Vector &size, const GFXColor & col, std::string type, float activation_speed) {
#ifdef CAR_SIM
  ani.push_back (new Animation ("flare6.ani",1,.1,MIPMAP,true,true,col));
  ani.back()->SetDimensions (size.i,size.j);
  ani.back()->SetPosition(loc);
  halo_type.push_back (CAR::type_map.lookup(type));//should default to headlights
#endif
  if (mesh==NULL) {
    mesh = new Mesh ((string (filename)+".xmesh").c_str(), 1,FactionUtil::GetFaction("neutral"),NULL);
    float gs =XMLSupport::parse_float (vs_config->getVariable("physics","game_speed","1"));
    activation=activation_speed*activation_speed*gs*gs;
  }
  static float engine_scale = XMLSupport::parse_float (vs_config->getVariable ("graphics","engine_radii_scale",".4"));
  static float engine_length = XMLSupport::parse_float (vs_config->getVariable ("graphics","engine_length_scale","1.25"));

  halo.push_back (MyIndHalo (loc, Vector (size.i*engine_scale,size.j*engine_scale,size.k)));
  return halo.size()-1;
}
using std::vector;
void HaloSystem::SetSize (unsigned int which, const Vector &size) {
  halo[which].size = size;
#ifdef CAR_SIM
  ani[which]->SetDimensions (size.i,size.j);
#endif
}
void HaloSystem::SetPosition (unsigned int which, const QVector &loc) {
  halo[which].loc = loc;
#ifdef CAR_SIM
  ani[which]->SetPosition(loc);
#endif

}
bool HaloSystem::ShouldDraw (float speedsquared) {
  return speedsquared>activation;
}
void HaloSystem::Draw(const Matrix & trans, const Vector &scale, short halo_alpha, float nebdist, float hullpercent, const Vector & velocity, int faction) {
#ifdef CAR_SIM
    for (unsigned int i=0;i<ani.size();++i) {
      int bitwise = scale.j;
      int typ = 0;
#ifdef CAR_SIM
      typ = halo_type [i];
#endif
      bool      drawnow= (typ==CAR::RUNNINGLIGHTS);
      if ((typ==CAR::BRAKE&&scale.k<.01&&scale.k>-.01)) {
	drawnow=true;
      }
      if ((typ==CAR::REVERSE&&scale.k<=-.01)) {
	drawnow=true;
      }
      if (typ==CAR::HEADLIGHTS) {
	if (scale.j>=CAR::ON_NO_BLINKEN||(bitwise<CAR::ON_NO_BLINKEN&&bitwise>0&&(bitwise&CAR::FORWARD_BLINKEN))) {
	  drawnow = true;
	}
      }
      if (typ==CAR::SIREN) {
	if ((bitwise>0)&&((bitwise>=CAR::ON_NO_BLINKEN)||(bitwise&CAR::SIREN_BLINKEN))) {
	  drawnow=true;
	}
      }
      float blink_prob=.8;
      if (typ==CAR::RIGHTBLINK) {
	if ((bitwise>0)&&(bitwise<CAR::ON_NO_BLINKEN)&&(bitwise&CAR::RIGHT_BLINKEN)) {
	  if (rand()<RAND_MAX*blink_prob) 
	    drawnow=true;
	}
      }
      if (typ==CAR::LEFTBLINK) {
	if ((bitwise>0)&&(bitwise<CAR::ON_NO_BLINKEN)&&(bitwise&CAR::LEFT_BLINKEN)) {
	  if (rand()<RAND_MAX*blink_prob) 
	    drawnow=true;
	}
      }
      if (drawnow) {
	ani[i]->SetPosition (Transform (trans,halo[i].loc));
	ani[i]->SetDimensions (scale.i,scale.i);
	ani[i]->Draw();
      }

    }
#else
  if (scale.k>0) {
    vector<MyIndHalo>::iterator i = halo.begin();
    for (;i!=halo.end();++i) {
      
      Matrix m = trans;
      ScaleMatrix (m,Vector (scale.i*i->size.i,scale.j*i->size.j,scale.k*i->size.k));
      m.p = Transform (trans,i->loc);
      mesh->Draw(50000000000000.0,m,1,halo_alpha,nebdist);    
      if (hullpercent<.99) {
	DoParticles(m.p,hullpercent,velocity,mesh->rSize()*scale.i,faction);
      }
    }
  }
#endif
}
HaloSystem::~HaloSystem() {
#ifdef CAR_SIM
  for (unsigned int i=0;i<ani.size();i++) {
    delete ani[i];
  }
  ani.clear();
#endif
  VSDESTRUCT2
  if (mesh) {
    delete mesh;
  }
}
