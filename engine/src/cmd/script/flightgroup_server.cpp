/*
 * flightgroup_server.cpp
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


#include "mission.h"
#include "flightgroup.h"
#include "cmd/unit_generic.h"
#include <stdio.h>

Flightgroup *Flightgroup::newFlightgroup(const std::string &name,
        const std::string &type,
        const std::string &faction,
        const std::string &order,
        int num_ships,
        int num_waves,
        const std::string &logo_tex,
        const std::string &logo_alp,
        Mission *mis) {
    Flightgroup *fg = mis->findFlightgroup(name, faction);
    Flightgroup *fgtmp = fg;
    if (fg == NULL) {
        fg = new Flightgroup;
    }
    fg->Init(fgtmp, name, type, faction, order, num_ships, num_waves, mis);
    if (!logo_tex.empty()) {
        if (logo_alp.empty()) {
            fg->squadLogoStr = logo_tex;
        } else {
            fg->squadLogoStr = logo_alp;
        }
    }
    return fg;
}

Flightgroup &Flightgroup::operator=(Flightgroup &other) {
    printf("warning: Flightgroup::operator= may *really* not work properly.\n"
           "In fact, this shouldn't be called at all anyway!\n");
    return other;
}

Flightgroup::~Flightgroup() {
}

