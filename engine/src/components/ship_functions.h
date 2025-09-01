/*
 * ship_functions.h
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_SHIP_FUNCTIONS_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_SHIP_FUNCTIONS_H

#include "component.h"
#include "resource/resource.h"

enum class Function {
    cockpit, communications, fire_control, life_support, ftl_interdiction
};

/** A catch-all component for anything not part of another component.
 *  Cannot be upgraded or sold. Can be repaired. */
class ShipFunctions : public Component {
    Resource<double> cockpit;
    Resource<double> communications;
    Resource<double> fire_control;
    Resource<double> life_support;
    double ftl_interdiction = 0;

public:
    ShipFunctions();

    // Component Methods
    void Load(std::string unit_key) override;

    void SaveToCSV(std::map<std::string, std::string>& unit) const override;

    bool CanDowngrade() const override;

    bool Downgrade() override;

    bool CanUpgrade(const std::string upgrade_key) const override;

    bool Upgrade(const std::string upgrade_key) override;

    void Repair() override;

    // Ship Function Methods
    double Value(Function function) const;
    double Max(Function function) const;
    double Percent(Function function) const;

    void Damage(Function function);
    void Repair(Function function);

    std::string GetHudText(std::string getDamageColor(double));

    ~ShipFunctions() override;
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_SHIP_FUNCTIONS_H
