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
void GameUnit<UnitType>::UpdatePhysics2 (const Transformation &trans, const Transformation & old_physical_state, const Vector & accel, float difficulty, const Matrix &transmat, const Vector & cum_vel,  bool lastframe, UnitCollection *uc) {
	int player = -1;

  // Here send (new position + direction = curr_physical_state.position and .orientation)
  // + speed to server (which velocity is to consider ?)
  // + maybe Angular velocity to anticipate rotations in the other network clients
  if( Network!=NULL && Network[0].isTime())
  {
	  //cout<<"SEND UPDATE"<<endl;
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
				ClientState cstmp( this->serial, curr_physical_state, Velocity, accel, 0);
				Network[player].sendPosition( &cstmp);
		  }
		  else
				// Say we are still alive
				Network[player].sendAlive();
	  }
	  else
	  {
		  // Not a player so update the unit's position and stuff with the last received snapshot from the server
		  // This may be be a bot or a unit controlled by the server
		  if( !this->networked)
			// Case it is a local unit
 			curr_physical_state.position = curr_physical_state.position +  (Velocity*SIMULATION_ATOM*difficulty).Cast();
	  }
  }
  else
  {
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
  UnitType::UpdatePhysics2 (trans,old_physical_state,accel,difficulty,transmat, cum_vel,  lastframe,uc);
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

