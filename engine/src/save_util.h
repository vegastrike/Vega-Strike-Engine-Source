/*
 * save_util.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_SAVE_UTIL_H
#define VEGA_STRIKE_ENGINE_SAVE_UTIL_H

#include <string>
using std::string;

extern const char *mission_key;

Unit *DockToSavedBases(int playernum, QVector &safevec);

/*
 * C++ versions of some useful and comfy Python API
 */

float getSaveData(int whichcp, const std::string &key, unsigned int num);
const std::vector<float> &getSaveData(int whichcp, const std::string &key);
std::string getSaveString(int whichcp, const std::string &key, unsigned int num);
unsigned int getSaveDataLength(int whichcp, const std::string &key);
unsigned int getSaveStringLength(int whichcp, const std::string &key);
unsigned int pushSaveData(int whichcp, const std::string &key, float val);
unsigned int eraseSaveData(int whichcp, const std::string &key, unsigned int index);
unsigned int clearSaveData(int whichcp, const std::string &key);
unsigned int pushSaveString(int whichcp, const std::string &key, const std::string &value);
void putSaveString(int whichcp, const std::string &key, unsigned int num, const std::string &val);
void putSaveData(int whichcp, const std::string &key, unsigned int num, float val);
unsigned int eraseSaveString(int whichcp, const std::string &key, unsigned int index);
unsigned int clearSaveString(int whichcp, const std::string &key);
std::vector<std::string> loadStringList(int playernum, const std::string &mykey);
const std::vector<std::string> &getStringList(int playernum, const std::string &mykey);
void saveStringList(int playernum, const std::string &mykey, const std::vector<std::string> &names);
void saveDataList(int playernum, const std::string &mykey, const std::vector<float> &values);

#endif //VEGA_STRIKE_ENGINE_SAVE_UTIL_H

