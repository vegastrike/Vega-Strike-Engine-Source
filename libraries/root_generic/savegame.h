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
#include "src/SharedPool.h"
#include "cmd/unit_type.h"

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
class MissionFloatDat;
class MissionStringDat;
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
    std::string WriteMissionData();
    void WriteMissionStringData(std::vector<char> &ret);
    std::string WriteNewsData();
    void ReadStardate(char *&buf);
    void ReadNewsData(char *&buf, bool just_skip = false);
    void ReadMissionData(char *&buf, bool select_data = false,
            const std::set<std::string> &select_data_filter = std::set<std::string>());
    void ReadMissionStringData(char *&buf, bool select_data = false,
            const std::set<std::string> &select_data_filter = std::set<std::string>());
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
    float GetSavedCredits();
    void SetSavedCredits(float);
    void SetPlayerLocation(const QVector &playerloc);
    QVector GetPlayerLocation();
    void SetStarSystem(std::string sys);
    std::string GetStarSystem();
    std::string GetOldStarSystem();

    std::string GetPlayerFaction() {
        return playerfaction;
    }

    void SetPlayerFaction(std::string faction) {
        playerfaction = faction;
    }

    std::string WriteSavedUnit(SavedUnits *su);
    std::string WriteSaveGame(const char *systemname,
            const QVector &Pos,
            float credits,
            std::vector<std::string> unitname,
            int player_num,
            std::string fact = "",
            bool write = true);
    std::string WritePlayerData(const QVector &FP,
            std::vector<std::string> unitname,
            const char *systemname,
            float credits,
            std::string fact = "");
    std::string WriteDynamicUniverse();
    void ReadSavedPackets(char *&buf, bool commitfaction, bool skip_news = false, bool select_data = false,
            const std::set<std::string> &select_data_filter = std::set<std::string>());
///cast address to long (for 64 bits compatibility)
    void AddUnitToSave(const char *unitname, int type, const char *faction, long address);
    void RemoveUnitFromSave(long address); //cast it to a long
    void SetOutputFileName(const std::string &filename);
    void ParseSaveGame(const std::string &filename,
            std::string &ForceStarSystem,
            const std::string &originalstarsystem,
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
    void LoadSavedMissions();
};
void WriteSaveGame(class Cockpit *cp, bool auto_save);
const std::string &GetCurrentSaveGame();
std::string SetCurrentSaveGame(std::string newname);
const std::string &GetSaveDir();
void CopySavedShips(std::string filename, int player_num, const std::vector<std::string> &starships, bool load);
bool isUtf8SaveGame(std::string filename);

#endif //VEGA_STRIKE_ENGINE_SAVEGAME_H
