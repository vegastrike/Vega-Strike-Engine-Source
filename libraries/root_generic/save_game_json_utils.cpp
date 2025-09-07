/*
 * save_game_json_utils.cpp
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

#include "root_generic/savegame.h"

#include <iostream>
#include <boost/json.hpp>

#include "vsfilesystem.h"
#include "cmd/script/msgcenter.h"
#include "src/vs_logging.h"
#include "root_generic/vs_globals.h"
#include "cmd/script/mission.h"
#include "components/components_manager.h"
#include "load_mission.h"
#include "faction_generic.h"
#include "src/universe.h"

using namespace VSFileSystem;

void SaveGame::WriteNewsData(boost::json::object &save_game_json) {
    boost::json::array news_array;
    for(const gameMessage& message : mission->msgcenter->messages) {
        boost::json::object news_object = {
            {"from", message.from.get()},
            {"to", message.to.get()},
            {"message", message.message.get()},
            {"time", message.time}
        };
        news_array.push_back(news_object);
    }
    save_game_json["news"] = news_array;
}

void SaveGame::WriteMissionData(boost::json::object &save_game_json) {
    boost::json::array mission_array;
    for (const auto &mission : missiondata->m) {
        if(mission.second.empty()) {
            continue;
        }

        boost::json::object json_object;
        json_object[mission.first] = boost::json::value_from(mission.second);
        mission_array.push_back(json_object);
    }
    save_game_json["mission_data"] = mission_array;
}

void SaveGame::WriteMissionStringData(boost::json::object &save_game_json) {
    boost::json::array mission_array;
    for(const auto& mission : missionstringdata->m) {
        const std::string key = mission.first;
        std::vector<std::string> value = mission.second;

        if(value.empty()) {
            continue;
        }

        if (key == "mission_descriptions" || key == "mission_scripts" || key == "mission_vars"
            || key == "mission_names") {
            //*** BLACKLIST ***
            //Don't bother to write these out since they waste a lot of space and aren't used.
            value.clear(); //Not writing them out altogether will cause saved games to break.
        }

        boost::json::object object = {
            {"key", key},
            {"value", boost::json::value_from(value)}
        };

        mission_array.push_back(object);
    }
    save_game_json["mission_string_data"] = mission_array;
}

void SaveGame::WritePlayerData(boost::json::object &save_game_json,
                                 const QVector &FP,
                                 const std::vector<std::string> unit_names_vector,
                                 const std::string system_name) {
    // System Name
    save_game_json["system"] = system_name;

    // Unit Name
    boost::json::value unit_names = boost::json::value_from(unit_names_vector);
    save_game_json["unit_names"] = unit_names;

    // Position
    // This code does nothing!
    QVector fighter_position = PlayerLocation - FP;
    fighter_position = FP;
    assert(FP.i == fighter_position.i);
    assert(FP.j == fighter_position.j);
    assert(FP.k == fighter_position.k);

    boost::json::object position;
    position["i"] = FP.i;
    position["j"] = FP.j;
    position["k"] = FP.k;

    save_game_json["position"] = position;

    // Credits
    save_game_json["credits"] = ComponentsManager::credits;
}

void SaveGame::WriteDynamicUniverse(boost::json::object &save_game_json) {
    //Write mission data
    //we save the stardate
    // TODO: consider changing this
    std::string star_date = _Universe->current_stardate.GetFullTrekDate();
    save_game_json["date"] = star_date;

    // Write Mission Data
    WriteMissionData(save_game_json);
    WriteMissionStringData(save_game_json);
    if (!STATIC_VARS_DESTROYED) {
        std::cout << PickleAllMissions() << std::endl;
    }
    
    //Write news data
    WriteNewsData(save_game_json);

    //Write faction relationships
    std::cout << FactionUtil::SerializeFaction() << std::endl;  
}

std::string SaveGame::WriteSaveGame(const std::string system_name,
                               const QVector &fighter_position,
                               const std::vector<std::string> unit_names,
                               int player_num) {
    VS_LOG(info, (boost::format("Writing Save Game %1%") % outputsavegame));

    boost::json::object json_object;
    WritePlayerData(json_object, fighter_position, unit_names, system_name);
    WriteDynamicUniverse(json_object);
    savestring = boost::json::serialize(json_object);
    if (outputsavegame.length() != 0) {
        if (!write) {
            return savestring;
        }
            
        VSFile f;
        VSError err = f.OpenCreateWrite(outputsavegame, SaveFile);
        if (err <= Ok) {
            //check
            //WRITE THE SAVEGAME TO THE MISSION SAVENAME
            f.Write(savestring.c_str(), savestring.length());
            f.Close();
            if (player_num != -1) {
                //AND THEN COPY IT TO THE SPECIFIED SAVENAME (from save.4.x.txt)
                last_pickled_data = last_written_pickled_data;
                std::string sg = GetWritePlayerSaveGame(player_num);
                SaveFileCopy(outputsavegame.c_str(), sg.c_str());
            }
        } else {
            //error occured while opening file
            VS_LOG(error, (boost::format("Error occurred while opening file: %1%") % outputsavegame));
        }
    }
    return savestring;
}