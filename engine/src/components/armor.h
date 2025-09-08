/*
 * armor.h
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_ARMOR_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_ARMOR_H

#include "component.h"
#include "damage/damageable_layer.h"

/** Armor component
 *
 * Always present and always implemented as a 4 facet DL.
 * If no armor, each facet is 0.
*/
class Armor : public Component, public DamageableLayer {
public:
    static int front;
    static int back;
    static int left;
    static int right;

    Armor();

    // Component Methods
    void Load(std::string unit_key) override;

    void SaveToCSV(std::map<std::string, std::string>& unit) const override;

    bool CanDowngrade() const override;

    bool Downgrade() override;

    bool CanUpgrade(const std::string upgrade_key) const override;

    bool Upgrade(const std::string upgrade_key) override;

    double PercentOperational() const override;

    bool Damaged() const override;
    void Repair() override;

    ~Armor() override;
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_ARMOR_H
