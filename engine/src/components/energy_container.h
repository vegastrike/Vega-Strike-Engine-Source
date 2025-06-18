/*
 * energy_container.h
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_ENERGYCONTAINER_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_ENERGYCONTAINER_H

#include <vector>
#include <iostream>

#include "resource/resource.h"
#include "component.h"

/* Discussion of FTL - yes, it isn't really faster. However, 
* it would be easier for a new developer or someone from WC
* to figure what it means.
*/


/**
 * @brief The EnergyContainer class models the fuel cell, capacitor and SPEC capacitor
 */
class EnergyContainer: public Component
{
private:
    Resource<double> level;

public:
    EnergyContainer(ComponentType type);

    // Return value - any surplus charge
    double Charge(const double quantity);

    // Partial - can power consumer with less energy than requested
    double Deplete(bool partial, const double quantity);
    bool Depleted() const;

    void SetCapacity(const double capacity, bool refill = true);
    double Level() const;
    void SetLevel(double new_level);
    double MaxLevel() const;
    double Percent() const;
    void Refill();

    void Zero();

    // Component
    void Load(std::string unit_key) override;      
    
    void SaveToCSV(std::map<std::string, std::string>& unit) const override;

    bool CanDowngrade() const override;

    bool Downgrade() override;

    bool CanUpgrade(const std::string upgrade_key) const override;

    bool Upgrade(const std::string upgrade_key) override;

    void Damage() override;
    void DamageByPercent(double percent) override;

    void Repair() override;

    bool Damaged() const override;
    bool Installed() const override;
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_ENERGYCONTAINER_H
