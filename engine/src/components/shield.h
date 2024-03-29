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

class Unit;

/* This class builds on top of DamageableLayer to represent a shield.
 * Shield functions (e.g. opacity, regeneration) are handled here.
 * DamageableLayer and Facet do not handle these at all.
 * Damage to the shield generator is handled here. 
 */
class Shield : public Component, public DamageableLayer {
    Resource<double> regeneration;
    Resource<double> power;   // 1.0 Full, 0.66 Two thirds, 0.0 Suppressed (FTL) or turned off

    // TODO: implement "shield leaks" aka
    // TODO: implement opacity
    //Resource<double> opacity;
    friend class Damageable;
public:
    Shield();

    virtual void Load(std::string upgrade_key, std::string unit_key, 
                      Unit *unit);      // Load from dictionary
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
    void Regenerate();
    
    double GetPower() const;
    double GetPowerCap() const;
    void SetPower(const double power);
    void SetPowerCap(const double power);
};


#endif // VEGA_STRIKE_ENGINE_COMPONENTS_SHIELD_H
