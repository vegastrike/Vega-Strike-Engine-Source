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
#include "gfx_mesh.h"
#include "cmd_unit.h"
#include "lin_time.h"
#include "physics.h"
#include "cmd_beam.h"
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

void Unit::ApplyTorque (const Vector &Vforce, const Vector &Location)
{
  //Not completely correct
	NetForce += Vforce;
	NetTorque += Vforce.Cross (Location-curr_physical_state.position);
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
  NetTorque += ClampTorque(torque);
}
#ifdef WIN32
float copysign (float x, float y) {
	if (y>0)
			return x;
	else
			return -x;
}
#endif

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
Vector Unit::ClampTorque (const Vector &amt1) {
  Vector Res=amt1;
  if (Res.i>fabs(limits.yaw))
    Res.i=amt1.i>0?fabs(limits.yaw):-fabs(limits.yaw);
  if (amt1.j>fabs(limits.pitch))
    Res.j=amt1.j>0?fabs(limits.pitch):-fabs(limits.pitch);
  if (amt1.j>fabs(limits.roll))
    Res.k=amt1.k>0?fabs(limits.roll):-fabs(limits.roll);
  return Res;
}

Vector Unit::MaxThrust(const Vector &amt1) {
  // amt1 is a normal
  return amt1 * (Vector(copysign(limits.lateral, amt1.i), 
	       copysign(limits.vertical, amt1.j),
	       copysign(limits.longitudinal, amt1.k)) * amt1);
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
Vector Unit::ClampThrust (const Vector &amt1) {
  Vector Res=amt1;
  if (Res.i>fabs(limits.lateral))
    Res.i=amt1.i>0?fabs(limits.lateral):-fabs(limits.lateral);
  if (amt1.j>fabs(limits.vertical))
    Res.j=amt1.j>0?fabs(limits.vertical):-fabs(limits.vertical);
  if (amt1.j>fabs(limits.longitudinal))
    Res.k=amt1.k>0?fabs(limits.longitudinal):-fabs(limits.longitudinal);
  return Res;
}

void Unit::Thrust(const Vector &amt1){
  Vector amt = ClampThrust(amt1);
  ApplyLocalForce(amt);
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
  ApplyLocalForce(amt*limits.longitudinal * Vector(0,0,1));
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

void Unit::ResolveForces (const Transformation &trans, const Matrix transmat, bool lastframe) {
  if (lastframe)
    prev_physical_state = curr_physical_state;  
  // Torque is modeled as a perfect impulse at the beginning of a game
  // turn, for simplicity
  Vector temp = NetTorque *SIMULATION_ATOM*(1.0/MomentOfInertia);
  AngularVelocity += temp;
  if(AngularVelocity.Magnitude() > 0) {
    Rotate (SIMULATION_ATOM*(AngularVelocity));
  }
  Vector p, q, r;
  GetOrientation(p,q,r);
//	cerr << "Orientation: " << p << q << r << endl;
  temp = ((NetForce + NetLocalForce.i*p + NetLocalForce.j*q + NetLocalForce.k*r ) * SIMULATION_ATOM)/mass; //acceleration
  Velocity += temp; // modelled as an impulse
	//now the fuck with it... add relitivity to the picture here
	/*
	if (fabs (Velocity.i)+fabs(Velocity.j)+fabs(Velocity.k)> co10)
	{
		float magvel = Velocity.Magnitude();
		float y = (1-magvel*magvel*oocc);
		temp = temp * powf (y,1.5);
		}*/
	
  curr_physical_state.position += Velocity*SIMULATION_ATOM;
  cumulative_transformation = curr_physical_state;
  cumulative_transformation.Compose (trans,transmat);
  cumulative_transformation.to_matrix (cumulative_transformation_matrix);
  int i;
  for (i=0;i<nummounts;i++) {
    if (mounts[i].type.type==weapon_info::BEAM) {
      if (mounts[i].gun&&!mounts[i].gun->Dissolved()) {
	mounts[i].gun->UpdatePhysics (cumulative_transformation, cumulative_transformation_matrix);
      }
    }
  }
  for (i=0;i<numsubunit;i++) {
    subunits[i]->ResolveForces(cumulative_transformation,cumulative_transformation_matrix,lastframe);
  }
  NetForce = NetLocalForce = Vector(0,0,0);
  NetTorque = Vector(0,0,0);
  //cerr << "new position of " << name << ": " << curr_physical_state.position << ", velocity " << Velocity << endl;
}

void Unit::GetOrientation(Vector &p, Vector &q, Vector &r) const {
  Matrix m;
  curr_physical_state.to_matrix(m);
  p.i = m[0];
  p.j = m[1];
  p.k = m[2];

  q.i = m[4];
  q.j = m[5];
  q.k = m[6];

  r.i = m[8];
  r.j = m[9];
  r.k = m[10];
}

Vector Unit::ToLocalCoordinates(const Vector &v) const {
  //Matrix m;
  //062201: not a cumulative transformation...in prev unit space  curr_physical_state.to_matrix(m);
  
#define M(A,B) cumulative_transformation_matrix[B*4+A]
  return Vector(v.i*M(0,0)+v.j*M(1,0)+v.k*M(2,0),
		v.i*M(0,1)+v.j*M(1,1)+v.k*M(2,1),
		v.i*M(0,2)+v.j*M(1,2)+v.k*M(2,2));
}

