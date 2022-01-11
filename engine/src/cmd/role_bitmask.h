/**
* role_bitmask.h
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

#ifndef _ROLE_BITMASK_H_
#define _ROLE_BITMASK_H_
#include <string>
#include <vector>
namespace ROLES
{
const unsigned int FIRE_ONLY_AUTOTRACKERS = (1<<31);
const unsigned int FIRE_MISSILES   = (1<<30);
const unsigned int FIRE_GUNS       = (1<<29);
const unsigned int EVERYTHING_ELSE = ( ~(FIRE_MISSILES|FIRE_GUNS|FIRE_ONLY_AUTOTRACKERS) );
int discreteLog( int bitmask );
std::vector< char >& getPriority( unsigned char rolerow );
//get the list of all prirotieis 31 is nogo
std::vector< std::vector< char > >& getAllRolePriorities();
unsigned int maxRoleValue();
//getRole returns a number between 0 and 32 that's the discrete log
unsigned char getRole( const std::string &role );
const std::string& getRole( unsigned char role );

//returns the getRoles or'd together
unsigned int readBitmask( const std::string &s );
unsigned int getCapitalRoles();
const std::string& getRoleEvents( unsigned char ourrole, unsigned char theirs );        //get AI ervent associated with this
}
#endif

