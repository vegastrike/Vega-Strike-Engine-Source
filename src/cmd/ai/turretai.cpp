#include "config_xml.h"
#include "vs_globals.h"
#include "turretai.h"
#include "cmd/unit_generic.h"
#include "cmd/role_bitmask.h"
using namespace Orders;
TurretAI::TurretAI ():FaceTarget (false) {
  type|=WEAPON;
  range=-1;
}
void TurretAI::getAverageGunSpeed (float &speed, float & range, float &mrange) const {
  speed=this->speed;range=this->range;mrange=this->mrange;
}
extern unsigned int FireBitmask (Unit * parent, bool shouldfire, float missileprob);
void TurretAI::Execute () {
  Unit * targ = parent->Target();
  if (range==-1) {
	float mrange;
    parent->getAverageGunSpeed (speed, range,mrange);
    float tspeed, trange,tmrange;
    Unit * gun;
    if (parent->GetNumMounts()==0){
      speed=1;range=1;//not much
    }
    for (un_iter i=parent->getSubUnits();(gun=*i)!=NULL;++i) {
      (*i)->getAverageGunSpeed(tspeed,trange,tmrange);
      if (trange>range) {
	speed=tspeed;
	range=trange;
      }
    }
  }
  if (targ) {
    static float dot_cutoff = XMLSupport::parse_float (vs_config->getVariable ("AI","Firing","TurretDotCutoff",".4"));
    static float missile_prob = XMLSupport::parse_float (vs_config->getVariable ("AI","Firing","TurretMissileProbability",".05"));
    FaceTarget::Execute();
    if (parent->GetNumMounts()>0) {
      Vector R (parent->GetTransformation().getR());
      QVector Pos (targ->Position()-parent->Position());
      double mag = Pos.Magnitude();
      Pos=Pos/mag;
      float dot = R.Dot (Pos.Cast());
      bool shouldfire = (mag-targ->rSize()-parent->rSize()<1.2*range&&dot>dot_cutoff);
      parent->Fire(FireBitmask(parent,shouldfire,missile_prob));
      if (!shouldfire)
	parent->UnFire();
      
    }   
  }
}
