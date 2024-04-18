/*
 * afterburner.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2023 Stephen G. Tuggy, Benjamen R. Meyer, Roy Falk and other Vega Strike Contributors
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

#ifndef AFTERBURNER_H
#define AFTERBURNER_H

#include <string>
#include <map>

#include "component.h"
#include "energy_container.h"
#include "energy_consumer.h"

// TODO: Take ship size, mass, cost, something into account.
// TODO: figure out the whole type thing.

/* @discussion Afterburners are extra engines or a feature of existing 
    engines that accelrate faster and achieve a top higher speed.
    Overdrive upgrade is an upgrade to improve the performance of 
    the afterburners.
    By default, ships do not have afterburners. If they do, it must be
    integrated and cannot be upgraded.
*/
class Afterburner : public Component, public EnergyConsumer
{
    int type = 0;
    double usage_cost = 1.0;        // Multiplier
    double acceleration = 1.0;      // Multiplier
    double speed_governor = 1.0;    // Multiplier
    double usage_factor = 1.0;

    double atomic_drive_usage = 1.0;
    /*  Discussion of fuel usage
        Afterburner fuel usage is based on normal drive usage.
        Therefore, we need this data here as well.
    */
public:
    Afterburner(EnergyContainer *source, double usage_factor = 1.0);

    // Component
    virtual void Load(std::string upgrade_key, std::string unit_key);      
    
    virtual void SaveToCSV(std::map<std::string, std::string>& unit) const;

    virtual std::string Describe() const; // Describe component in base_computer 

    virtual bool CanDowngrade() const;

    virtual bool Downgrade();

    virtual bool CanUpgrade(const std::string upgrade_name) const;

    virtual bool Upgrade(const std::string upgrade_name);

    virtual void Damage();
    virtual void Repair();

    virtual bool Damaged() const;
    virtual bool Installed() const;

    //void WriteUnitString(std::map<std::string, std::string> &unit);
    double UsageFactor() { return usage_factor; }
};

#endif // AFTERBURNER_H