/*
 * missile.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
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
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_ENGINE_CMD_MISSILE_H
#define VEGA_STRIKE_ENGINE_CMD_MISSILE_H

#include "cmd/unit_util.h"
#include "cmd/unit_generic.h"

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

    Missile(std::vector<Mesh *> m, bool b, int i) : Unit(m, b, i), time(0.0F), damage(0.0F), phasedamage(0.0F), radial_effect(0.0F),
                                                    radial_multiplier(0.0F),
                                                    detonation_radius(0.0F),
                                                    discharged(false),
                                                    had_target(false), retarget(0) {
    }

    virtual void Kill(bool erase = true);
    void Discharge();
    float ExplosionRadius() override;

    enum Vega_UnitType isUnit() const override {
        return Vega_UnitType::missile;
    }

    Vega_UnitType getUnitType() const override {
        return Vega_UnitType::missile;
    }

    void UpdatePhysics2(const Transformation &trans,
            const Transformation &old_physical_state,
            const Vector &accel,
            float difficulty,
            const Matrix &transmat,
            const Vector &CumulativeVelocity,
            bool ResolveLast,
            UnitCollection *uc = NULL) override;

    Unit *breakECMLock(Unit *target);
    bool proximityFuse(Unit *target);
    bool useFuel(Unit *target, bool had_target);

    /// default constructor forbidden
    Missile() = delete;
    /// copy constructor forbidden
    Missile(const Missile &) = delete;
    /// assignment operator forbidden
    Missile &operator=(const Missile &) = delete;

    ~Missile() override;
};

#endif //VEGA_STRIKE_ENGINE_CMD_MISSILE_H
