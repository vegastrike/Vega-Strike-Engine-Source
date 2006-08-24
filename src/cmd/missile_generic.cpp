#include "universe_util.h"
#include "missile_generic.h"
#include "unit_generic.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include "configxml.h"
#include "images.h"
#include "collection.h"
#include "star_system_generic.h"
#include "role_bitmask.h"
#include "ai/order.h"
#include "faction_generic.h"
#include "unit_util.h"
void StarSystem::UpdateMissiles() {  
  // WARNING: This is a big performance problem...
  //   ...responsible for many hiccups.
  //   TODO: Make it use the collidemap to only iterate through potential hits...
  //   PROBLEM: The current collidemap does not allow this efficiently (no way of
  //      taking the other unit's rSize() into account).
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
  float rad =(smaller->Position().Cast()-pos).Magnitude()-smaller->rSize();
  if(rad<.001) rad =.001;
  float orig = rad;
  rad=rad*rad;
  if (smaller->isUnit()!=MISSILEPTR&&rad<radius*radius) {
	if(rad<(radialmultiplier*radialmultiplier)){
	  rad = (radialmultiplier*radialmultiplier*radialmultiplier*radialmultiplier/((2*radialmultiplier*radialmultiplier)-(orig*orig))); 
	  /*
	  contrived formula to create paraboloid falloff rather than quadratic peaking at 2x damage at origin
	  rad = radmul^4/(2radmul^2-orig^2)
	  */
	}
    rad=rad/(radialmultiplier*radialmultiplier); // where radialmultiplier is radius of point with 0 falloff
	
      Vector norm (pos-smaller->Position().Cast());
      norm.Normalize();
	  //UniverseUtil::IOmessage(0,"game","all",string("dealt ")+XMLSupport::tostring(damage/rad)+string(" damage from ")+XMLSupport::tostring(orig)+string(" meters ")+XMLSupport::tostring(damage)+string(" damage and "+XMLSupport::tostring(radialmultiplier)+string(" rad mult"))); 
      //divide effects by r^2
      smaller->ApplyDamage (pos,norm,damage/rad,smaller,GFXColor(1,1,1,1),ownerDoNotDereference,phasedamage>0?phasedamage/rad:0);
  }
}

float Missile::ExplosionRadius() {  

	static float missile_multiplier = XMLSupport::parse_float (vs_config->getVariable ("graphics","missile_explosion_radius_mult","1"));   

	return radial_effect*(missile_multiplier); 

}

void StarSystem::AddMissileToQueue(MissileEffect * me) {
  dischargedMissiles.push_back (me);
}
void Missile::Discharge() {
  if ((damage!=0||phasedamage!=0)&&!discharged)
    _Universe->activeStarSystem()->AddMissileToQueue(new MissileEffect(Position().Cast(),damage,phasedamage,radial_effect,radial_multiplier,owner));
  discharged =true;
}
void Missile::Kill (bool erase) {
  Discharge();
  Unit::Kill(erase);
}
void Missile::reactToCollision (Unit * smaller, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal, float dist) {
  static bool doesmissilebounce  = XMLSupport::parse_bool (vs_config->getVariable("physics","missile_bounce","false"));
  if (doesmissilebounce) {
    Unit::reactToCollision (smaller,biglocation,bignormal,smalllocation,smallnormal,dist);
  }
  if (smaller->isUnit()!=MISSILEPTR) {//2 missiles in a row can't hit each other
    this->Velocity = smaller->Velocity;
    Velocity = smaller->Velocity;
    Discharge();
    if (!killed)
      DealDamageToHull (smalllocation.Cast(),hull+1);//should kill, applying addmissile effect
  }
  
}

Unit * getNearestTarget (Unit *me) {
  return NULL;//THIS FUNCTION IS TOO SLOW__AND ECM SHOULD WORK DIFFERENTLY ANYHOW...WILL SAVE FIXING IT FOR LATER
  QVector pos (me->Position());
  Unit * un=NULL;
  Unit * targ=NULL;
  double minrange=FLT_MAX;
  for (un_iter i=_Universe->activeStarSystem()->getUnitList().createIterator();
       (un=(*i));
       ++i) {
         if (un==me)
           continue;
         if (un->isUnit()!=UNITPTR) {
            continue;     
         }
		 if (un->hull<0)
			 continue;
		 if (FactionUtil::GetIntRelation(me->faction,un->faction)>=0) {
			 continue;
		 }
         double temp= (un->Position()-pos).Magnitude()-un->rSize();
         if (targ==NULL) {
            targ = un;
            minrange = temp;
         }else {
           if (temp<minrange) {
              targ = un;
           }
         }
       }
  if (targ==NULL) {
	  for (un_iter i=_Universe->activeStarSystem()->getUnitList().createIterator();
		   (un=(*i));
		   ++i) {
		  if (UnitUtil::isSun(un)){
			  targ = un;
			  break;
		  }
	  }
  }
  return targ;
}
void Missile::UpdatePhysics2 (const Transformation &trans, const Transformation & old_physical_state, const Vector & accel, float difficulty, const Matrix &transmat, const Vector & CumulativeVelocity, bool ResolveLast, UnitCollection *uc) {
    Unit * targ;
	if ((targ=(Unit::Target()))) {
          had_target=true;
		if (targ->hull<0){
			targ=NULL; 
		}else {
                  static int max_ecm = XMLSupport::parse_int(vs_config->getVariable("physics","max_ecm","4"));
			if ((((size_t)this)/16383)%max_ecm<targ->GetImageInformation().ecm){
				Target (NULL);//go wild
			}else if (hull>0){
				static unsigned int pointdef = ROLES::getRole("POINTDEF");
				targ->graphicOptions.missilelock=true;
				un_iter i = targ->getSubUnits();
				
				Unit * su;
				for (;(su =*i)!=NULL;++i) {
                                  if (su->attackPreference()==pointdef) {
                                    if (su->Target()==NULL) {
                                      
                                      float speed,range,mrange;
                                      su->getAverageGunSpeed(speed,range,mrange);
                                      if ((Position()-su->Position()).MagnitudeSquared()<range*range) {
                                        su->Target(this);
                                        su->TargetTurret(this);
                                        //VSFileSystem::vs_fprintf (stderr,"%f targets %s",mission->getGametime(),targ->name.c_str());
                                        
                                      }
                                      
                                    }
                                  }
				}
			}
		}
        }
        if (retarget==-1){
          if (targ) {
            retarget=1;
          }else {
            retarget=0;
          }
        }
        if (retarget&&targ==NULL) {
          Target (NULL);// BROKEN
        }
        if (had_target&&!(Unit::Target())) {
          static float max_lost_target_live_time=XMLSupport::parse_float(vs_config->getVariable("physics","max_lost_target_live_time","30"));
          if (time>max_lost_target_live_time) {
            time=max_lost_target_live_time;
          }
        }
	Unit::UpdatePhysics2 (trans, old_physical_state, accel, difficulty,transmat, CumulativeVelocity, ResolveLast, uc);
        this->time-=SIMULATION_ATOM;
        if (NULL!=targ) {
          float checker = targ->querySphere (Position()-(SIMULATION_ATOM*GetVelocity()),Position(),rSize());
          if ((checker&&detonation_radius>=0)||((Position()-targ->Position()).Magnitude()-targ->rSize()-rSize()<detonation_radius)) {
            // spiritplumber assumes that the missile is hitting a much larger object than itself
            static float percent_missile_match_target_velocity=XMLSupport::parse_float(vs_config->getVariable("physics","percent_missile_match_target_velocity",".5"));                  
            
            this->Velocity += percent_missile_match_target_velocity*(targ->Velocity-this->Velocity);                
            Discharge();
            time=-1;
            //Vector norm;
            //float dist;
            /*** WARNING COLLISION STUFF... TO FIX FOR SERVER SIDE SOMEDAY ***
                 if ((targ)->queryBoundingBox (Position(),detonation_radius+rSize())) {
                 Discharge();
                 time=-1;
                 }
            */
          }
        }
        if (time<0) {
          DealDamageToHull (Vector(.1,.1,.1),hull+1);
        }
}
