/**
* save_util.h
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __SAVE_UTIL_H__
#define __SAVE_UTIL_H__

#include <string>
using std::string;

extern const char *mission_key;

Unit * DockToSavedBases( int playernum, QVector &safevec );


/*
 * C++ versions of some useful and comfy Python API
 */

float getSaveData( int whichcp, const std::string &key, unsigned int num );
const std::vector< float >& getSaveData( int whichcp, const std::string &key );
std::string getSaveString( int whichcp, const std::string &key, unsigned int num );
unsigned int getSaveDataLength( int whichcp, const std::string &key );
unsigned int getSaveStringLength( int whichcp, const std::string &key );
unsigned int pushSaveData( int whichcp, const std::string &key, float val );
unsigned int eraseSaveData( int whichcp, const std::string &key, unsigned int index );
unsigned int clearSaveData( int whichcp, const std::string &key );
unsigned int pushSaveString( int whichcp, const std::string &key, const std::string &value );
void putSaveString( int whichcp, const std::string &key, unsigned int num, const std::string &val );
void putSaveData( int whichcp, const std::string &key, unsigned int num, float val );
unsigned int eraseSaveString( int whichcp, const std::string &key, unsigned int index );
unsigned int clearSaveString( int whichcp, const std::string &key );
std::vector< std::string >loadStringList( int playernum, const std::string &mykey );
const std::vector< std::string >& getStringList( int playernum, const std::string &mykey );
void saveStringList( int playernum, const std::string &mykey, const std::vector< std::string > &names );
void saveDataList( int playernum, const std::string &mykey, const std::vector< float > &values );

#endif

