/*
 * options.cpp
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

#include "root_generic/options.h"
#include "root_generic/configxml.h"
#include "configuration/configuration.h"

extern VegaConfig *vs_config;

string vs_options::getCallsign(int squadnum) {
    return vs_config->getVariable(std::string("player")
            + ((squadnum > 0) ? XMLSupport::tostring(squadnum + 1) : std::string(
                    "")), "callsign", "pilot");
}

string vs_options::getPlayer(int playernum) {
    return (vs_config->getVariable("player" + ((playernum > 0) ? XMLSupport::tostring(playernum + 1) : string("")),
            "callsign",
            ""));
}

string vs_options::getPassword(int playernum) {
    return (vs_config->getVariable("player" + ((playernum > 0) ? XMLSupport::tostring(playernum + 1) : string("")),
            "password",
            ""));
}

std::shared_ptr<vs_options> game_options() {
    static const std::shared_ptr<vs_options> GAME_OPTIONS = std::make_shared<vs_options>();
    return GAME_OPTIONS;
}
