#include "config_xml.h"
#include "vs_globals.h"
#include "turretai.h"
#include "cmd/unit.h"
using namespace Orders;
TurretAI::TurretAI ():FaceTarget (false) {
  type|=WEAPON;
  range=-1;
}


void TurretAI::Execute () {
  Unit * targ = parent->Target();
  if (targ) {
    static float dot_cutoff = XMLSupport::parse_float (vs_config->getVariable ("AI","Firing","TurretDotCutoff",".4"));
    static float missile_prob = XMLSupport::parse_float (vs_config->getVariable ("AI","Firing","MissileProbability",".01"));
    FaceTarget::Execute();
    if (parent->GetNumMounts()>0) {
      if (range==-1) {
	float speed;
        parent->getAverageGunSpeed (speed, range);
      }
      Vector R (parent->GetTransformation().getR());
      QVector Pos (targ->Position()-parent->Position());
      double mag = Pos.Magnitude();
      Pos=Pos/mag;
      float dot = R.Dot (Pos.Cast());
      if (mag-targ->rSize()-parent->rSize()<1.2*range&&dot>dot_cutoff) {
	parent->Fire(false);
	if (missile_prob*SIMULATION_ATOM*RAND_MAX<rand()) {
	  parent->Fire(true);
	}
      }else {
	parent->UnFire();
      }
    }    
  }
}
