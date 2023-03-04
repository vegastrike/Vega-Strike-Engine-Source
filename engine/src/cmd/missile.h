/*
 * missile.h
 *
 * Copyright (C) 2001-2022 Daniel Horn
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


#ifndef MISSILE_H_
#define MISSILE_H_

#include "unit_util.h"
#include "unit_generic.h"

class MissileEffect {
private:
    QVector pos;
    float damage;
    float phasedamage;
    float radius;
    float radialmultiplier;
    void *ownerDoNotDereference;

public:
    MissileEffect(const QVector &pos, float dam, float pdam, float radius, float radmult, void *owner);
    void ApplyDamage(Unit *);
    const QVector &GetCenter() const;
    float GetRadius() const;

private:
    void DoApplyDamage(Unit *parent, Unit *un, float distance, float damage_fraction);
};

class Missile : public Unit {
protected:
    float time;
    float damage;
    float phasedamage;
    float radial_effect;
    float radial_multiplier;
    float detonation_radius;
    bool discharged;
    bool had_target;
    signed char retarget;
public:
    Missile(const char *filename,
            int faction,
            const string &modifications,
            const float damage,
            float phasedamage,
            float time,
            float radialeffect,
            float radmult,
            float detonation_radius);

    Missile(vega_types::SequenceContainer<vega_types::SharedPtr<Mesh>> &m, bool b, int i) :
            Unit(m, b, i) {
    }

public:
    virtual void Kill(bool erase = true);
    void Discharge();
    float ExplosionRadius();

    enum Vega_UnitType isUnit() const {
        return Vega_UnitType::missile;
    }

    virtual void UpdatePhysics2(const Transformation &trans,
            const Transformation &old_physical_state,
            const Vector &accel,
            float difficulty,
            const Matrix &transmat,
            const Vector &CumulativeVelocity,
            bool ResolveLast,
            UnitCollection *uc = NULL);

    Unit *breakECMLock(Unit *target);
    bool proximityFuse(Unit *target);
    bool useFuel(Unit *target, bool had_target);

    ~Missile() override = default;

    // TODO: consider if this is really necessary
    /// default constructor forbidden
    Missile() = delete;
    /// copy constructor forbidden
    Missile(const Missile &) = delete;
    /// assignment operator forbidden
    Missile &operator=(const Missile &) = delete;
};

#endif

