/*
 * options.h
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
#ifndef VEGA_STRIKE_ENGINE_OPTIONS_H
#define VEGA_STRIKE_ENGINE_OPTIONS_H

#include <string>
#include <memory>

#ifndef uint
typedef unsigned int uint;
#endif

class vs_options {
public:
    vs_options() {
    }

    ~vs_options() {
    }

    void init();

    /* Data Options */

    std::string universe_path;
    std::string sectors;
    std::string techniquesBasePath;
    std::string unitCSV;
    std::string modUnitCSV;
    std::string cockpits;
    std::string animations;
    std::string movies;
    std::string sprites;
    std::string serialized_xml;
    std::string sharedtextures;
    std::string sharedsounds;
    std::string sharedmeshes;
    std::string sharedunits;
    std::string ai_directory;
    std::string datadir;
    std::string hqtextures;
    std::string volume_format;

    /* Galaxy Options */

    bool PushValuesToMean{};
    float AmbientLightFactor{};
    float StarRadiusScale{};
    std::string DefaultAtmosphereTexture;
    float AtmosphereProbability{};
    float RingProbability{};
    float DoubleRingProbability{};
    float InnerRingRadius{};
    float OuterRingRadius{};
    std::string DefaultRingTexture;
    float SecondRingDifference{};
    float MoonRelativeToPlanet{};
    float MoonRelativeToMoon{};
    float RockyRelativeToPrimary{};
    float CompactnessScale{};
    float JumpCompactnessScale{};
    int MeanNaturalPhenomena{};
    int MeanStarBases{};
//    float    SmallUnitsMultiplier;  not used


    /* Network Options */

    bool force_client_connect{};
    bool use_account_server{};
    std::string server_ip;
    std::string server_port;
    std::string account_server_url;
    bool chat_only_in_network{};

    // TODO: Refactor / move these
    std::string getCallsign(int);
    std::string getPlayer(int);
    std::string getPassword(int);
};

extern std::shared_ptr<vs_options> game_options();

#endif //VEGA_STRIKE_ENGINE_OPTIONS_H
