/**
 * energetic.cpp
 *
 * Copyright (C) 2020-2022 Daniel Horn, Roy Falk, Stephen G. Tuggy, and
 * other Vega Strike contributors
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

#include "cmd/energetic.h"

#include "configuration/game_config.h"
#include "configuration/configuration.h"
#include "cmd/damageable.h"
#include "src/vegastrike.h"
#include "cmd/unit_generic.h"
#include "src/universe.h"
#include "resource/resource.h"
#include "src/vega_cast_utils.h"

#include <algorithm>

/* This class provides all energy generation methods to unit types -
 * ships, space installations, missiles, drones, etc. */


Energetic::Energetic() :
        constrained_charge_to_shields(0.0f),
        sufficient_energy_to_recharge_shields(true),
        afterburntype(0) {
}


void Energetic::DecreaseWarpEnergyInWarp() {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);

    if (!unit->ftl_drive.Enabled()) {
        return;
    }

    if(unit->ftl_drive.CanConsume()) {
        unit->ftl_drive.Consume();
    } else {
        unit->ftl_drive.Disable();
        unit->graphicOptions.WarpRamping = 1;
    }
}



float Energetic::getFuelUsage(bool afterburner) {
    if (afterburner) {
        return vega_config::config->fuel.afterburner_fuel_usage;
    }
    return vega_config::config->fuel.normal_fuel_usage;
}


void Energetic::setEnergyRecharge(float enrech) {
    Unit *unit = vega_dynamic_cast_ptr<Unit>(this);
    unit->reactor.SetCapacity(enrech);
}


float Energetic::VSDPercent() {
    return vega_config::config->fuel.vsd_mj_yield / 100;
}
