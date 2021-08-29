/**
 * damageable.h
 *
 * Copyright (C) 2020 Roy Falk, Stephen G. Tuggy and other Vega Strike
 * contributors
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

class Unit;
class GFXColor;

/**
 * @brief The Damageable class TODO
 */
class Damageable : public DamageableObject
{
public:
    DamageableLayer *hull;
    DamageableLayer *armor;
    DamageableLayer *shield;

    float *current_hull;
    float *max_hull;


    bool dying;
    //Is dead already?
    bool  killed;

    // Methods
public:
    Damageable(): hull(nullptr),
        armor(nullptr),
        shield(nullptr),
        current_hull(nullptr),
        max_hull(nullptr),
        dying(false),
        killed(false) {}

protected:
    virtual ~Damageable() = default;
    // forbidden
    Damageable( const Damageable& ) = delete;
    // forbidden
    Damageable& operator=( const Damageable& ) = delete;

public:
    // We follow the existing convention of GetX for the actual health value
    // because we are constrained by existing python interfaces, which cannot
    // be easily changed.
    const float GetHull() const {
        return hull->facets[0].health;
    }

    const float GetArmor(int facet = 0) const {
        return armor->facets[facet].health;
    }

    const float GetShield(int facet = 0) const {
        return shield->facets[facet].health;
    }

    DamageableLayer& GetHullLayer() {
        return layers[0];
    }

    DamageableLayer& GetArmorLayer() {
        return layers[1];
    }

    DamageableLayer& GetShieldLayer() {
        return layers[2];
    }

    const float GetShieldRegeneration() const {
        return shield->facets[0].regeneration;
    }

    const float GetHullPercent() const
    {
        return hull->facets[0].Percent();
    }

    const float GetShieldPercent() const
    {
        return shield->facets[0].Percent();
    }



    void ArmorData( float armor[8] ) const;

    // Is the shield up from this position
    bool ShieldUp( const Vector& ) const;

    // Currently available in case someone wants to call from python
    // but there isn't an interface
    float DealDamageToShield( const Vector &pnt, float &Damage );

    // This has a python interface
    float DealDamageToHull( const Vector &pnt, float Damage );

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
    void leach( float XshieldPercent, float YrechargePercent, float ZenergyPercent );



    float MaxShieldVal() const;
    //regenerates all 2,4, or 6 shields for 1 SIMULATION_ATOM
    virtual void RegenShields() = 0;

    static float totalShieldEnergyCapacitance( const DamageableLayer &shield );


protected:
    bool flickerDamage();
};



#endif // DAMAGEABLE_H
