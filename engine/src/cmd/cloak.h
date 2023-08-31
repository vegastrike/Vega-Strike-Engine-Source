/*
 * cloak.h
 *
 * Copyright (C) 2001-2023 Daniel Horn, Benjaman Meyer, Roy Falk, Stephen G. Tuggy,
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

#include "energetic.h"
#include "damageable_layer.h"

enum class CloakingStatus {
    disabled,
    damaged,
    ready,
    cloaking,
    cloaked,
    decloaking
};

class Cloak
{
    friend class Unit;

    CloakingStatus status;

    // How much energy cloaking takes per frame
    double energy;

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
    Cloak(std::string unit_key);

    void Update(Energetic *energetic);
    void Toggle(); // Toggle cloak on/off

    bool Capable() const {
        return (status != CloakingStatus::disabled);
    }

    bool Cloaking() {
        return (status == CloakingStatus::cloaking);
    }

    bool Cloaked() {
        return (status == CloakingStatus::cloaked);
    }

    // Active is cloaking, cloaked or decloaking
    bool Active() {
        return (status == CloakingStatus::cloaking ||
                status == CloakingStatus::cloaked ||
                status == CloakingStatus::decloaking);
    }

    bool Damaged() {
        return (status == CloakingStatus::damaged);
    }

    bool Ready() {
        return (status == CloakingStatus::ready);
    }

    bool Glass() {
        return glass;
    }

    double Current() {
        return current;
    }

    double Energy() {
        return energy;
    }

    //how visible the ship is from 0 to 1
    double Visible() const {
        return 1-current;
    }

    // TODO: more granular damage
    // TODO: damageable component
    void Damage() {
        status = CloakingStatus::damaged;
        current = 0;
    }

    void Repair() {
        if(status == CloakingStatus::damaged) {
            status = CloakingStatus::ready;
            current = 0;
        }
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
};

#endif // CLOAK_H
