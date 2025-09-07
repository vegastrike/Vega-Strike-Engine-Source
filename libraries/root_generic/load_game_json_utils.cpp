/*
 * load_game_json_utils.cpp
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

void SaveGame::ReadJsonNewsData(const boost::json::object &save_game_json) {
    // Clear the news
    // TODO: this is awful
    vector<string> n00s;
    n00s.push_back("news");
    vector<string> nada;
    mission->msgcenter->clear(n00s, nada);

    if (!save_game_json.if_contains("news")) return;
    const auto& news_array = save_game_json.at("news").as_array();
    mission->msgcenter->messages.clear();
    for (const auto& item : news_array) {
        const auto& news_object = item.as_object();
        if (!news_object.if_contains("from") || !news_object.if_contains("to") ||
            !news_object.if_contains("message") || !news_object.if_contains("time")) {
            continue;
        }
        
        gameMessage message_object;
        
        const std::string from = boost::json::value_to<const std::string>(news_object.at("from"));
        const std::string to = boost::json::value_to<const std::string>(news_object.at("to"));
        const std::string message = boost::json::value_to<const std::string>(news_object.at("message"));
        const double time = boost::json::value_to<const double>(news_object.at("time"));
        
        message_object.from.set(from);
        message_object.to.set(to);
        message_object.message.set(message);
        message_object.time = time;
        
        mission->msgcenter->messages.push_back(message_object);
    }
}

void SaveGame::ReadJsonMissionData(const boost::json::object &save_game_json) {
    missiondata->m.clear();

    if (!save_game_json.if_contains("mission_data")) return;

    const auto& mission_array = save_game_json.at("mission_data").as_array();

    // Mission Data array
    for (const auto& item : mission_array) {
        const auto& json_object = item.as_object();
        for (const auto& kv : json_object) {
            const std::string& key = kv.key();
            //const boost::json::array& array = kv.value().as_array();
            std::vector<float> vec = boost::json::value_to<std::vector<float>>(kv.value());
            /*for (const boost::json::value& item : array) {
                std::string obj = item.as_string().c_str();
                //vec.push_back(static_cast<float>(v.as_double()));
            }*/
            missiondata->m[key] = vec;
        }
    }
}

void SaveGame::ReadJsonMissionStringData(const boost::json::object &save_game_json) {
    if (!save_game_json.if_contains("mission_string_data")) return;
    const auto& mission_array = save_game_json.at("mission_string_data").as_array();
    missionstringdata->m.clear();
    for (const auto& item : mission_array) {
        const auto& obj = item.as_object();
        if (!obj.if_contains("key") || !obj.if_contains("value")) continue;
        std::string key = obj.at("key").as_string().c_str();
        std::vector<std::string> value;
        for (const auto& v : obj.at("value").as_array()) {
            value.push_back(v.as_string().c_str());
        }
        missionstringdata->m[key] = value;
    }
}

void SaveGame::ReadJsonPlayerData(const boost::json::object &save_game_json,
                              QVector &FP,
                              std::vector<std::string> &unit_names_vector) {
    // System Name
    if (save_game_json.if_contains("system")) {
        ForceStarSystem = save_game_json.at("system").as_string().c_str();
    }

    // Unit Names
    unit_names_vector.clear();
    if (save_game_json.if_contains("unit_names")) {
        const auto& unit_names = save_game_json.at("unit_names").as_array();
        for (const auto& name : unit_names) {
            unit_names_vector.push_back(name.as_string().c_str());
        }
    }

    // Faction
    this->playerfaction = string("privateer");;
    

    // Position
    if (save_game_json.if_contains("position")) {
        const auto& position = save_game_json.at("position").as_object();
        if (position.if_contains("i")) FP.i = position.at("i").as_double();
        if (position.if_contains("j")) FP.j = position.at("j").as_double();
        if (position.if_contains("k")) FP.k = position.at("k").as_double();
    }

    // Credits
    if (save_game_json.if_contains("credits"))
        ComponentsManager::credits = save_game_json.at("credits").as_double();
}

void SaveGame::ReadJsonDynamicUniverse(const boost::json::object &save_game_json) {
    // Read stardate
    if (save_game_json.if_contains("date")) {
        std::string star_date = save_game_json.at("date").as_string().c_str();
        _Universe->current_stardate.InitTrek(star_date);
    }

    // Read Mission Data
    ReadJsonMissionData(save_game_json);
    ReadJsonMissionStringData(save_game_json);

    // Read news data
    ReadJsonNewsData(save_game_json);

    // Faction relationships would be deserialized here if implemented
    // (No-op for now, as original WriteDynamicUniverse only prints)
}

bool SaveGame::ReadJsonSaveGame(const std::string& file_contents,
                            QVector &fighter_position,
                            std::vector<std::string> &unit_names) {
    boost::json::error_code ec;
    boost::json::value parsed = boost::json::parse(file_contents, ec);
    if (ec) {
        //VS_LOG(error, (boost::format("Error parsing JSON: %1%") % ec.message()));
        return false;
    }

    const auto& json_object = parsed.as_object();

    // Read player data
    ReadJsonPlayerData(json_object, fighter_position, unit_names);

    // Read dynamic universe data
    ReadJsonDynamicUniverse(json_object);
    return true;
}