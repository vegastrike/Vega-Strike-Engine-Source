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

#ifndef CLOAK_H
#define CLOAK_H

#include <string>
#include <map>

#include "energetic.h"
#include "damageable_layer.h"
#include "energy_container.h"
#include "energy_consumer.h"

// TODO: remove dependency on unit
class Unit;

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
    Cloak(std::string unit_key, EnergyContainer* capacitor);

    virtual void SaveToCSV(std::map<std::string, std::string>& unit) const;
    virtual std::string Describe() const;

    virtual bool CanDowngrade() const;

    virtual bool Downgrade();

    virtual bool CanUpgrade(const std::string upgrade_key) const;

    virtual bool Upgrade(const std::string upgrade_key);

    virtual void Damage();
    virtual void DamageByPercent(double percent);
    virtual void Repair();

    virtual bool Damaged() const;
    virtual bool Installed() const;

    void Update(Unit *unit);
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
    bool Active() {
        return (status == CloakingStatus::cloaking ||
                status == CloakingStatus::cloaked ||
                status == CloakingStatus::decloaking);
    }

    bool Ready() {
        return (status == CloakingStatus::ready);
    }

    bool Glass() {
        return glass;
    }

    

    // Is the ship visible
    bool Visible() {
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

#endif // CLOAK_H
