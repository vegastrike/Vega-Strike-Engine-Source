/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "gfx/mesh.h"
#include "unit.h"
#include "lin_time.h"
//#include "physics.h"
#include "beam.h"
#include "planet.h"
#include "audiolib.h"
#include "images.h"
#include "config_xml.h"
#include "vs_globals.h"
//#ifdef WIN32
#include "gfx/planetary_transform.h"
#include "gfx/cockpit.h"
float copysign (float x, float y) {
	if (y>0)
			return x;
	else
			return -x;
}
//#endif

// the rotation should be applied in world coordinates
void Unit:: Rotate (const Vector &axis)
{
	float theta = axis.Magnitude();
	float ootheta = 1/theta;
	float s = cos (theta * .5);
	Quaternion rot = Quaternion(s, axis * (sinf (theta*.5)*ootheta));
	if(theta < 0.0001) {
	  rot = identity_quaternion;
	}
	curr_physical_state.orientation *= rot;
	if (limits.limitmin>-1) {
	  Matrix mat;
	  curr_physical_state.orientation.to_matrix (mat);
	  if (limits.structurelimits.Dot (mat.getR())<limits.limitmin) {
	    curr_physical_state.orientation=prev_physical_state.orientation;
	  }
	}
}

void Unit:: FireEngines (const Vector &Direction/*unit vector... might default to "r"*/,
					float FuelSpeed,
					float FMass)
{
	mass -= FMass; //fuel is sent out
	fuel -= FMass;
	if (fuel <0)
	{
		
		FMass +=fuel;
		mass -= fuel;
		fuel = 0; //ha ha!
	}
	NetForce += Direction *(FuelSpeed *FMass/GetElapsedTime());
}
void Unit::ApplyForce(const Vector &Vforce) //applies a force for the whole gameturn upon the center of mass
{
	NetForce += Vforce;
}
void Unit::ApplyLocalForce(const Vector &Vforce) //applies a force for the whole gameturn upon the center of mass
{
	NetLocalForce += Vforce;
}
void Unit::Accelerate(const Vector &Vforce)
{
  NetForce += Vforce * mass;
}

void Unit::ApplyTorque (const Vector &Vforce, const QVector &Location)
{
  //Not completely correct
	NetForce += Vforce;
	NetTorque += Vforce.Cross ((Location-curr_physical_state.position).Cast());
}
void Unit::ApplyLocalTorque (const Vector &Vforce, const Vector &Location)
{
	NetForce += Vforce;
	NetTorque += Vforce.Cross (Location);
}
void Unit::ApplyBalancedLocalTorque (const Vector &Vforce, const Vector &Location) //usually from thrusters remember if I have 2 balanced thrusters I should multiply their effect by 2 :)
{
	NetTorque += Vforce.Cross (Location);
	
}

void Unit::ApplyLocalTorque(const Vector &torque) {
  /*  Vector p,q,r;
  Vector tmp(ClampTorque(torque));
  GetOrientation (p,q,r);
  fprintf (stderr,"P: %f,%f,%f Q: %f,%f,%f",p.i,p.j,p.k,q.i,q.j,q.k);
  NetTorque+=tmp.i*p+tmp.j*q+tmp.k*r; 
  */
  NetLocalTorque+= ClampTorque(torque); 
}

Vector Unit::MaxTorque(const Vector &torque) {
  // torque is a normal
  return torque * (Vector(copysign(limits.pitch, torque.i), 
			  copysign(limits.yaw, torque.j),
			  copysign(limits.roll, torque.k)) * torque);
}

/** MISNOMER...not really clamping... more like renomalizing  slow too
Vector Unit::ClampTorque(const Vector &amt1) {
  Vector norm = amt1;
  norm.Normalize();
  Vector max = MaxTorque(norm);

  if(max.Magnitude() > amt1.Magnitude())
    return amt1;
  else 
    return max;
}
*/
//FIXME 062201
extern unsigned short apply_float_to_short (float tmp);
Vector Unit::ClampTorque (const Vector &amt1) {
  Vector Res=amt1;
  static float staticfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelThrust",".9"));
  float fuelclamp=(fuel<=0)?staticfuelclamp:1;
  if (fabs(amt1.i)>fuelclamp*limits.pitch)
    Res.i=copysign(fuelclamp*limits.pitch,amt1.i);
  if (fabs(amt1.j)>fuelclamp*limits.yaw)
    Res.j=copysign(fuelclamp*limits.yaw,amt1.j);
  if (fabs(amt1.k)>fuelclamp*limits.roll)
    Res.k=copysign(fuelclamp*limits.roll,amt1.k);
  fuel-=Res.Magnitude()*SIMULATION_ATOM;
  return Res;
}
//    float max_speed;
//    float max_ab_speed;
//    float max_yaw;
//    float max_pitch;
//    float max_roll;

Vector Unit::ClampVelocity (const Vector & velocity, const bool afterburn) {
  static float staticfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelThrust",".9"));
  static float staticabfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelAfterburn",".1"));
  float fuelclamp=(fuel<=0)?staticfuelclamp:1;
  float abfuelclamp= (fuel<=0||(energy<afterburnenergy))?staticabfuelclamp:1;
  float limit = afterburn?(abfuelclamp*(computer.max_ab_speed-computer.max_speed)+(fuelclamp*computer.max_speed)):fuelclamp*computer.max_speed;
  float tmp = velocity.Magnitude();
  if (tmp>fabs(limit)) {
    return velocity * (limit/tmp);
  }
  return velocity;
}


Vector Unit::ClampAngVel (const Vector & velocity) {
  Vector res (velocity);
  if (fabs (res.i)>computer.max_pitch) {
    res.i = copysign (computer.max_pitch,res.i);
  }
  if (fabs (res.j)>computer.max_yaw) {
    res.j = copysign (computer.max_yaw,res.j);
  }
  if (fabs (res.k)>computer.max_roll) {
    res.k = copysign (computer.max_roll,res.k);
  }
  return res;
}


Vector Unit::MaxThrust(const Vector &amt1) {
  // amt1 is a normal
  return amt1 * (Vector(copysign(limits.lateral, amt1.i), 
	       copysign(limits.vertical, amt1.j),
	       amt1.k>0?limits.forward:-limits.retro) * amt1);
}
/* misnomer..this doesn't get the max value of each axis
Vector Unit::ClampThrust(const Vector &amt1){ 
  // Yes, this can be a lot faster with LUT
  Vector norm = amt1;
  norm.Normalize();
  Vector max = MaxThrust(norm);

  if(max.Magnitude() > amt1.Magnitude())
    return amt1;
  else 
    return max;
}
*/
//CMD_FLYBYWIRE depends on new version of Clampthrust... don't change without resolving it

Vector Unit::ClampThrust (const Vector &amt1, bool afterburn) {
  Vector Res=amt1;
  if (energy<afterburnenergy) {
    afterburn=false;
  }
  if (afterburn) {
    energy -=apply_float_to_short( afterburnenergy*SIMULATION_ATOM);
  }


  static float staticfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelThrust",".4"));
  static float staticabfuelclamp = XMLSupport::parse_float (vs_config->getVariable ("physics","NoFuelAfterburn","0"));
  static float abfuelusage = XMLSupport::parse_float (vs_config->getVariable ("physics","AfterburnerFuelUsage","4"));
  float fuelclamp=(fuel<=0)?staticfuelclamp:1;
  float abfuelclamp= (fuel<=0)?staticabfuelclamp:1;
  if (fabs(amt1.i)>fabs(fuelclamp*limits.lateral))
    Res.i=copysign(fuelclamp*limits.lateral,amt1.i);
  if (fabs(amt1.j)>fabs(fuelclamp*limits.vertical))
    Res.j=copysign(fuelclamp*limits.vertical,amt1.j);
  float ablimit =       
    afterburn
    ?((limits.afterburn-limits.forward)*abfuelclamp+limits.forward*fuelclamp)
    :limits.forward;

  if (amt1.k>ablimit)
    Res.k=ablimit;
  if (amt1.k<-limits.retro)
    Res.k =-limits.retro;
  fuel-=(afterburn?abfuelusage:1)*Res.Magnitude();
  return Res;
}


void Unit::Thrust(const Vector &amt1,bool afterburn){
  Vector amt = ClampThrust(amt1,afterburn);
  ApplyLocalForce(amt);
  if (afterburn!=AUDIsPlaying (sound->engine)) {
    if (afterburn)
      AUDPlay (sound->engine,cumulative_transformation.position,cumulative_velocity,1);
    else
      //    if (Velocity.Magnitude()<computer.max_speed)
      AUDStopPlaying (sound->engine);
    
  }
}

void Unit::LateralThrust(float amt) {
  if(amt>1.0) amt = 1.0;
  if(amt<-1.0) amt = -1.0;
  ApplyLocalForce(amt*limits.lateral * Vector(1,0,0));
}

void Unit::VerticalThrust(float amt) {
  if(amt>1.0) amt = 1.0;
  if(amt<-1.0) amt = -1.0;
  ApplyLocalForce(amt*limits.vertical * Vector(0,1,0));
}

void Unit::LongitudinalThrust(float amt) {
  if(amt>1.0) amt = 1.0;
  if(amt<-1.0) amt = -1.0;
  ApplyLocalForce(amt*limits.forward * Vector(0,0,1));
}

void Unit::YawTorque(float amt) {
  if(amt>limits.yaw) amt = limits.yaw;
  else if(amt<-limits.yaw) amt = -limits.yaw;
  ApplyLocalTorque(amt * Vector(0,1,0));
}

void Unit::PitchTorque(float amt) {
  if(amt>limits.pitch) amt = limits.pitch;
  else if(amt<-limits.pitch) amt = -limits.pitch;
  ApplyLocalTorque(amt * Vector(1,0,0));
}

void Unit::RollTorque(float amt) {
  if(amt>limits.roll) amt = limits.roll;
  else if(amt<-limits.roll) amt = -limits.roll;
  ApplyLocalTorque(amt * Vector(0,0,1));
}
static int applyto (unsigned short &shield, const unsigned short max, const float amt) {
  shield+=apply_float_to_short(amt);
  if (shield>max)
    shield=max;
  return (shield>=max)?1:0;
}

float Unit::MaxShieldVal() const{
  float maxshield;
  switch (shield.number) {
  case 2:
    maxshield = .5*(shield.fb[2]+shield.fb[3]);
    break;
  case 4:
    maxshield = .25*(float(shield.fbrl.frontmax)+shield.fbrl.backmax+shield.fbrl.leftmax+shield.fbrl.rightmax);
    break;
  case 6:
    maxshield = .25*float(shield.fbrltb.fbmax)+.75*shield.fbrltb.rltbmax;
    break;
  }
  return maxshield;
}
void Unit::RechargeEnergy() {
    energy +=apply_float_to_short (recharge *SIMULATION_ATOM);
}
void Unit::RegenShields () {
  int rechargesh=1;
  float maxshield=MaxShieldVal();
  static bool energy_before_shield=XMLSupport::parse_bool(vs_config->getVariable ("physics","engine_energy_priority","true"));
  if (!energy_before_shield) {
    RechargeEnergy();
  }
  float rec = shield.recharge*SIMULATION_ATOM>energy?energy:shield.recharge*SIMULATION_ATOM;
  if (_Universe->isPlayerStarship(this)==NULL) {
    rec*=g_game.difficulty;
  }else {
    rec*=g_game.difficulty;//sqrtf(g_game.difficulty);
  }
  if ((image->ecm>0)) {
    static float ecmadj = XMLSupport::parse_float(vs_config->getVariable ("physics","ecm_energy_cost",".05"));
    float sim_atom_ecm = ecmadj * image->ecm*SIMULATION_ATOM;
    if (energy-10>sim_atom_ecm) {
      energy-=sim_atom_ecm;
    }else {
      energy=energy<10?energy:10;
    }
  }
  if (GetNebula()!=NULL) {
    static float nebshields=XMLSupport::parse_float(vs_config->getVariable ("physics","nebula_shield_recharge",".5"));
    rec *=nebshields;
  }
  switch (shield.number) {
  case 2:

    shield.fb[0]+=rec;
    shield.fb[1]+=rec;
    if (shield.fb[0]>shield.fb[2]) {
      shield.fb[0]=shield.fb[2];
    } else {
      rechargesh=0;
    }
    if (shield.fb[1]>shield.fb[3]) {
      shield.fb[1]=shield.fb[3];

    } else {
      rechargesh=0;
    }
    break;
  case 4:

    rechargesh = applyto (shield.fbrl.front,shield.fbrl.frontmax,rec)*(applyto (shield.fbrl.back,shield.fbrl.backmax,rec))*applyto (shield.fbrl.right,shield.fbrl.rightmax,rec)*applyto (shield.fbrl.left,shield.fbrl.leftmax,rec);
    break;
  case 6:
    rechargesh = (applyto(shield.fbrltb.v[0],shield.fbrltb.fbmax,rec))*applyto(shield.fbrltb.v[1],shield.fbrltb.fbmax,rec)*applyto(shield.fbrltb.v[2],shield.fbrltb.rltbmax,rec)*applyto(shield.fbrltb.v[3],shield.fbrltb.rltbmax,rec)*applyto(shield.fbrltb.v[4],shield.fbrltb.rltbmax,rec)*applyto(shield.fbrltb.v[5],shield.fbrltb.rltbmax,rec);
    break;
  }
  if (rechargesh==0)
    energy-=(short unsigned int)rec;
  if (energy_before_shield) {
    RechargeEnergy();
  }
  if (maxenergy>maxshield) {
    if (energy>maxenergy-maxshield)//allow shields to absorb xtra power
      energy=maxenergy-maxshield;  
  }else {
    energy=0;
  }

}
Cockpit * Unit::GetVelocityDifficultyMult(float &difficulty) const{
  difficulty=1;
  Cockpit * player_cockpit=_Universe->isPlayerStarship(this);
  if ((player_cockpit)==NULL) {
    static float exp = XMLSupport::parse_float (vs_config->getVariable ("physics","difficulty_speed_exponent",".2"));
    difficulty = pow(g_game.difficulty,exp);
  }
  return player_cockpit;
}
void Unit::UpdatePhysics (const Transformation &trans, const Matrix &transmat, const Vector & cum_vel,  bool lastframe, UnitCollection *uc) {
  static float VELOCITY_MAX=XMLSupport::parse_float(vs_config->getVariable ("physics","velocity_max","10000"));
  if (docked&DOCKING_UNITS) {
    PerformDockingOperations();
  }
  Repair();
  if (fuel<0)
    fuel=0;
  if (cloaking>=cloakmin) {
    if (image->cloakenergy*SIMULATION_ATOM>energy) {
      Cloak(false);//Decloak
    } else {
      if (image->cloakrate>0||cloaking==cloakmin) {
	energy-=apply_float_to_short(SIMULATION_ATOM*image->cloakenergy);
      }
      if (cloaking>cloakmin) {
	AUDAdjustSound (sound->cloak, cumulative_transformation.position,cumulative_velocity);
	if ((cloaking==32767&&image->cloakrate>0)||(cloaking==cloakmin+1&&image->cloakrate<0)) {
	  AUDStartPlaying (sound->cloak);
	}
	cloaking-=image->cloakrate*SIMULATION_ATOM;
	if (cloaking<=cloakmin&&image->cloakrate>0) {
	  //AUDStopPlaying (sound->cloak);
	  cloaking=cloakmin;
	}
	if (cloaking<0&&image->cloakrate<0) {
	  //AUDStopPlaying (sound->cloak);
	  cloaking=(short)32768;//wraps
	}
      }
    }
  }

  RegenShields();
  if (lastframe) {
    if (!(docked&(DOCKED|DOCKED_INSIDE))) 
      prev_physical_state = curr_physical_state;//the AIscript should take care
    if (planet) {
      if (!planet->dirty) {
	SetPlanetOrbitData (NULL);
      }else {
	planet->pps = planet->cps;
      }
    }
  }
  if (isUnit()==PLANETPTR) {
    ((Planet *)this)->gravitate (uc);
  } else {
    if (resolveforces) {
      ResolveForces (trans,transmat);//clamp velocity
      if (fabs (Velocity.i)>VELOCITY_MAX) {
	Velocity.i = copysign (VELOCITY_MAX,Velocity.i);
      }
      if (fabs (Velocity.j)>VELOCITY_MAX) {
	Velocity.j = copysign (VELOCITY_MAX,Velocity.j);
      }
      if (fabs (Velocity.k)>VELOCITY_MAX) {
	Velocity.k = copysign (VELOCITY_MAX,Velocity.k);
      }
    }
  } 
  if(AngularVelocity.i||AngularVelocity.j||AngularVelocity.k) {
    Rotate (SIMULATION_ATOM*(AngularVelocity));
  }
  float difficulty;
  Cockpit * player_cockpit=GetVelocityDifficultyMult (difficulty);

  curr_physical_state.position = curr_physical_state.position +  (Velocity*SIMULATION_ATOM*difficulty).Cast();
#ifdef DEPRECATEDPLANETSTUFF
  if (planet) {
    Matrix basis;
    curr_physical_state.to_matrix (cumulative_transformation_matrix);
    Vector p,q,r,c;
    MatrixToVectors (cumulative_transformation_matrix,p,q,r,c);
    planet->trans->InvTransformBasis (cumulative_transformation_matrix,p,q,r,c);
    planet->cps=Transformation::from_matrix (cumulative_transformation_matrix);
  }
#endif
  cumulative_transformation = curr_physical_state;
  cumulative_transformation.Compose (trans,transmat);
  cumulative_transformation.to_matrix (cumulative_transformation_matrix);
  cumulative_velocity = TransformNormal (transmat,Velocity)+cum_vel;


  Transformation * ct;
  Matrix * ctm=NULL;
  SetPlanetHackTransformation (ct,ctm);
  int i;
  if (lastframe) {
    char tmp=0;
    for (i=0;i<=nummesh;i++) {
      if (!meshdata[i])
	continue;
      tmp |=meshdata[i]->HasBeenDrawn();
      if (!meshdata[i]->HasBeenDrawn()) {
	meshdata[i]->UpdateFX(SIMULATION_ATOM);
      }
      meshdata[i]->UnDraw();
    }
    if (!tmp&&hull<0) {
      Explode(false,SIMULATION_ATOM);
	
    }
  }      
  Unit * target = Target();
  bool increase_locking=false;
  if (target&&cloaking<0/*-1 or -32768*/) {
    if (target->isUnit()!=PLANETPTR) {
      Vector TargetPos (ToLocalCoordinates ((target->Position()-Position()).Cast())); 
      TargetPos.Normalize(); 
      if (TargetPos.Dot(Vector(0,0,1))>computer.radar.lockcone) {
	increase_locking=true;
      }
    }
  }
  static string LockingSoundName = vs_config->getVariable ("unitaudio","locking","locking.wav");
  static int LockingSound = AUDCreateSoundWAV (LockingSoundName,true);
  bool locking=false;
  bool touched=false;
  for (i=0;i<nummounts;i++) {
//    if (increase_locking&&cloaking<0) {
//      mounts[i].time_to_lock-=SIMULATION_ATOM;
//    }
    if (mounts[i].status==Mount::ACTIVE&&cloaking<0&&mounts[i].ammo!=0) {
      if (player_cockpit) {
	  touched=true;
      }
      if (increase_locking) {
	mounts[i].time_to_lock-=SIMULATION_ATOM;
	static bool ai_lock_cheat=XMLSupport::parse_bool(vs_config->getVariable ("physics","ai_lock_cheat","true"));	
	if (!player_cockpit) {
	  if (ai_lock_cheat) {
	    mounts[i].time_to_lock=-1;
	  }
	}else {

	  if (mounts[i].type->LockTime>0) {
	    static string LockedSoundName= vs_config->getVariable ("unitaudio","locked","locked.wav");
	    static int LockedSound = AUDCreateSoundWAV (LockedSoundName,false);

	    if (mounts[i].time_to_lock>-SIMULATION_ATOM&&mounts[i].time_to_lock<=0) {
	      if (!AUDIsPlaying(LockedSound)) {
		AUDStartPlaying(LockedSound);
		AUDStopPlaying(LockingSound);	      
	      }
	      AUDAdjustSound (LockedSound,Position(),GetVelocity()); 
	    }else if (mounts[i].time_to_lock>0)  {
	      locking=true;
	      if (!AUDIsPlaying(LockingSound)) {

		AUDStartPlaying(LockingSound);	      
		

	      }
	      AUDAdjustSound (LockingSound,Position(),GetVelocity());

	    }
	  }

	}
      
      }else {
        if (mounts[i].ammo!=0) {
	  mounts[i].time_to_lock=mounts[i].type->LockTime;
        }
      }
    } else {
      if (mounts[i].ammo!=0) {
        mounts[i].time_to_lock=mounts[i].type->LockTime;
      }
    }
    if (mounts[i].type->type==weapon_info::BEAM) {
      if (mounts[i].ref.gun) {
	mounts[i].ref.gun->UpdatePhysics (cumulative_transformation, cumulative_transformation_matrix);
      }
    } else {
      mounts[i].ref.refire+=SIMULATION_ATOM;
    }
    if (mounts[i].processed==Mount::FIRED) {
      Transformation t1;
      Matrix m1;
      t1=prev_physical_state;//a hack that will not work on turrets
      t1.Compose (trans,transmat);
      t1.to_matrix (m1);
      int autotrack=0;
      if ((0!=(mounts[i].size&weapon_info::AUTOTRACKING))) {
	autotrack = computer.itts?2:1;
      }
      mounts[i].PhysicsAlignedFire (t1,m1,cumulative_velocity,owner==NULL?this:owner,target,autotrack, computer.radar.trackingcone);
      if (mounts[i].ammo==0&&mounts[i].type->type==weapon_info::PROJECTILE) {
	ToggleWeapon (true);
      }
    }else if (mounts[i].processed==Mount::UNFIRED) {
      mounts[i].PhysicsAlignedUnfire();
    }
  }
  if (locking==false&&touched==true) {
    if (AUDIsPlaying(LockingSound)) {
      AUDStopPlaying(LockingSound);	
    }      
  }
  bool dead=true;

  if (!SubUnits.empty()) {
    Unit * su;
    UnitCollection::UnitIterator iter=getSubUnits();
    while ((su=iter.current())) {
      su->UpdatePhysics(cumulative_transformation,cumulative_transformation_matrix,cumulative_velocity,lastframe,uc); 
      su->cloaking = (short unsigned int) cloaking;
      if (hull<0) {
	UnFire();//don't want to go off shooting while your body's splitting everywhere
	su->hull-=SIMULATION_ATOM;
      }
      iter.advance();
      //    dead &=(subunits[i]->hull<0);
    }
  }
  if (hull<0) {
    dead&= (image->explosion==NULL);    
    if (dead)
      Kill();
  }
  if ((!SubUnit)&&(!killed)&&(!(docked&DOCKED_INSIDE))) {
    UpdateCollideQueue();
  }
}
void Unit::SetPlanetOrbitData (PlanetaryTransform *t) {
  if (isUnit()!=BUILDINGPTR)
        return;
  if (!planet)
    planet = (PlanetaryOrbitData *)malloc (sizeof (PlanetaryOrbitData));
  else if (!t) {
    free (planet);
    planet=NULL;
  }
  if (t) {
    planet->trans = t;
    planet->dirty=true;
  }
}
PlanetaryTransform * Unit::GetPlanetOrbit () const {
  if (planet==NULL)
    return NULL;
  return planet->trans;
}

bool Unit::jumpReactToCollision (Unit * smalle) {
  if (!GetDestinations().empty()) {//only allow big with small
    if ((smalle->GetJumpStatus().drive>=0||image->forcejump)) {
      smalle->DeactivateJumpDrive();
      Unit * jumppoint = this;
      _Universe->activeStarSystem()->JumpTo (smalle, jumppoint, std::string(GetDestinations()[smalle->GetJumpStatus().drive%GetDestinations().size()]));
      return true;
    }
    return true;
  }
  if (!smalle->GetDestinations().empty()) {
    if ((GetJumpStatus().drive>=0||smalle->image->forcejump)) {
      DeactivateJumpDrive();
      Unit * jumppoint = smalle;
      _Universe->activeStarSystem()->JumpTo (this, jumppoint, std::string(smalle->GetDestinations()[GetJumpStatus().drive%smalle->GetDestinations().size()]));
      return true;
    }
    return true;
  }

  return false;
}
void Unit::reactToCollision(Unit * smalle, const QVector & biglocation, const Vector & bignormal, const QVector & smalllocation, const Vector & smallnormal,  float dist) {
  clsptr smltyp = smalle->isUnit();
  if (smltyp==ENHANCEMENTPTR||smltyp==MISSILEPTR) {
    if (isUnit()!=ENHANCEMENTPTR&&isUnit()!=MISSILEPTR) {
      smalle->reactToCollision (this,smalllocation,smallnormal,biglocation,bignormal,dist);
      return;
    }
  }	       
  //don't bounce if you can Juuuuuuuuuuuuuump
  if (!jumpReactToCollision(smalle)) {
#ifdef NOBOUNCECOLLISION
#else
    static float bouncepercent = XMLSupport::parse_float (vs_config->getVariable ("physics","BouncePercent",".1"));
    smalle->ApplyForce (bignormal*.4*bouncepercent*smalle->GetMass()*fabs(bignormal.Dot (((smalle->GetVelocity()-this->GetVelocity())/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM)));
    this->ApplyForce (smallnormal*.4*bouncepercent*(smalle->GetMass()*smalle->GetMass()/this->GetMass())*fabs(smallnormal.Dot ((smalle->GetVelocity()-this->GetVelocity()/SIMULATION_ATOM))+fabs (dist)/(SIMULATION_ATOM*SIMULATION_ATOM)));
    
    smalle->ApplyDamage (biglocation.Cast(),bignormal,g_game.difficulty*(  .5*fabs(bignormal.Dot(smalle->GetVelocity()-this->GetVelocity()))*this->mass*SIMULATION_ATOM),smalle,GFXColor(1,1,1,2),NULL);
    this->ApplyDamage (smalllocation.Cast(),smallnormal, g_game.difficulty*(.5*fabs(smallnormal.Dot(smalle->GetVelocity()-this->GetVelocity()))*smalle->mass*SIMULATION_ATOM),this,GFXColor(1,1,1,2),NULL);
#endif
    
  //each mesh with each mesh? naw that should be in one way collide
  }
}



void Unit::ResolveForces (const Transformation &trans, const Matrix &transmat) {
  Vector p, q, r;
  GetOrientation(p,q,r);
  Vector temp = (InvTransformNormal(transmat,NetTorque)+NetLocalTorque.i*p+NetLocalTorque.j*q+NetLocalTorque.k *r)*SIMULATION_ATOM*(1.0/MomentOfInertia);
  if (FINITE(temp.i)&&FINITE (temp.j)&&FINITE(temp.k)) {
    AngularVelocity += temp;
  }
  temp = ((InvTransformNormal(transmat,NetForce) + NetLocalForce.i*p + NetLocalForce.j*q + NetLocalForce.k*r ) * SIMULATION_ATOM)/mass; //acceleration
  if (FINITE(temp.i)&&FINITE (temp.j)&&FINITE(temp.k)) {	//FIXME
    Velocity += temp;
  } 
#ifndef PERFRAMESOUND
  AUDAdjustSound (sound->engine,cumulative_transformation.position, cumulative_velocity); 
#endif
  NetForce = NetLocalForce = NetTorque = NetLocalTorque = Vector(0,0,0);

  /*
    if (fabs (Velocity.i)+fabs(Velocity.j)+fabs(Velocity.k)> co10) {
    float magvel = Velocity.Magnitude(); float y = (1-magvel*magvel*oocc);
    temp = temp * powf (y,1.5);
    }*/
}
void Unit::SetOrientation (QVector q, QVector r) {
  q.Normalize();
  r.Normalize();
  QVector p;
  CrossProduct (q,r,p);
  CrossProduct (r,p,q);
  curr_physical_state = Transformation (Quaternion::from_vectors (p.Cast(),q.Cast(),r.Cast()),Position());
}
void Unit::GetOrientation(Vector &p, Vector &q, Vector &r) const {
  Matrix m;
  curr_physical_state.to_matrix(m);
  p=m.getP();
  q=m.getQ();
  r=m.getR();
}

Vector Unit::UpCoordinateLevel (const Vector &v) const {
  Matrix m;
  curr_physical_state.to_matrix(m);
#define M(A,B) m.r[B*3+A]
  return Vector(v.i*M(0,0)+v.j*M(1,0)+v.k*M(2,0),
		v.i*M(0,1)+v.j*M(1,1)+v.k*M(2,1),
		v.i*M(0,2)+v.j*M(1,2)+v.k*M(2,2));
#undef M
}

Vector Unit::ToLocalCoordinates(const Vector &v) const {
  //Matrix m;
  //062201: not a cumulative transformation...in prev unit space  curr_physical_state.to_matrix(m);
  
#define M(A,B) cumulative_transformation_matrix.r[B*3+A]
  return Vector(v.i*M(0,0)+v.j*M(1,0)+v.k*M(2,0),
		v.i*M(0,1)+v.j*M(1,1)+v.k*M(2,1),
		v.i*M(0,2)+v.j*M(1,2)+v.k*M(2,2));
#undef M
}

Vector Unit::ToWorldCoordinates(const Vector &v) const {
  return TransformNormal(cumulative_transformation_matrix,v); 
#undef M

}
static float getAutoRSize (Unit * orig,Unit * un, bool ignore_friend=false) {
  static float friendly_autodist =  XMLSupport::parse_float (vs_config->getVariable ("physics","friendly_auto_radius","100"));
  static float neutral_autodist =  XMLSupport::parse_float (vs_config->getVariable ("physics","neutral_auto_radius","1000"));
  static float hostile_autodist =  XMLSupport::parse_float (vs_config->getVariable ("physics","hostile_auto_radius","8000"));
  static int upgradefaction = _Universe->GetFaction("upgrades");
  static int neutral = _Universe->GetFaction("neutral");

  if (un->isUnit()==PLANETPTR||(un->getFlightgroup()==orig->getFlightgroup()&&orig->getFlightgroup())) {
    //same flihgtgroup
    return orig->rSize();
  }
  if (un->faction==upgradefaction) {
    return ignore_friend?-FLT_MAX:(-orig->rSize()-un->rSize());
  }
  float rel=_Universe->GetRelation(un->faction,orig->faction);
  if (orig == un->Target())
	rel-=1.5;
  if (rel>.1||un->faction==neutral) {
	  return ignore_friend?-FLT_MAX:friendly_autodist;//min distance apart
  }else if (rel<-.1) {
    
    return hostile_autodist;
  }else {
	  return ignore_friend?-FLT_MAX:neutral_autodist;
  }
}
bool Unit::AutoPilotTo (Unit * target, bool ignore_friendlies) {
  static float autopilot_term_distance = XMLSupport::parse_float (vs_config->getVariable ("physics","auto_pilot_termination_distance","6000"));
//  static float autopilot_p_term_distance = XMLSupport::parse_float (vs_config->getVariable ("physics","auto_pilot_planet_termination_distance","60000"));
  static float planet_rad_percent =  XMLSupport::parse_float (vs_config->getVariable ("physics","auto_pilot_planet_radius_percent",".75"));
  if (SubUnit) {
    return false;//we can't auto here;
  }
  StarSystem * ss = activeStarSystem;
  if (ss==NULL) {
    ss = _Universe->activeStarSystem();
  }
  Unit * un=NULL;
  QVector start (Position());
  QVector end (target->LocalPosition());
  float totallength = (start-end).Magnitude();
  if (totallength>1) {
    //    float apt = (target->isUnit()==PLANETPTR&&target->GetDestinations().empty())?autopilot_p_term_distance:autopilot_term_distance;
    float apt = (target->isUnit()==PLANETPTR)?(autopilot_term_distance+target->rSize()*planet_rad_percent):autopilot_term_distance;
    float percent = (getAutoRSize(this,this)+rSize()+target->rSize()+apt)/totallength;
    if (percent>1) {
      end=start;
    }else {
      end = start*percent+end*(1-percent);
    }
  }
  bool ok=true;
  for (un_iter i=ss->getUnitList().createIterator();
       (un=*i)!=NULL; 
       ++i) {
    if (un->isUnit()!=NEBULAPTR) {
     
    if (un!=this&&un!=target) {
      if ((start-un->Position()).Magnitude()-getAutoRSize (this,this,ignore_friendlies)-rSize()-un->rSize()-getAutoRSize(this,un,ignore_friendlies)<=0) {
	return false;
      }
      float intersection = un->querySphere (start,end,getAutoRSize (this,un,ignore_friendlies));
      if (intersection>0) {
	end = start+ (end-start)*intersection;
	ok=false;
      }
    }
    }
  }
  if (this!=target) {
    SetCurPosition(end);
    if (_Universe->isPlayerStarship (this)&&getFlightgroup()!=NULL) {
      Unit * other=NULL;
      for (un_iter ui=ss->getUnitList().createIterator();
	   NULL!=(other = *ui);
	   ++ui) {
	if (other->getFlightgroup()==getFlightgroup()) {
	  if (NULL==_Universe->isPlayerStarship (other)) {
	    other->AutoPilotTo(this);
	  }
	}
      }
    }
  }
  return ok;
}
