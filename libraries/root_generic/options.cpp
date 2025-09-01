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

void vs_options::init() {
    /* Data Options */

    universe_path = vs_config->getVariable("data", "universe_path", "universe");
    sectors = vs_config->getVariable("data", "sectors", "sectors");
    techniquesBasePath = vs_config->getVariable("data", "techniques", "techniques");
    modUnitCSV = vs_config->getVariable("data", "ModUnitCSV", "");
    cockpits = vs_config->getVariable("data", "cockpits", "cockpits");
    animations = vs_config->getVariable("data", "animations", "animations");
    movies = vs_config->getVariable("data", "movies", "movies");
    sprites = vs_config->getVariable("data", "sprites", "sprites");
    serialized_xml = vs_config->getVariable("data", "serialized_xml", "serialized_xml");
    sharedtextures = vs_config->getVariable("data", "sharedtextures", "textures");
    sharedsounds = vs_config->getVariable("data", "sharedsounds", "sounds");
    sharedmeshes = vs_config->getVariable("data", "sharedmeshes", "meshes");
    sharedunits = vs_config->getVariable("data", "sharedunits", "units");
    ai_directory = vs_config->getVariable("data", "ai_directory", "ai");
    datadir = vs_config->getVariable("data", "datadir", "");
    hqtextures = vs_config->getVariable("data", "hqtextures", "");
    volume_format = vs_config->getVariable("data", "volume_format", "pk3");

    /* Galaxy Options */

    PushValuesToMean = XMLSupport::parse_bool(vs_config->getVariable("galaxy", "PushValuesToMean", "true"));
    AmbientLightFactor = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "AmbientLightFactor", "0"));
    StarRadiusScale = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "StarRadiusScale", "1000"));
    DefaultAtmosphereTexture =
            vs_config->getVariable("galaxy", "DefaultAtmosphereTexture", "sol/earthcloudmaptrans.png");
    AtmosphereProbability = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "AtmosphereProbability", "0.5"));
    RingProbability = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "RingProbability", ".1"));
    DoubleRingProbability = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "DoubleRingProbability", ".025"));
    InnerRingRadius = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "InnerRingRadius", "1.5"));
    OuterRingRadius = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "OuterRingRadius", "2.5"));
    DefaultRingTexture = vs_config->getVariable("galaxy", "DefaultRingTexture", "planets/ring.pngwrapx36wrapy2");
    SecondRingDifference = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "SecondRingDifference", ".4"));
    MoonRelativeToPlanet = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "MoonRelativeToPlanet", ".4"));
    MoonRelativeToMoon = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "MoonRelativeToMoon", ".8"));
    RockyRelativeToPrimary =
            XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "RockyRelativeToPrimary", ".05"));
    CompactnessScale = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "CompactnessScale", "1.5"));
    JumpCompactnessScale = XMLSupport::parse_floatf(vs_config->getVariable("galaxy", "JumpCompactnessScale", "1.5"));
    MeanNaturalPhenomena = XMLSupport::parse_int(vs_config->getVariable("galaxy", "MeanNaturalPhenomena", "1"));
    MeanStarBases = XMLSupport::parse_int(vs_config->getVariable("galaxy", "MeanStarBases", "2"));
//    SmallUnitsMultiplier   = XMLSupport::parse_floatf( vs_config->getVariable( "galaxy", "SmallUnitsMultiplier", "0" ) );

    /* Network Options */

    force_client_connect = XMLSupport::parse_bool(vs_config->getVariable("network", "force_client_connect", "false"));
    use_account_server = XMLSupport::parse_bool(vs_config->getVariable("network", "use_account_server", "true"));
    server_ip = vs_config->getVariable("network", "server_ip", "");
    server_port = vs_config->getVariable("network", "server_port", "6777");
    account_server_url = vs_config->getVariable("network",
            "account_server_url",
            "http://vegastrike.sourceforge.net/cgi-bin/accountserver.py?");
    chat_only_in_network = XMLSupport::parse_bool(vs_config->getVariable("network", "chat_only_in_network", "false"));
}

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
