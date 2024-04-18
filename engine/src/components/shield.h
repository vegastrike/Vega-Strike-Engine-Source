/*
 * shield.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2024 Stephen G. Tuggy, Benjamen R. Meyer, Roy Falk and other Vega Strike Contributors
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_SHIELD_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_SHIELD_H

#include <string>
#include <map>

#include "component.h"
#include "damageable_layer.h"
#include "energy_consumer.h"

class Unit;

/* This class builds on top of DamageableLayer to represent a shield.
 * Shield functions (e.g. opacity, regeneration) are handled here.
 * DamageableLayer and Facet do not handle these at all.
 * Damage to the shield generator is handled here. 
 */
class Shield : public Component, public DamageableLayer, public EnergyConsumer {
    Resource<double> regeneration;
    Resource<double> power;   // 1.0 Full, 0.66 Two thirds, 0.0 Suppressed (FTL) or turned off

    // TODO: implement "shield leaks" aka
    // TODO: implement opacity
    //Resource<double> opacity;

    // TODO:
    //const bool shields_in_spec = configuration()->physics_config.shields_in_spec;
    //const float discharge_per_second = configuration()->physics_config.speeding_discharge;
    //const float min_shield_discharge = configuration()->physics_config.min_shield_speeding_discharge;
    //const float nebshields = configuration()->physics_config.nebula_shield_recharge;

    // Initialization of this should happen somewhere like main
    // Note that this implementation doesn't work right for asymmetric shields.
    // It would apply the same discharge value to all facets.
    // Currently disabled and applies regen value instead.
    static const bool shield_in_ftl = false;
    //static const double percent_shield_discharge_per_second = 0.5;
    //double atom_shield_discharge_per_second = 0.1;

    // TODO: Nebula shields
    // Shields are supposed to recharge slower in a nebula

    // Recharge energy and shields
    // Currently disabled and applies only to shield regen
    // Think about this some more
    // 
    const bool difficulty_modifier = 1.0; 

   
    friend class Damageable;
public:
    Shield(EnergyContainer *source);

    virtual void Load(std::string upgrade_key, std::string unit_key, 
                      Unit *unit, double difficulty);      // Load from dictionary
    virtual void SaveToCSV(std::map<std::string, std::string>& unit) const;  

    virtual std::string Describe() const; // Describe component in base_computer 

    virtual bool CanDowngrade() const;
    virtual bool CanUpgrade(const std::string upgrade_key)  const;
    virtual bool Downgrade();
    virtual bool Upgrade(const std::string upgrade_key);

    virtual void Damage();
    virtual void Repair();

    virtual bool Damaged() const;
    virtual bool Installed() const;

    void AdjustPower(const double &percent);
    void Disable();
    void Discharge();
    void Enable();
    bool Enabled() const;
    void Enhance();     // see collision enhancement
    double GetRegeneration() const;
    void Regenerate(bool ftl, bool player_ship);
    
    double GetPower() const;
    double GetPowerCap() const;
    void SetPower(const double power);
    void SetPowerCap(const double power);
};


#endif // VEGA_STRIKE_ENGINE_COMPONENTS_SHIELD_H
