/**
 * damageable.h
 *
 * Copyright (C) 2020 Roy Falk, Stephen G. Tuggy and other Vega Strike
 * contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


#ifndef DAMAGEABLE_H
#define DAMAGEABLE_H

#include "damageable_object.h"
#include "gfx/vec.h"
#include "mount_size.h"

class Unit;
struct GFXColor;

/**
 * @brief The Damageable class TODO
 */
class Damageable : public DamageableObject {
public:
    DamageableLayer *hull;
    DamageableLayer *armor;
    DamageableLayer *shield;

    float *current_hull;
    float *max_hull;

    // These are only used for upgrade due to macros
    // TODO: refactor
    float upgrade_hull;

    float shield_regeneration;

    //Is dead already?
    bool killed;

    // Methods
public:
    Damageable() : hull(&layers[0]),
                   armor(&layers[1]),
                   shield(&layers[2]),
                   current_hull(&hull->facets[as_integer(FacetName::single)].health),
                   max_hull(&hull->facets[as_integer(FacetName::single)].max_health),
                   upgrade_hull(0),
                   shield_regeneration(0),
                   killed(false)
    {
    }

protected:
    virtual ~Damageable() = default;
    // forbidden
    Damageable(const Damageable &) = delete;
    // forbidden
    Damageable &operator=(const Damageable &) = delete;

public:
    // We follow the existing convention of GetX for the actual health value
    // because we are constrained by existing python interfaces, which cannot
    // be easily changed.
    // TODO: convert all calls to *current_hull
    const float GetHull() const
    {
        return *current_hull;
    }

    // TODO: check for valid index
    const float GetArmor(int facet = 0) const
    {
        return armor->facets[facet].health;
    }

    const float GetShield(int facet = 0) const
    {
        return shield->facets[facet].health;
    }

    DamageableLayer &GetHullLayer()
    {
        return layers[0];
    }

    DamageableLayer &GetArmorLayer()
    {
        return layers[1];
    }

    DamageableLayer &GetShieldLayer()
    {
        return layers[2];
    }

    const float GetShieldRegeneration() const
    {
        return shield->facets[as_integer(FacetName::left_top_front)].regeneration;
    }

    virtual const float GetHullPercent() const
    {
        return hull->GetPercent(FacetName::single);
    }

    virtual const float GetShieldPercent() const
    {
        return shield->GetPercent(FacetName::left_top_front);
    }

    void ArmorData(float armor[8]) const;

    // Is the shield up from this position
    bool ShieldUp(const Vector &) const;

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

    float FShieldData() const;
    float RShieldData() const;
    float LShieldData() const;
    float BShieldData() const;
    //short fix
    //  void ArmorData( float armor[8] ) const;
    //Gets the current status of the hull
    //  float GetHull() const
    //  {
    //      return hull;
    //  }



    //reduces shields to X percentage and reduces shield recharge to Y percentage
    void leach(float XshieldPercent, float YrechargePercent, float ZenergyPercent);

    float MaxShieldVal() const;
    void RegenerateShields(const float difficulty, const bool player_ship);

    bool flickerDamage();
};

#endif // DAMAGEABLE_H
