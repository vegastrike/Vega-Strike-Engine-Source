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


void Mesh:: Rotate (const Vector &axis)
{
	float theta = axis.Magnitude();
	float ootheta = 1/theta;
	float s = cos (theta * .5);
	Quaternion rot = Quaternion(s, axis * (sinf (theta*.5)*ootheta));
	Quaternion rotprime = rot.Conjugate();
	Quaternion pquat = rot * Quaternion(0, p) * rotprime;
	Quaternion qquat = rot * Quaternion(0, q) * rotprime;
	Quaternion rquat = rot * Quaternion(0, r) * rotprime;
	p = pquat.v;
	q = qquat.v;
	r = rquat.v;
}
void Unit:: FireEngines (Vector Direction/*unit vector... might default to "r"*/,
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
void Unit::ApplyForce(Vector Vforce) //applies a force for the whole gameturn upon the center of mass
{
	NetForce += Vforce;
}
void Unit::Accelerate(Vector Vforce)
{
  NetForce += Vforce * mass;
}

void Unit::ApplyTorque (Vector Vforce, Vector Location)
{
	NetForce += Vforce;
	NetTorque += Vforce.Cross (Location-pos);
}
void Unit::ApplyLocalTorque (Vector Vforce, Vector Location)
{
	NetForce += Vforce;
	NetTorque += Vforce.Cross (Location);
}
void Unit::ApplyBalancedLocalTorque (Vector Vforce, Vector Location) //usually from thrusters remember if I have 2 balanced thrusters I should multiply their effect by 2 :)
{
	NetTorque += Vforce.Cross (Location);
}

void Unit::ResolveForces () // don't call this 2x
{
  Vector temp = NetTorque *SIMULATION_ATOM*(1.0/MomentOfInertia);//assume force is constant throughout the time
  if(AngularVelocity.Magnitude() > 0) {
    Rotate (SIMULATION_ATOM*(AngularVelocity+ temp*.5));
  }
	AngularVelocity+= temp;
	temp = NetForce * SIMULATION_ATOM*(1/mass);//acceleration
	//now the fuck with it... add relitivity to the picture here
	/*
	if (fabs (Velocity.i)+fabs(Velocity.j)+fabs(Velocity.k)> co10)
	{
		float magvel = Velocity.Magnitude();
		float y = (1-magvel*magvel*oocc);
		temp = temp * powf (y,1.5);
		}*/
	pos += (Velocity+.5*temp)*SIMULATION_ATOM;
	Velocity += temp;
	NetForce = Vector(0,0,0);
	NetTorque = Vector(0,0,0);
	changed = TRUE;
}
