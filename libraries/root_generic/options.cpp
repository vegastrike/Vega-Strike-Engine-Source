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
    /* Audio Options */
    threadtime = XMLSupport::parse_int(vs_config->getVariable("audio", "threadtime", "1"));
    missionvictorysong = vs_config->getVariable("audio", "missionvictorysong", "../music/victory.ogg");
    Music = XMLSupport::parse_bool(vs_config->getVariable("audio", "Music", "true"));
    sound_gain = XMLSupport::parse_floatf(vs_config->getVariable("audio", "sound_gain", ".5"));
    audio_ref_distance = XMLSupport::parse_floatf(vs_config->getVariable("audio", "audio_ref_distance", "4000"));
    audio_max_distance = XMLSupport::parse_floatf(vs_config->getVariable("audio", "audio_max_distance", "1000000"));
    Doppler = XMLSupport::parse_bool(vs_config->getVariable("audio", "Doppler", "false"));
    Positional = XMLSupport::parse_bool(vs_config->getVariable("audio", "Positional", "true"));
    Volume = XMLSupport::parse_floatf(vs_config->getVariable("audio", "Volume", "100"));
    DopplerScale = XMLSupport::parse_floatf(vs_config->getVariable("audio", "DopplerScale", "1"));
    frequency = XMLSupport::parse_int(vs_config->getVariable("audio", "frequency", "48000"));
    MaxSingleSounds = XMLSupport::parse_int(vs_config->getVariable("audio", "MaxSingleSounds", "8"));
    MaxTotalSounds = XMLSupport::parse_int(vs_config->getVariable("audio", "MaxTotalSounds", "20"));
    Sound = XMLSupport::parse_bool(vs_config->getVariable("audio", "Sound", "true"));
    ai_sound = XMLSupport::parse_bool(vs_config->getVariable("audio", "ai_sound", "true"));
    explosion_closeness = XMLSupport::parse_floatf(vs_config->getVariable("audio", "explosion_closeness", ".8"));
    loss_relationship = XMLSupport::parse_floatf(vs_config->getVariable("audio", "loss_relationship", "-.1"));
    victory_relationship = XMLSupport::parse_floatf(vs_config->getVariable("audio", "victory_relationship", ".5"));
    time_between_music = XMLSupport::parse_floatf(vs_config->getVariable("audio", "time_between_music", "180"));

    /* Cockpit Audio Options */
    comm = vs_config->getVariable("cockpitaudio", "comm", "vdu_c");
    scanning = vs_config->getVariable("cockpitaudio", "scanning", "vdu_c");
    objective = vs_config->getVariable("cockpitaudio", "objective", "vdu_c");
    examine = vs_config->getVariable("cockpitaudio", "examine", "vdu_b");
    view = vs_config->getVariable("cockpitaudio", "view", "vdu_b");
    repair = vs_config->getVariable("cockpitaudio", "repair", "vdu_a");
    manifest = vs_config->getVariable("cockpitaudio", "manifest", "vdu_a");
    compress_max = XMLSupport::parse_int(vs_config->getVariable("cockpitaudio", "compress_max", "3"));
    compress_loop = vs_config->getVariable("cockpitaudio", "compress_loop", "compress_loop");
    compress_change = vs_config->getVariable("cockpitaudio", "compress_change", "compress_burst");
    compress_stop = vs_config->getVariable("cockpitaudio", "compress_stop", "compress_end");
    compress_interval = XMLSupport::parse_int(vs_config->getVariable("cockpitaudio", "compress_interval", "3"));

    comm_preload = XMLSupport::parse_bool(vs_config->getVariable("cockpitaudio", "comm_preload", "false"));

    /* Unit Audio Options */
    jumpleave = vs_config->getVariable("unitaudio", "jumpleave", "sfx43.wav");
    player_armor_hit = vs_config->getVariable("unitaudio", "player_armor_hit", "bigarmor.wav");
    player_hull_hit = vs_config->getVariable("unitaudio", "player_hull_hit", "bigarmor.wav");
    player_shield_hit = vs_config->getVariable("unitaudio", "player_shield_hit", "shieldhit.wav");

    /* AI Options */
    AllowCivilWar = XMLSupport::parse_bool(vs_config->getVariable("AI", "AllowCivilWar", "false"));
    CappedFactionRating = XMLSupport::parse_bool(vs_config->getVariable("AI", "CappedFactionRating", "true"));
    AllowNonplayerFactionChange =
            XMLSupport::parse_bool(vs_config->getVariable("AI", "AllowNonplayerFactionChange", "false"));
    min_relationship = XMLSupport::parse_float(vs_config->getVariable("AI", "min_relationship", "-20.0"));
    startDockedTo = vs_config->getVariable("AI", "startDockedTo", "MiningBase");
    dockOnLoad = XMLSupport::parse_bool(vs_config->getVariable("AI", "dockOnLoad", "true"));

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
