/*
 * role_bitmask.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_ROLE_BITMASK_H
#define VEGA_STRIKE_ENGINE_CMD_ROLE_BITMASK_H

#include <string>
#include <vector>

namespace ROLES {
const unsigned int FIRE_ONLY_AUTOTRACKERS = (1 << 31);
const unsigned int FIRE_MISSILES = (1 << 30);
const unsigned int FIRE_GUNS = (1 << 29);
const unsigned int EVERYTHING_ELSE = (~(FIRE_MISSILES | FIRE_GUNS | FIRE_ONLY_AUTOTRACKERS));
int discreteLog(int bitmask);
std::vector<char> &getPriority(unsigned char rolerow);
//get the list of all prirotieis 31 is nogo
std::vector<std::vector<char> > &getAllRolePriorities();
unsigned int maxRoleValue();
//getRole returns a number between 0 and 32 that's the discrete log
unsigned char getRole(const std::string &role);
const std::string &getRole(unsigned char role);

//returns the getRoles or'd together
unsigned int readBitmask(const std::string &s);
unsigned int getCapitalRoles();
const std::string &getRoleEvents(unsigned char ourrole,
        unsigned char theirs);        //get AI ervent associated with this
}
#endif //VEGA_STRIKE_ENGINE_CMD_ROLE_BITMASK_H
