/*
 * shield.h
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_SHIELD_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_SHIELD_H

#include "component.h"
#include "energy_consumer.h"
#include "damage/damageable_layer.h"

class FtlDrive;
class Cloak;

/** Shield component */
class Shield : public Component, public EnergyConsumer, public DamageableLayer {
    const FtlDrive *ftl_drive;
    const Cloak *cloak;
    Resource<double> regeneration; // The current capability of a potentially damaged part
    Resource<double> max_power; // Used to manually throttle shield
public:
    static int front;
    static int back;
    static int left;
    static int right;

    Shield(EnergyContainer *source, FtlDrive *ftl_drive, Cloak *cloak, FacetConfiguration configuration = FacetConfiguration::four);

    // Component Methods
    void Load(std::string unit_key) override;      
    void SaveToCSV(std::map<std::string, std::string>& unit) const override;

    bool CanDowngrade() const override;
    bool Downgrade() override;
    bool CanUpgrade(const std::string upgrade_key) const override;
    bool Upgrade(const std::string upgrade_key) override;

    void Damage();
    void DamageByPercent(double percent);
    void Repair();

    void Regenerate(const bool player_ship);

    // Manipulate shield strength
    void AdjustPower(double percent);
    void FullyCharge();
    void Decrease();
    void Zero();
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_SHIELD_H
