/*
 * health.h
 *
 * Copyright (C) 2021-2023 Daniel Horn, Roy Falk, Stephen G. Tuggy, Benjamen R. Meyer,
 * and other Vega Strike contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
#ifndef VEGA_STRIKE_ENGINE_DAMAGE_HEALTH_H
#define VEGA_STRIKE_ENGINE_DAMAGE_HEALTH_H

#include "damage.h"

#include "resource/resource.h"

/**
 * @brief The Health struct represents the health of something.
 * It can be a shield, armor, hull or subsystem.
 * @details
 * More specifically, it represents the health of DamageableFacet.
 * If the health of a non-regenerative facet is zero, it is destroyed.
 * If this facet is the sole facet of a DamageableLayer, it is destroyed.
 * If this DamageableLayer is mortal, the whole DamageableObject is destroyed.
 * Therefore, destroying the hull of a ship destroys it.
 * But, destroying the reactor or life-support of a ship also disabled it,
 * killing everyone inside,
 * potentially without actually destroying the ship.
 */
struct Health {
    friend class Shield;
public:
    int layer; // The layer we're in, for recording damage

    Resource<double> health;
    bool destroyed;
    Damage vulnerabilities;
public:

    /**
     * @brief The Effect enum specifies what happens when the health of a specific object is zero.
     */
    enum class Effect {
        none,       // Nothing happens
        isolated,   // The facet/layer/subsystem is destroyed
        disabling,  // The whole DamageableObject is disabled but not destroyed.
        // It can then be tractored to another ship and repaired or sold.
        destroying  // The DamageableObject is destroyed, potentially leaving debris behind
    } effect{};


    Health(int layer, double health = 1);

    double Percent() const {
        return health.Percent();
    }

    
    void DealDamage(Damage &damage, InflictedDamage &inflicted_damage);
    void Destroy();
    void SetHealth(double new_health);

private:
    void DealDamageComponent(int type, double &damage, 
                             double vulnerability, 
                             InflictedDamage &inflicted_damage);

};

#endif //VEGA_STRIKE_ENGINE_DAMAGE_HEALTH_H
