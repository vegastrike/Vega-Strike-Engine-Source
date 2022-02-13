/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */
#include "macosx_math.h"
#include "lin_time.h"
#include "physics.h"
#include "gfx/quaternion.h"

PhysicsSystem::PhysicsSystem(float M, float I, QVector *pos, Vector *p, Vector *q, Vector *r) :
        mass(M),
        MomentOfInertia(I),
        NetForce(0, 0, 0),
        NetTorque(0, 0, 0),
        AngularVelocity(0, 0, 0),
        Velocity(0, 0, 0),
        pos(pos),
        p(p),
        q(q),
        r(r) {
    NumActiveForces = 0;
    NumActiveTorques = 0;
}

void PhysicsSystem::ResistiveLiquidTorque(float ResistiveForceCoef) {
    NetTorque += ResistiveForceCoef * AngularVelocity;
}

void PhysicsSystem::ResistiveLiquidForce(float ResistiveForceCoef) {
    NetForce += ResistiveForceCoef * Velocity;
}

void PhysicsSystem::ResistiveThrust(float strength) {
    Vector V1 = Velocity;
    float mag = V1.Magnitude();
    float t = mag / fabs(strength);
    ApplyForce((strength / mag) * V1, t);
}

void PhysicsSystem::ResistiveTorqueThrust(float strength, const Vector &Position) {
    Vector V1 = AngularVelocity;
    float mag = V1.Magnitude();
    float t = mag / fabs(strength);
    ApplyBalancedLocalTorque((strength / mag) * V1, Position, t);
}

void PhysicsSystem::ResistiveTorque(float ResistiveForceCoef) {
    if ((AngularVelocity.i || AngularVelocity.j || AngularVelocity.k) && ResistiveForceCoef) {
        Vector temp = AngularVelocity;
        NetTorque += (ResistiveForceCoef * AngularVelocity * AngularVelocity) * temp.Normalize();
    }
}

void PhysicsSystem::ResistiveForce(float ResistiveForceCoef) {
    if ((Velocity.i || Velocity.j || Velocity.k) && ResistiveForceCoef) {
        Vector temp = Velocity;
        NetForce += (ResistiveForceCoef * Velocity * Velocity) * temp.Normalize();
    }
}

void PhysicsSystem::Update() {
    ApplyImpulses(GetElapsedTime() / getTimeCompression());
    NetForce = Vector(0, 0, 0);
    NetTorque = Vector(0, 0, 0);
}

void PhysicsSystem::Rotate(const Vector &axis) {
    float theta = axis.Magnitude();
    if (theta == 0.0f) {
        return;
    }
    float ootheta = 1 / theta;
    float s = cos(theta * .5);
    Quaternion rot = Quaternion(s, axis * (sin(theta * .5) * ootheta));
    Quaternion rotprime = rot.Conjugate();
    Quaternion pquat = rot * Quaternion(0, *p) * rotprime;
    Quaternion qquat = rot * Quaternion(0, *q) * rotprime;
    Quaternion rquat = rot * Quaternion(0, *r) * rotprime;
    *p = pquat.v;
    *q = qquat.v;
    *r = rquat.v;
}

void PhysicsSystem::JettisonReactionMass(const Vector &Direction, float speed, float mass) {
    NetForce += Direction * (speed * mass / GetElapsedTime());
}

void PhysicsSystem::JettisonMass(const Vector &Direction, float speed, float jmass) {
    mass -= jmass;     //fuel is sent out
    JettisonReactionMass(Direction, speed, jmass);
}

void PhysicsSystem::ApplyForce(const Vector &Vforce, float time) {
    if (NumActiveForces < forcemax) {
        ActiveForces[NumActiveForces].F = Vforce;
        ActiveForces[NumActiveForces].t = time;
        ++NumActiveForces;
    }
}

void PhysicsSystem::ApplyTorque(const Vector &Vforce, const Vector &Location, float time) {
    ApplyForce(Vforce, time);
    if (NumActiveTorques < forcemax) {
        ActiveTorques[NumActiveTorques].F = (Location.Cast() - *pos).Cast().Cross(Vforce);
        ActiveTorques[NumActiveTorques].t = time;
        ++NumActiveTorques;
    }
}

void PhysicsSystem::ApplyLocalTorque(const Vector &Vforce, const Vector &Location, float time) {
    ApplyForce(Vforce, time);
    if (NumActiveTorques < forcemax) {
        ActiveTorques[NumActiveTorques].F = Location.Cross(Vforce);
        ActiveTorques[NumActiveTorques].t = time;
        ++NumActiveTorques;
    }
}

void PhysicsSystem::ApplyBalancedLocalTorque(const Vector &Vforce, const Vector &Location, float time) {
    if (NumActiveTorques < forcemax) {
        ActiveTorques[NumActiveTorques].F = Location.Cross(Vforce);
        ActiveTorques[NumActiveTorques].t = time;
        ++NumActiveTorques;
    }
}

void PhysicsSystem::ApplyImpulses(float Time) {
    Vector temptorque = Time * NetTorque;
    Vector tempforce = Time * NetForce;
    int i;
    for (i = 0; i < NumActiveTorques; ++i) {
        if (Time >= ActiveTorques[i].t) {
            temptorque += ActiveTorques[i].t * ActiveTorques[i].F;
            ActiveTorques[i].F = ActiveTorques[NumActiveTorques - 1].F;
            ActiveTorques[i].t = ActiveTorques[NumActiveTorques - 1].t;
            --NumActiveTorques;
            --i;             //so the loop goes through the active force that was just switched places with
        } else {
            temptorque += Time * ActiveTorques[i].F;
            ActiveTorques[i].t -= Time;
        }
    }
    for (i = 0; i < NumActiveForces; ++i) {
        if (Time >= ActiveForces[i].t) {
            tempforce += ActiveForces[i].t * ActiveForces[i].F;
            ActiveForces[i].F = ActiveForces[NumActiveForces - 1].F;
            ActiveForces[i].t = ActiveForces[NumActiveForces - 1].t;
            NumActiveForces--;
            i--;             //so the loop goes through the active force that was just switched places with
        } else {
            tempforce += Time * ActiveForces[i].F;
            ActiveForces[i].t -= Time;
        }
    }
    temptorque = temptorque * (0.5 / MomentOfInertia);
    Rotate(AngularVelocity + 0.5 * temptorque);
    AngularVelocity += temptorque;
    tempforce = tempforce * (0.5 / mass);       //acceleration
    //now the fuck with it... add relitivity to the picture here
    if (fabs(Velocity.i) + fabs(Velocity.j) + fabs(Velocity.k) > co10) {
        float magvel = Velocity.Magnitude();
        float y = (1 - magvel * magvel * oocc);
        tempforce = tempforce * powf(y, 1.5);
    }
    *pos += (Velocity + .5 * tempforce).Cast();
    Velocity += tempforce;
}

