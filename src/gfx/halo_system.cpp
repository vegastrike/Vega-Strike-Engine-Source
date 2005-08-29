#include <vector>
#include <string>
#include "vec.h"
#include "matrix.h"
#include "cmd/unit_generic.h"
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
#include "cmd/collide/rapcol.h"
#include "cmd/unit_collide.h"

#define HALO_SMOOTHING_UP_FACTOR 0.02
#define HALO_SMOOTHING_DOWN_FACTOR 0.01
#define HALO_STEERING_UP_FACTOR 0.00
#define HALO_STEERING_DOWN_FACTOR 0.01
#define HALO_STABILIZATION_RANGE 0.25

static float ffmax(float a, float b) {
  return a>b?a:b;
}
void DoParticles (QVector pos, float percent, const Vector & velocity, float radial_size,float particle_size,int faction) {
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
  static float absspeed = XMLSupport::parse_float (vs_config->getVariable("graphics",
								       "sparkleabsolutespeed",
								       
								       ".04"));
  static bool fixed_size= XMLSupport::parse_bool (vs_config->getVariable("graphics",
								       "sparklefixedsize",
								       
								       "0"));
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
	  static float sciz=XMLSupport::parse_float (vs_config->getVariable("graphics","sparklesizeenginerelative",".125"));
      particleTrail.AddParticle(pp,rand*(ffmax(velocity.Magnitude(),absspeed)*spread+absspeed)+velocity*sspeed,fixed_size?sciz:(particle_size*sciz));
    }
}
  

void LaunchOneParticle (const Matrix &mat,const Vector &vel,unsigned int seed, Unit * mush, float hull,int faction) {
  static float sciz = XMLSupport::parse_float (vs_config->getVariable ("graphics","sparkleenginesizerelativetoship",".0625"));
  
  if (mush){
          bool done=false;
          collideTrees * colTrees=mush->colTrees;
          if (colTrees) {
            if (colTrees->usingColTree()) {            
              csRapidCollider * colTree=colTrees->rapidColliders[0];
              unsigned int numvert=colTree->getNumVertex();
              if (numvert) {
                unsigned int whichvert = seed%numvert;
                QVector v (colTree->getVertex(whichvert).Cast());
                v=Transform(mat,v);
                DoParticles (v,hull,vel,0,mush->rSize()*sciz,faction);
                done=true;
              }
            }
          }
          if (!done) {
            // get it from the BSP
            
          }
          if (!done) {
            unsigned int siz=(unsigned int) (2*mush->rSize());
            if (siz!=0){
              QVector v((seed%siz)-siz/2,
                        (seed%siz)-siz/2,
                        (seed%siz)-siz/2);
              DoParticles(v,hull,vel,0,mush->rSize()*sciz,faction);
              done=true;
            }
          }
	}

}



HaloSystem::HaloSystem() {
  VSCONSTRUCT2('h')
  mesh=NULL;
  activation=0;
  oscale=0;
}

MyIndHalo::MyIndHalo(const QVector & loc, const Vector & size) {
    this->loc = loc;
    this->size = size;
}

unsigned int HaloSystem::AddHalo (const char * filename, const QVector & loc, const Vector &size, const GFXColor & col, std::string type, float activation_accel) {
#ifdef CAR_SIM
  ani.push_back (new Animation ("flare6.ani",1,.1,MIPMAP,true,true,col));
  ani.back()->SetDimensions (size.i,size.j);
  ani.back()->SetPosition(loc);
  halo_type.push_back (CAR::type_map.lookup(type));//should default to headlights
#endif
  if (mesh==NULL) {
	  mesh = Mesh::LoadMesh ((string (filename)).c_str(), Vector(1,1,1),FactionUtil::GetFaction("neutral"),NULL);
    static float gs =XMLSupport::parse_float (vs_config->getVariable("physics","game_speed","1"));
    activation=activation_accel*gs;
  }
  static float engine_scale = XMLSupport::parse_float (vs_config->getVariable ("graphics","engine_radii_scale",".4"));
  static float engine_length = XMLSupport::parse_float (vs_config->getVariable ("graphics","engine_length_scale","1.25"));

  halo.push_back (MyIndHalo (loc, Vector (size.i*engine_scale,size.j*engine_scale,size.k*engine_length)));
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

static float HaloAccelSmooth(float linaccel, float olinaccel, float maxlinaccel) {
    linaccel = max(0,min(maxlinaccel,linaccel)); //Clamp input, somehow, sometimes it's not clamped
    float phase = pow(((linaccel>olinaccel)?HALO_SMOOTHING_UP_FACTOR:HALO_SMOOTHING_DOWN_FACTOR),GetElapsedTime());
    float olinaccel2;
    if (linaccel>olinaccel)
        olinaccel2 = min(linaccel,olinaccel+maxlinaccel*HALO_STEERING_UP_FACTOR); else
        olinaccel2 = max(linaccel,olinaccel-maxlinaccel*HALO_STEERING_DOWN_FACTOR);
    linaccel=(1-phase)*linaccel+phase*olinaccel2;
    linaccel=max(0,min(maxlinaccel,linaccel));
    return linaccel;
}

bool HaloSystem::ShouldDraw (const Matrix & trans, const Vector & velocity, const Vector & accel, float maxaccel, float maxvelocity) {
    static bool halos_by_velocity = XMLSupport::parse_bool( vs_config->getVariable("graphics","halos_by_velocity","false") );

    Vector thrustvector=trans.getR().Normalize();
    if (halos_by_velocity) {
        float linvel=velocity.Dot(thrustvector);
        return (linvel>activation);
    } else {
        if (maxaccel<=0) maxaccel=1;
        if (maxvelocity<=0) maxvelocity=1;
        float linaccel=HaloAccelSmooth(accel.Dot(thrustvector)/maxaccel,oscale,1.0f);
        return (linaccel>activation*maxaccel);
    }
}
void HaloSystem::Draw(const Matrix & trans, const Vector &scale, int halo_alpha, float nebdist, float hullpercent, const Vector & velocity, const Vector & accel, float maxaccel, float maxvelocity, int faction) {
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
  if (halo_alpha>=0) {
    halo_alpha/=2;
    if ((halo_alpha&0x1)==0) {
      halo_alpha+=1;
    }
    
  }

  static bool halos_by_velocity = XMLSupport::parse_bool( vs_config->getVariable("graphics","halos_by_velocity","false") );

  Vector thrustvector=trans.getR().Normalize();
  if (maxaccel<=0) maxaccel=1;
  if (maxvelocity<=0) maxvelocity=1;
  float value,maxvalue,minvalue;
  if (halos_by_velocity) {
      value = velocity.Dot(thrustvector);
      maxvalue = maxvelocity;
      minvalue = activation;
  } else {
      oscale = HaloAccelSmooth(accel.Dot(thrustvector)/maxaccel,oscale,1.0f);
      value = oscale;
      maxvalue = 1.0f;
      minvalue = activation/maxaccel;
  }
  if ((value>minvalue)&&(scale.k>0)) {
    vector<MyIndHalo>::iterator i = halo.begin();
    for (;i!=halo.end();++i) {
        Matrix m = trans;
        ScaleMatrix (m,Vector (scale.i*i->size.i,scale.j*i->size.j,scale.k*i->size.k*value/maxvalue));
        m.p = Transform (trans,i->loc);
        mesh->Draw(50000000000000.0,m,1,halo_alpha,nebdist);   
        if (hullpercent<.99)
            DoParticles(m.p,hullpercent,velocity,mesh->rSize()*scale.i,mesh->rSize()*scale.i,faction);
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
