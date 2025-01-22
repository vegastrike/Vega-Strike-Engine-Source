/*
 * afterburner.h
 *
 * Copyright (C) 2001-2023 Daniel Horn, Benjamen Meyer, Roy Falk, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_AFTERBURNER_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_AFTERBURNER_H

#include "component.h"
#include "energy_consumer.h"

class EnergyContainer;

/** We split this class from Drive for one reason - it may use a different source. */
class Afterburner : public Component, public EnergyConsumer {
public:
    //after burner acceleration max
    Resource<double> thrust;

    Resource<double> speed;

    Afterburner(EnergyContainer *source = nullptr);
    
    // Component Methods
    void Load(std::string unit_key) override;      
    
    void SaveToCSV(std::map<std::string, std::string>& unit) const override;

    bool CanDowngrade() const override;
    bool Downgrade() override;
    bool CanUpgrade(const std::string upgrade_key) const override;
    bool Upgrade(const std::string upgrade_key) override;

    void Damage() override;
    void DamageByPercent(double percent) override;
    void Repair() override;
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_AFTERBURNER_H
