#include "nebula.h"
#include "vegastrike.h"
#include "vs_path.h"
#include <assert.h>
#include "config_xml.h"
#include "vs_globals.h"
#include <sys/stat.h>
#include "xml_support.h"
#include "gfx/mesh.h"
#undef BOOST_NO_CWCHAR

using XMLSupport::EnumMap;
using XMLSupport::Attribute;
using XMLSupport::AttributeList;

extern double interpolation_blend_factor;
void GameNebula::SetFogState () {
  float thisfadein = (lastfadein*(1-interpolation_blend_factor)+ (fadeinvalue)*interpolation_blend_factor);
  GFXFogMode (fogmode);
  GFXFogDensity (Density*thisfadein);
  GFXFogLimits (fognear,fogfar*thisfadein);
  GFXFogColor (GFXColor (color.i,color.j,color.k,1));
  GFXFogIndex (index);
  
}

// WARNING : USED TO CALL a GameUnit constructor but now Nebula::Nebula calls a Unit one
GameNebula::GameNebula(const char * unitfile, bool SubU, int faction,
	       Flightgroup* fg, int fg_snumber) :
  GameUnit<Nebula> (unitfile,SubU,faction,string(""),fg,fg_snumber)
{
	  Nebula::InitNebula( unitfile, SubU, faction, fg, fg_snumber);
}

void GameNebula::UpdatePhysics2 (const Transformation &trans, const Transformation & o, const Matrix & m ,float diff,const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc) {
  static float nebdelta= XMLSupport::parse_float (vs_config->getVariable ("graphics","fog_time",".01"));
  lastfadein = fadeinvalue;
  fadeinvalue-=nebdelta*SIMULATION_ATOM;
  if (fadeinvalue<0) {
    fadeinvalue=0;
  }
  GameUnit<Nebula>::UpdatePhysics2 (trans,o,m,d,CumulativeVelocity,ResolveLast,uc);
  Vector t1;
  float dis;
  unsigned int i;
  if (_Universe->activeStarSystem()==_Universe->AccessCockpit()->activeStarSystem) {
  for (i=0;i<NUM_CAM;i++) {
    if (Inside (_Universe->AccessCamera(i)->GetPosition(),0,t1,dis)) {
      PutInsideCam(i);

    }
  }
  }
  if (nummesh()>0) {
    i = rand()%(nummesh());
    Vector randexpl (rand()%2*rSize()-rSize(),rand()%2*rSize()-rSize(),rand()%2*rSize()-rSize());
    if (((int)(explosiontime/SIMULATION_ATOM))!=0) 
      if (!(rand()%((int)(explosiontime/SIMULATION_ATOM)))) 
	meshdata[i]->AddDamageFX(randexpl,Vector (0,0,0),.00001,color);
  }
}

void GameNebula::PutInsideCam(int i) {
  static float nebdelta= XMLSupport::parse_float (vs_config->getVariable ("graphics","fog_time",".01"));
  static float fadeinrate= XMLSupport::parse_float (vs_config->getVariable ("graphics","fog_fade_in_percent","0.5"));
  if (_Universe->AccessCamera()==_Universe->AccessCamera(i)) {
    fadeinvalue+=(1+fadeinrate)*nebdelta*SIMULATION_ATOM;
    if (fadeinvalue>1) {
      fadeinvalue=1;
    }
  }
  _Universe->AccessCamera(i)->SetNebula (this);
}
