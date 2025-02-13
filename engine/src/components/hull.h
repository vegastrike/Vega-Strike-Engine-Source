/*
 * hull.h
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_HULL_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_HULL_H

#include "component.h"
#include "damage/damageable_layer.h"

/** Hull component */
class Hull : public Component, public DamageableLayer {
public:
    Hull();

    // Component Methods
    void Load(std::string unit_key) override;      
    
    void SaveToCSV(std::map<std::string, std::string>& unit) const;

    bool CanDowngrade() const override;

    bool Downgrade() override;

    bool CanUpgrade(const std::string upgrade_name) const override;

    bool Upgrade(const std::string upgrade_name) override;

    double PercentOperational() const override;

    double Percent() const;

    void Destroy() override;

    double Get();
    double GetMax();
    void Set(double value);
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_HULL_H
