/*
 * cloak.h
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

#ifndef VEGA_STRIKE_ENGINE_COMPONENTS_CLOAK_H
#define VEGA_STRIKE_ENGINE_COMPONENTS_CLOAK_H

#include <string>
#include <map>

#include "components/energy_consumer.h"

// TODO: remove dependency on unit
class EnergyContainer;

enum class CloakingStatus {
    disabled,
    damaged,
    ready,
    cloaking,
    cloaked,
    decloaking
};

class Cloak : public Component, public EnergyConsumer
{
    friend class Unit;

    CloakingStatus status;

    // How fast does this starship cloak/decloak
    double rate;

    // If this unit cloaks like glass or like fading.
    // Glass is alpha only. Non glass affects rgb as well.
    bool glass;

    // Current cloak value. 0 is uncloaked. 1 is fully cloaked.
    double current;

    // The minimum cloaking value...
    double minimum;

public:
    Cloak();

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

    bool Damaged() const override;
    bool Installed() const override;

// Cloak Methods

    void Update();
    void Toggle(); // Toggle cloak on/off

    bool Capable() const {
        return (status != CloakingStatus::disabled);
    }

    bool Cloaking() {
        return (status == CloakingStatus::cloaking);
    }

    bool Cloaked() const {
        return (status == CloakingStatus::cloaked);
    }

    // Active is cloaking, cloaked or decloaking
    bool Active() const {
        return (status == CloakingStatus::cloaking ||
                status == CloakingStatus::cloaked ||
                status == CloakingStatus::decloaking);
    }

    bool Ready() const {
        return (status == CloakingStatus::ready);
    }

    bool Glass() const {
        return glass;
    }



    // Is the ship visible
    bool Visible() const {
        return !Cloaked();
    }

    double Current() const {
        return current;
    }

    //how visible the ship is from 0 to 1
    double Visibility() const {
        return 1-current;
    }

    void Disable() {
        status = CloakingStatus::disabled;
        current = 0;
    }

    void Enable() {
        status = CloakingStatus::ready;
        current = 0;
    }

    void Activate();
    void Deactivate();

private:
    void _Downgrade();
    void _Upgrade(const std::string upgrade_key);
};

#endif // VEGA_STRIKE_ENGINE_COMPONENTS_CLOAK_H
