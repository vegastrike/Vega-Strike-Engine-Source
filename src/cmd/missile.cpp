#include "unit.h"
#include "missile.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include "config_xml.h"
#include "images.h"
#include "collection.h"
void StarSystem::UpdateMissiles() {
  if (!dischargedMissiles.empty()) {
    Unit * un;
    for (un_iter ui=getUnitList().createIterator();
	 NULL!= (un=(*ui));
	 ++ui) {
      dischargedMissiles.back()->ApplyDamage (un);
    }
    delete dischargedMissiles.back();
    dischargedMissiles.pop_back();
  }
  
}
void MissileEffect::ApplyDamage (Unit * smaller) {
  float rad =(smaller->Position()-pos).Magnitude()-smaller->rSize();
  rad=rad*rad;
  if (smaller->isUnit()!=MISSILEPTR&&rad<radius*radius) {
    //    fprintf (stderr,"exploding %s %d at radius %f with dist %f %f\n",smaller->name.c_str(),smaller,radius,sqrtf(rad),(smaller->Position()-pos).Magnitude());
    rad/=(radialmultiplier*radialmultiplier);
    if (rad<1)
      rad=1;
    if( (damage>0)) {
      Vector norm (pos-smaller->Position());
      norm.Normalize();
      //divide effects by r^2
      smaller->ApplyDamage (pos,norm,damage/rad,smaller,GFXColor(1,1,1,1),NULL,phasedamage>0?phasedamage/rad:0);
    }
    if (damage<0||phasedamage<0) {
      smaller->leach (1,phasedamage<0?-phasedamage:1,damage<0?-damage:0);
    }  
  }
}

float Missile::ExplosionRadius() {
  return radial_effect;
}

void StarSystem::AddMissileToQueue(MissileEffect * me) {
  dischargedMissiles.push_back (me);
}
void Missile::Discharge() {
  if (!discharged)
    _Universe->activeStarSystem()->AddMissileToQueue(new MissileEffect(Position(),damage,phasedamage,radial_effect,radial_multiplier));
  discharged =true;
}
void Missile::Kill (bool erase) {
  Discharge();
  Unit::Kill(erase);
}
void Missile::reactToCollision (Unit * smaller, const Vector & biglocation, const Vector & bignormal, const Vector & smalllocation, const Vector & smallnormal, float dist) {
  static bool doesmissilebounce  = XMLSupport::parse_bool (vs_config->getVariable("physics","missile_bounce","false"));
  if (doesmissilebounce) {

    Unit::reactToCollision (smaller,biglocation,bignormal,smalllocation,smallnormal,dist);
  }
  Discharge();
  if (!killed)
    DealDamageToHull (smalllocation,hull+1);//should kill, applying addmissile effect
  
}
void Missile::UpdatePhysics (const Transformation &trans, const Matrix transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc=NULL){
    Unit * targ;
    if ((targ=Target())) {
      if (rand()/((float)RAND_MAX)<((float)targ->GetImageInformation().ecm)*SIMULATION_ATOM/32768){
	Target (this);//go wild
      }
    }
    Unit::UpdatePhysics (trans, transmat, CumulativeVelocity, ResolveLast, uc);
    this->time-=SIMULATION_ATOM;
    if (NULL!=targ) {
      if ((Position()-targ->Position()).Magnitude()-targ->rSize()-rSize()<detonation_radius) {
	Discharge();
	time=-1;
      }
    }
    if (time<0) {
      DealDamageToHull (Vector(.1,.1,.1),hull+1);
    }
  }
