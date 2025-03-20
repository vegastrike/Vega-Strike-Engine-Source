/*
 * Copyright (C) 2001-2023 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
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
#ifndef VEGA_STRIKE_ENGINE_PHYSICS_H
#define VEGA_STRIKE_ENGINE_PHYSICS_H

#include "vs_math.h"
#include "vec.h"

static const float oocc = (float) 0.0000000000000000111265005605; //1/c^2
static const float c = (float) 299792458.0;
static const float co10 = (float) 29979245.8;

struct Force {
    Vector F;
    float t;
};
const int forcemax = 64;
class PhysicsSystem {
    float mass, MomentOfInertia;
    Vector NetForce;
    Vector NetTorque;
    Vector AngularVelocity;
    Vector Velocity;
    QVector *pos;
    Vector *p, *q, *r;
    int NumActiveForces;
    Force ActiveForces[forcemax];
    int NumActiveTorques;
    Force ActiveTorques[forcemax];
public:
    PhysicsSystem(float M, float I, QVector *pos, Vector *p, Vector *q, Vector *r);
//~PhysicsSystem();
    virtual void Rotate(const Vector &axis);
    virtual void JettisonReactionMass(const Vector &Direction, float speed, float mass);
    virtual void JettisonMass(const Vector &Direction, float speed, float mass);
    virtual void ResistiveLiquidTorque(float ResistiveForceCoef);
    virtual void ResistiveTorque(float ResistiveForceCoef);
    virtual void ResistiveLiquidForce(float ResistiveForceCoef);
    virtual void ResistiveForce(float ResistiveForceCoef);
    virtual void ResistiveThrust(float strength);
    virtual void ResistiveTorqueThrust(float strength, const Vector &Position);
    virtual void ApplyForce(const Vector &Vforce, float time);
    virtual void ApplyTorque(const Vector &Vforce, const Vector &Location, float time);
    virtual void ApplyLocalTorque(const Vector &Vforce, const Vector &Location, float time);
    virtual void ApplyBalancedLocalTorque(const Vector &Vforce, const Vector &Location, float time);
    void ApplyImpulses(float Time);

    void SetVelocity(Vector v) {
        Velocity = v;
    }

    void SetAngularVelocity(Vector w) {
        AngularVelocity = w;
    }

    const Vector &GetVelocity() const {
        return Velocity;
    }

    const Vector &GetAngularVelocity() const {
        return AngularVelocity;
    }

    virtual void Update();
};

#endif //VEGA_STRIKE_ENGINE_PHYSICS_H

