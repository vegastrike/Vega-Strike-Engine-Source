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
//#include "unit.h"
#include "lin_time.h"
//#include "physics.h"
#include "beam.h"
#include "planet.h"
#include "audiolib.h"
#include "config_xml.h"
#include "vs_globals.h"
//#ifdef WIN32
#ifdef FIX_TERRAIN
#include "gfx/planetary_transform.h"
#endif
#include "gfx/cockpit.h"
#include "unit_util.h"
#include "universe_util.h"
#include "cmd/script/mission.h"
#include "networking/vsnet_clientstate.h"
#include "networking/netclient.h"
//#endif
extern float copysign (float x, float y);

// the rotation should be applied in world coordinates
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
//    float max_speed;
//    float max_ab_speed;
//    float max_yaw;
//    float max_pitch;
//    float max_roll;

template <class UnitType>
void GameUnit<UnitType>::UpdatePhysics (const Transformation &trans, const Matrix &transmat, const Vector & cum_vel,  bool lastframe, UnitCollection *uc) {
  static float VELOCITY_MAX=XMLSupport::parse_float(vs_config->getVariable ("physics","velocity_max","10000"));

	int player = -1;
	Vector accel;
	Transformation old_physical_state = curr_physical_state;
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
	cloaking-= (short)(image->cloakrate*SIMULATION_ATOM);
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
#ifdef FIX_TERRAIN
    if (planet) {
      if (!planet->dirty) {
	SetPlanetOrbitData (NULL);
      }else {
	planet->pps = planet->cps;
      }
    }
#endif
  }

  if (isUnit()==PLANETPTR) {
    ((Planet *)this)->gravitate (uc);
  } else {
    if (resolveforces) {
      accel = ResolveForces (trans,transmat);//clamp velocity
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

  // Here send (new position + direction = curr_physical_state.position and .orientation)
  // + speed to server (which velocity is to consider ?)
  // + maybe Angular velocity to anticipate rotations in the other network clients
  if( Network!=NULL && Network[0].isTime())
  {
	  //cout<<"SEND UPDATE"<<endl;
	  //this->networked=1;
	  // Check if this is a player, because in network mode we should only send updates of our moves
	  player= _Universe->whichPlayerStarship( this);
	  if( player>=0 /* && this->networked */ )
	  {
		  cout<<"Player number : "<<player<<endl;
		  // (NetForce + Transform (ship_matrix,NetLocalForce) )/mass = GLOBAL ACCELERATION
		  curr_physical_state.position = curr_physical_state.position +  (Velocity*SIMULATION_ATOM*difficulty).Cast();
		  // If we want to inter(extra)polate sent position, DO IT HERE
		  if( !(old_physical_state.position == curr_physical_state.position && old_physical_state.orientation == curr_physical_state.orientation))
				// We moved so update
		  {
				ClientState cstmp( Network[player].getSerial(), curr_physical_state, Velocity, accel, 0);
				Network[player].sendPosition( &cstmp);
		  }
		  else
				// Say we are still alive
				Network[player].sendAlive();
	  }
	  else
		  // Not the player so update the unit's position and stuff with the last received snapshot from the server
			;
  }
  else
  {
	  //this->networked++;
 	 curr_physical_state.position = curr_physical_state.position +  (Velocity*SIMULATION_ATOM*difficulty).Cast();
  }

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
  unsigned int i;
  if (lastframe) {
    char tmp=0;
    for (i=0;i<meshdata.size();i++) {
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
  float dist_sqr_to_target=FLT_MAX;
  Unit * target = Unit::Target();
  bool increase_locking=false;
  if (target&&cloaking<0/*-1 or -32768*/) {
    if (target->isUnit()!=PLANETPTR) {
      Vector TargetPos (InvTransform (cumulative_transformation_matrix,(target->Position()).Cast())); 
      dist_sqr_to_target = TargetPos.MagnitudeSquared(); 
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

  for (i=0;(int)i<GetNumMounts();i++) {
//    if (increase_locking&&cloaking<0) {
//      mounts[i]->time_to_lock-=SIMULATION_ATOM;
//    }
    if (mounts[i]->status==Mount::ACTIVE&&cloaking<0&&mounts[i]->ammo!=0) {
      if (player_cockpit) {
	  touched=true;
      }
      if (increase_locking&&(dist_sqr_to_target>mounts[i]->type->Range*mounts[i]->type->Range)) {
	mounts[i]->time_to_lock-=SIMULATION_ATOM;
	static bool ai_lock_cheat=XMLSupport::parse_bool(vs_config->getVariable ("physics","ai_lock_cheat","true"));	
	if (!player_cockpit) {
	  if (ai_lock_cheat) {
	    mounts[i]->time_to_lock=-1;
	  }
	}else {

	  if (mounts[i]->type->LockTime>0) {
	    static string LockedSoundName= vs_config->getVariable ("unitaudio","locked","locked.wav");
	    static int LockedSound = AUDCreateSoundWAV (LockedSoundName,false);

	    if (mounts[i]->time_to_lock>-SIMULATION_ATOM&&mounts[i]->time_to_lock<=0) {
	      if (!AUDIsPlaying(LockedSound)) {
		AUDStartPlaying(LockedSound);
		AUDStopPlaying(LockingSound);	      
	      }
	      AUDAdjustSound (LockedSound,Position(),GetVelocity()); 
	    }else if (mounts[i]->time_to_lock>0)  {
	      locking=true;
	      if (!AUDIsPlaying(LockingSound)) {
		AUDStartPlaying(LockingSound);	      
	      }
	      AUDAdjustSound (LockingSound,Position(),GetVelocity());
	    }
	  }
	}
      }else {
        if (mounts[i]->ammo!=0) {
	  mounts[i]->time_to_lock=mounts[i]->type->LockTime;
        }
      }
    } else {
      if (mounts[i]->ammo!=0) {
        mounts[i]->time_to_lock=mounts[i]->type->LockTime;
      }
    }
    if (mounts[i]->type->type==weapon_info::BEAM) {
      if (mounts[i]->ref.gun) {
	mounts[i]->ref.gun->UpdatePhysics (cumulative_transformation, cumulative_transformation_matrix,((mounts[i]->size&weapon_info::AUTOTRACKING)&&(mounts[i]->time_to_lock<=0))?target:NULL ,computer.radar.trackingcone, target);
      }
    } else {
      mounts[i]->ref.refire+=SIMULATION_ATOM;
    }
    if (mounts[i]->processed==Mount::FIRED) {
      Transformation t1;
      Matrix m1;
      t1=prev_physical_state;//a hack that will not work on turrets
      t1.Compose (trans,transmat);
      t1.to_matrix (m1);
      int autotrack=0;
      if ((0!=(mounts[i]->size&weapon_info::AUTOTRACKING))) {
	autotrack = computer.itts?2:1;
      }
      mounts[i]->PhysicsAlignedFire (t1,m1,cumulative_velocity,(!SubUnit||owner==NULL)?this:owner,target,autotrack, computer.radar.trackingcone);
      if (mounts[i]->ammo==0&&mounts[i]->type->type==weapon_info::PROJECTILE) {
	ToggleWeapon (true);
      }
    }else if (mounts[i]->processed==Mount::UNFIRED) {
      mounts[i]->PhysicsAlignedUnfire();
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

/****************************** ONLY SOUND/GFX STUFF LEFT IN THOSE FUNCTIONS *********************************/

template <class UnitType>
void GameUnit<UnitType>::Thrust(const Vector &amt1,bool afterburn){
  Unit::Thrust( amt1, afterburn);
 if (_Universe->AccessCockpit(0)->GetParent()==this)
  if (afterburn!=AUDIsPlaying (sound->engine)) {
    if (afterburn)
      AUDPlay (sound->engine,cumulative_transformation.position,cumulative_velocity,1);
    else
      //    if (Velocity.Magnitude()<computer.max_speed)
      AUDStopPlaying (sound->engine);
  }
}

template <class UnitType>
Vector GameUnit<UnitType>::ResolveForces (const Transformation &trans, const Matrix &transmat) {
#ifndef PERFRAMESOUND
  AUDAdjustSound (sound->engine,cumulative_transformation.position, cumulative_velocity); 
#endif
	return Unit::ResolveForces( trans, transmat);
}

template <class UnitType>
void GameUnit<UnitType>::SetPlanetOrbitData (PlanetaryTransform *t) {
#ifdef FIX_TERRAIN
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
#endif
}

template <class UnitType>
PlanetaryTransform * GameUnit<UnitType>::GetPlanetOrbit () const {
#ifdef FIX_TERRAIN
if (planet==NULL)
    return NULL;
  return planet->trans;
#else
  return NULL;
#endif
}

