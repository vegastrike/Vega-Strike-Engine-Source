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
#include "planet.h"
//#ifdef WIN32
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
Vector Unit::ClampTorque (const Vector &amt1) {
  Vector Res=amt1;
  if (fabs(amt1.i)>limits.pitch)
    Res.i=copysign(limits.pitch,amt1.i);
  if (fabs(amt1.j)>limits.yaw)
    Res.j=copysign(limits.yaw,amt1.j);
  if (fabs(amt1.k)>limits.roll)
    Res.k=copysign(limits.roll,amt1.k);
  return Res;
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
  if (fabs(amt1.i)>fabs(limits.lateral))
    Res.i=copysign(limits.lateral,amt1.i);
  if (fabs(amt1.j)>fabs(limits.vertical))
    Res.j=copysign(limits.vertical,amt1.j);
  if (amt1.k>(afterburn?limits.afterburn:limits.forward))
    Res.k=afterburn?limits.afterburn:limits.forward;
  if (amt1.k<-limits.retro)
    Res.k =-limits.retro;
  return Res;
}


void Unit::Thrust(const Vector &amt1,bool afterburn){
  Vector amt = ClampThrust(amt1,afterburn);
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
void Unit::UpdatePhysics (const Transformation &trans, const Matrix transmat, bool lastframe, UnitCollection *uc) {
  if (lastframe)
    prev_physical_state = curr_physical_state;//the AIscript should take care
  CollideAll(); 
  if (isUnit()==PLANETPTR) {
    ((Planet *)this)->gravitate (uc);
  } else {
    ResolveForces (trans,transmat);
  } 
    
  curr_physical_state.position += Velocity*SIMULATION_ATOM;
  cumulative_transformation = curr_physical_state;
  cumulative_transformation.Compose (trans,transmat);
  cumulative_transformation.to_matrix (cumulative_transformation_matrix);
  
  
  int i;
  for (i=0;i<nummounts;i++) {
    if (mounts[i].type.type==weapon_info::BEAM) {
      if (mounts[i].gun) {
	mounts[i].gun->UpdatePhysics (cumulative_transformation, cumulative_transformation_matrix);
      }
    }
  }
  bool dead=true;
  for (i=0;i<numsubunit;i++) {
    subunits[i]->UpdatePhysics(cumulative_transformation,cumulative_transformation_matrix,lastframe,uc);
    if (hull<0) {
      subunits[i]->timeexplode+=.1*SIMULATION_ATOM;//urge slowly on
      subunits[i]->hull-=SIMULATION_ATOM;
    }
    dead &=subunits[i]->Killed();
  }
  UpdateCollideQueue();

  if (hull<0) {
    if (timeexplode==0) {
      Explode(false);
    }
    dead&= (explosion==NULL);
    
    if (dead)
      Kill();
  }
}

void Unit::ResolveForces (const Transformation &trans, const Matrix transmat) {
  Vector p, q, r;
  GetOrientation(p,q,r);
  Vector temp = (InvTransformNormal(transmat,NetTorque)+NetLocalTorque.i*p+NetLocalTorque.j*q+NetLocalTorque.k *r)*SIMULATION_ATOM*(1.0/MomentOfInertia);
  AngularVelocity += temp;
  if(AngularVelocity.i||AngularVelocity.j||AngularVelocity.k) {
    Rotate (SIMULATION_ATOM*(AngularVelocity));
  }
  temp = ((InvTransformNormal(transmat,NetForce) + NetLocalForce.i*p + NetLocalForce.j*q + NetLocalForce.k*r ) * SIMULATION_ATOM)/mass; //acceleration
  Velocity += temp; 
  NetForce = NetLocalForce = NetTorque = NetLocalTorque = Vector(0,0,0);
  /*
    if (fabs (Velocity.i)+fabs(Velocity.j)+fabs(Velocity.k)> co10) {
    float magvel = Velocity.Magnitude(); float y = (1-magvel*magvel*oocc);
    temp = temp * powf (y,1.5);
    }*/
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

Vector Unit::UpCoordinateLevel (const Vector &v) const {
  float m[16];
  curr_physical_state.to_matrix(m);
#define M(A,B) m[B*4+A]
  return Vector(v.i*M(0,0)+v.j*M(1,0)+v.k*M(2,0),
		v.i*M(0,1)+v.j*M(1,1)+v.k*M(2,1),
		v.i*M(0,2)+v.j*M(1,2)+v.k*M(2,2));
#undef M
}

Vector Unit::ToLocalCoordinates(const Vector &v) const {
  //Matrix m;
  //062201: not a cumulative transformation...in prev unit space  curr_physical_state.to_matrix(m);
  
#define M(A,B) cumulative_transformation_matrix[B*4+A]
  return Vector(v.i*M(0,0)+v.j*M(1,0)+v.k*M(2,0),
		v.i*M(0,1)+v.j*M(1,1)+v.k*M(2,1),
		v.i*M(0,2)+v.j*M(1,2)+v.k*M(2,2));
#undef M
}

Vector Unit::ToWorldCoordinates(const Vector &v) const {
  return TransformNormal(cumulative_transformation_matrix,v); 
#undef M

}

