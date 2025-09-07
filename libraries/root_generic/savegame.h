/*
 * savegame.h
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
#ifndef VEGA_STRIKE_ENGINE_SAVEGAME_H
#define VEGA_STRIKE_ENGINE_SAVEGAME_H

//#include "cmd/script/mission.h"

#include "gfx_generic/vec.h"

//WARNING, SAVE FILES ARE LIMITED TO MAXBUFFER SIZE !!! (LOOK IN NETWORKING/CONST.H)

#include <string>
#include <set>
#include <vector>
#include <map>
#include <boost/json.hpp>

#include "src/SharedPool.h"
#include "cmd/unit_type.h"

struct SaveGameValues {
    std::string FSS;
    std::string original_star_system;
    QVector PP;
    bool should_duplicate_date_position;
    std::vector<std::string> saved_starships;
    std::string save_contents;
    //std::vector select_data_filter;
};



struct SavedUnits {
    StringPool::Reference filename;
    Vega_UnitType type;
    StringPool::Reference faction;

    SavedUnits(const char *filen, int typ, const char *fact) {
        faction = std::string(fact);
        filename = std::string(filen);
        type = DeserializeUnitType(typ);
    }
};

class MissionStringDat {
public:
    typedef std::map<std::string, std::vector<std::string> > MSD;
    MSD m;
};


class MissionFloatDat {
public:
    typedef vsUMap<std::string, std::vector<float> > MFD;
    MFD m;
};

std::string GetWritePlayerSaveGame(int num);
void SaveFileCopy(const char *src, const char *dst);

class MissionFloatDat;
class SaveGame {
    SaveGame(const SaveGame &) {
    } //not used!
    std::string savestring;
    std::string ForceStarSystem;
    QVector PlayerLocation;
    std::string last_pickled_data;
    std::string last_written_pickled_data;
    std::string outputsavegame;
    std::string originalsystem;
    std::string callsign;
    
    
    void ReadStardate(char *&buf);
    
    MissionStringDat *missionstringdata;
    MissionFloatDat *missiondata;
    std::string playerfaction;
public:
    ~SaveGame();
    void ReloadPickledData();

    const std::string &GetCallsign() const {
        return callsign;
    }

    void SetCallsign(const std::string &cs) {
        callsign = cs;
    }

    /** Get read-write access to mission data */
    std::vector<float> &getMissionData(const std::string &magic_number);

    /** Get read-only access to mission data - note: returns empty stub if key isn't found */
    const std::vector<float> &readMissionData(const std::string &magic_number) const;

    /** Get mission data length (read-only) */
    unsigned int getMissionDataLength(const std::string &magic_number) const;

    /** Get read-write access to mission string data */
    std::vector<std::string> &getMissionStringData(const std::string &magic_number);

    /** Get read-only access to mission string data - note: returns empty stub if key isn't found */
    const std::vector<std::string> &readMissionStringData(const std::string &magic_number) const;

    /** Get mission string data length (read-only) */
    unsigned int getMissionStringDataLength(const std::string &magic_number) const;

    SaveGame(const std::string &pilotname);
    void PurgeZeroStarships();
    void SetPlayerLocation(const QVector &playerloc);
    QVector GetPlayerLocation();
    void SetStarSystem(std::string sys);
    std::string GetStarSystem();

    std::string GetPlayerFaction() {
        return playerfaction;
    }

    void SetPlayerFaction(std::string faction) {
        playerfaction = faction;
    }

    
    void ReadSavedPackets(char *&buf, bool commitfaction, bool skip_news = false, bool select_data = false,
            const std::set<std::string> &select_data_filter = std::set<std::string>());
    void SetOutputFileName(const std::string &filename);
    
    void LoadSavedMissions();

    // Write Save Game (JSON)
    // Implementation in save_game_json_utils.cpp
    private:
    void WriteNewsData(boost::json::object &save_game_json);
    void WriteMissionData(boost::json::object &save_game_json);
    void WriteMissionStringData(boost::json::object &save_game_json);
    void WritePlayerData(boost::json::object &save_game_json,
                        const QVector &FP,
                        const std::vector<std::string> unit_names_vector,
                        const std::string system_name);
    void WriteDynamicUniverse(boost::json::object &save_game_json);
    public:
    std::string WriteSaveGame(const std::string system_name,
                            const QVector &fighter_position,
                            const std::vector<std::string> unit_name,
                            int player_num);

    // Read Save Game (JSON)
    // Implementation in load_game_json_utils.cpp
    private:
    void ReadJsonNewsData(const boost::json::object &save_game_json);
    void ReadJsonMissionData(const boost::json::object &save_game_json);
    void ReadJsonMissionStringData(const boost::json::object &save_game_json);
    void ReadJsonPlayerData(const boost::json::object &save_game_json,
                                QVector &FP,
                                std::vector<std::string> &unit_names_vector);
    void ReadJsonDynamicUniverse(const boost::json::object &save_game_json);
    public:
    bool ReadJsonSaveGame(const std::string& file_contents,
                        QVector &fighter_position,
                        std::vector<std::string> &unit_names);

    // Read Save Game (Legacy/Binary)
    // Implementation in savegame.cpp
    private:
    void ReadNewsData(char *&buf, bool just_skip = false);
    void ReadMissionData(char *&buf, bool select_data = false,
            const std::set<std::string> &select_data_filter = std::set<std::string>());
    void ReadMissionStringData(char *&buf, bool select_data = false,
            const std::set<std::string> &select_data_filter = std::set<std::string>());
    public:
    float ParseSaveGame(const std::string &filename,
            std::string &ForceStarSystem,
            QVector &pos,
            bool &shouldupdatedfighter0pos,
            std::vector<std::string> &originalunit,
            int player_num);
    float ParseSaveGameInfo(const std::string &filename,
            std::string &ForceStarSystem,
            QVector &pos,
            bool &shouldupdatedfighter0pos,
            std::vector<std::string> &originalunit,
            int player_num,
            const std::string &savestr = std::string(),
            bool read = true,
            bool commitFaction = true,
            bool quick_read = false,
            bool skip_news = false,
            bool select_data = false,
            const std::set<std::string> &select_data_filter =
            std::set<std::string>());
};
void WriteSaveGame(class Cockpit *cp, bool auto_save);
const std::string &GetCurrentSaveGame();
std::string SetCurrentSaveGame(std::string newname);
const std::string &GetSaveDir();
void CopySavedShips(std::string filename, int player_num, const std::vector<std::string> &starships, bool load);
bool isUtf8SaveGame(std::string filename);


// Write Save Game (JSON)


#endif //VEGA_STRIKE_ENGINE_SAVEGAME_H
