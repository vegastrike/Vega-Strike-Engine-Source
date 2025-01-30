/**
 * damageable.h
 *
 * Copyright (C) 2020 Roy Falk, Stephen G. Tuggy and other Vega Strike
 * contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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
#ifndef VEGA_STRIKE_ENGINE_CMD_DAMAGEABLE_H
#define VEGA_STRIKE_ENGINE_CMD_DAMAGEABLE_H

#include "damageable_object.h"
#include "gfx/vec.h"
#include "mount_size.h"

class Unit;
struct GFXColor;

// TODO: most of what's left in this class should go up to lib damage.
// The rest can be refactored elsewhere and the class deleted.

/**
 * @brief The Damageable class TODO
 */
class Damageable : public DamageableObject {
public:
    //Is dead already?
    bool killed;

    // Methods
public:
    Damageable() : killed(false) {}

protected:
    virtual ~Damageable() = default;
    // forbidden
    Damageable(const Damageable &) = delete;
    // forbidden
    Damageable &operator=(const Damageable &) = delete;

public:

    // Currently available in case someone wants to call from python
    // but there isn't an interface
    float DealDamageToShield(const Vector &pnt, float &Damage);

    // This has a python interface
    float DealDamageToHull(const Vector &pnt, float Damage);

    //Applies damage to the pre-transformed area of the ship
    void ApplyDamage(const Vector &pnt,
            const Vector &normal,
            Damage damage,
            Unit *affected_unit,
            const GFXColor &color,
            void *ownerDoNotDereference);
    void DamageRandomSystem(InflictedDamage inflicted_damage, bool player, Vector attack_vector);
    void DamageCargo(InflictedDamage inflicted_damage);
    void Destroy(); //explodes then deletes
   



    //reduces shields to X percentage and reduces shield recharge to Y percentage
    void leach(float XshieldPercent, float YrechargePercent, float ZenergyPercent);

    bool flickerDamage();
};

#endif //VEGA_STRIKE_ENGINE_CMD_DAMAGEABLE_H
