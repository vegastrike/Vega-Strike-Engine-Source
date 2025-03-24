/**
 * unit_util_server.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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


#include <string>
#include "cmd/unit_generic.h"
#include "root_generic/configxml.h"
#include "unit_util.h"
#include "root_generic/vs_globals.h"
using std::string;

namespace UnitUtil {
void switchFg(Unit *my_unit, string arg) {
    if (!my_unit) {
        return;
    }
    string type = my_unit->name;
    int nr_waves_left = 0;
    int nr_ships = 1;
    string order("default");
    Flightgroup *fg = my_unit->getFlightgroup();
    if (fg) {
        type = fg->type;
        nr_waves_left = fg->nr_waves_left;
        nr_ships = 1;
        fg->Decrement(my_unit);
        order = fg->ainame;
    }
    fg = Flightgroup::newFlightgroup(arg, type, FactionUtil::GetFaction(
            my_unit->faction), order, nr_ships, nr_waves_left, "", "", mission);
    my_unit->SetFg(fg, fg->nr_ships_left - 1);
}

int communicateTo(Unit *my_unit, Unit *other_unit, float mood) {
    return 0;
}

bool commAnimation(Unit *my_unit, string anim) {
    return false;
}
}

