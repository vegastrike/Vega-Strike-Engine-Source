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
//#include "unit_template.h"
#include "lin_time.h"
//#include "physics.h"
#include "beam.h"
#include "planet.h"
#include "audiolib.h"
#include "configxml.h"
#include "vs_globals.h"
//#ifdef WIN32
#ifdef FIX_TERRAIN
#include "gfx/planetary_transform.h"
#endif
#include "gfx/cockpit.h"
#include "unit_util.h"
#include "universe_util.h"
#include "cmd/script/mission.h"
#include "networking/lowlevel/vsnet_clientstate.h"
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

extern unsigned int apply_float_to_unsigned_int (float tmp); //short fix
//    float max_speed;
//    float max_ab_speed;
//    float max_yaw;
//    float max_pitch;
//    float max_roll;

template <class UnitType>
void GameUnit<UnitType>::UpdatePhysics2 (const Transformation &trans, const Transformation & old_physical_state, const Vector & accel, float difficulty, const Matrix &transmat, const Vector & cum_vel,  bool lastframe, UnitCollection *uc) {
	int player = -1;

  UnitType::UpdatePhysics2( trans, old_physical_state, accel, difficulty, transmat, cum_vel, lastframe, uc);
  // Here send (new position + direction = curr_physical_state.position and .orientation)
  // + speed to server (which velocity is to consider ?)
  // + maybe Angular velocity to anticipate rotations in the other network clients
  if( Network!=NULL)
  {
	  //cout<<"SEND UPDATE"<<endl;
	  // Check if this is a player, because in network mode we should only send updates of our moves
	  player= _Universe->whichPlayerStarship( this);
	  if( player>=0 /* && this->networked */ )
	  {
		if (Network[0].isTime()) {
		  //cout<<"Player number : "<<player<<endl;
		  // (NetForce + Transform (ship_matrix,NetLocalForce) )/mass = GLOBAL ACCELERATION

		  //curr_physical_state.position = curr_physical_state.position +  (Velocity*SIMULATION_ATOM*difficulty).Cast();
		  // If we want to inter(extra)polate sent position, DO IT HERE
//		  if( !(old_physical_state.position == this->curr_physical_state.position && old_physical_state.orientation == this->curr_physical_state.orientation) )
				// We moved so update
		  {
			  /* If you're going to send an alive message, you might as well send your position while you're at it. */
				ClientState cstmp( this->serial, this->curr_physical_state, this->Velocity, accel, 0);
				Network[player].sendPosition( &cstmp);
		  }
//		  else
//		  {
//				// Say we are still alive
//				Network[player].sendAlive();
//		  }
		}
		this->AddVelocity(difficulty);
	  }
	  else
	  {
		  // Not a player so update the unit's position and stuff with the last received snapshot from the server
		  // This may be be a bot or a unit controlled by the server
		  if( !this->networked)
			// Case it is a local unit
 			this->AddVelocity(difficulty);
		  else
		  {
		  	// Networked unit so interpolate its position
			this->curr_physical_state = Network[0].Interpolate( this, SIMULATION_ATOM);
 			this->AddVelocity(difficulty);
		  }
	  }
  }
  else
  {
     this->AddVelocity(difficulty);
  }

#ifdef DEPRECATEDPLANETSTUFF
  if (planet) {
    Matrix basis;
    curr_physical_state.to_matrix (this->cumulative_transformation_matrix);
    Vector p,q,r,c;
    MatrixToVectors (this->cumulative_transformation_matrix,p,q,r,c);
    planet->trans->InvTransformBasis (this->cumulative_transformation_matrix,p,q,r,c);
    planet->cps=Transformation::from_matrix (this->cumulative_transformation_matrix);
  }
#endif
  this->cumulative_transformation = this->curr_physical_state;
  this->cumulative_transformation.Compose (trans,transmat);
  this->cumulative_transformation.to_matrix (this->cumulative_transformation_matrix);
  this->cumulative_velocity = TransformNormal (transmat,this->Velocity)+cum_vel;

  Transformation * ct;
  Matrix * ctm=NULL;
  SetPlanetHackTransformation (ct,ctm);
  unsigned int i;
  if (lastframe) {
    char tmp=0;
    for (i=0;i<this->meshdata.size();i++) {
      if (!this->meshdata[i])
		continue;
      tmp |=this->meshdata[i]->HasBeenDrawn();
      if (!this->meshdata[i]->HasBeenDrawn()) {
		this->meshdata[i]->UpdateFX(SIMULATION_ATOM);
      }
      this->meshdata[i]->UnDraw();
    }
    if (!tmp&&this->hull<0) {
      Explode(false,SIMULATION_ATOM);
	
    }
  }
  //UnitType::UpdatePhysics2 (trans,old_physical_state,accel,difficulty,transmat, cum_vel,  lastframe,uc);
}

/****************************** ONLY SOUND/GFX STUFF LEFT IN THOSE FUNCTIONS *********************************/

template <class UnitType>
void GameUnit<UnitType>::Thrust(const Vector &amt1,bool afterburn){
  afterburn=afterburn&&this->energy>this->afterburnenergy*SIMULATION_ATOM;
  Unit::Thrust( amt1, afterburn);
  static bool must_afterburn_to_buzz=XMLSupport::parse_bool(vs_config->getVariable("audio","buzzing_needs_afterburner","false"));
  if (_Universe->isPlayerStarship(this)!=NULL) {
    static int playerengine = AUDCreateSound (vs_config->getVariable ("unitaudio","player_afterburner","sfx10.wav"),true);
    static float enginegain=XMLSupport::parse_float(vs_config->getVariable("audio","afterburner_gain",".5"));
    if (afterburn!=AUDIsPlaying (playerengine)) {
      if (afterburn)
        AUDPlay (playerengine,QVector(0,0,0),Vector(0,0,0),enginegain);
      else
        AUDStopPlaying (playerengine);
    }
  }else if (afterburn||!must_afterburn_to_buzz) {
    static float buzzingtime=XMLSupport::parse_float(vs_config->getVariable("audio","buzzing_time","5"));
    static float buzzingdistance=XMLSupport::parse_float(vs_config->getVariable("audio","buzzing_distance","5"));
    static float lastbuzz=getNewTime();
    Unit * playa = _Universe->AccessCockpit()->GetParent();
    if (playa) {
      Vector diff=this->Position()-playa->Position();
      if (UnitUtil::getDistance(this,playa)<buzzingdistance&&playa->owner!=this&&this->owner!=playa&&this->owner!=playa->owner) {
        float ttime=getNewTime();
        if (ttime-lastbuzz>buzzingtime) {
          Vector pvel=playa->GetVelocity();
          Vector vel=this->GetVelocity();
          pvel.Normalize();
          vel.Normalize();
          float dotprod=vel.Dot(pvel);
          if (dotprod<.86) {
            
            lastbuzz=ttime;
            AUDPlay(this->sound->engine,this->Position(),this->GetVelocity(),1);
          } else {

          }         
        }
      }
    }
    
  }
}

template <class UnitType>
Vector GameUnit<UnitType>::ResolveForces (const Transformation &trans, const Matrix &transmat) {
#ifndef PERFRAMESOUND
  AUDAdjustSound (this->sound->engine,this->cumulative_transformation.position, this->cumulative_velocity); 
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

