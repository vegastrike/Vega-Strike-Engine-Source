/*
 * mount_size.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy, David Wales
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
#ifndef VEGA_STRIKE_ENGINE_CMD_MOUNT_SIZE_H
#define VEGA_STRIKE_ENGINE_CMD_MOUNT_SIZE_H

#include <string>

enum class MOUNT_SIZE {
    NOWEAP = 0x0,

    LIGHT = 0x1,
    MEDIUM = 0x2,
    HEAVY = 0x4,
    CAPSHIPLIGHT = 0x8,
    CAPSHIPHEAVY = 0x10,
    SPECIAL = 0x20,

    LIGHTMISSILE = 0x40,
    MEDIUMMISSILE = 0x80,
    HEAVYMISSILE = 0x100,
    CAPSHIPLIGHTMISSILE = 0x200,
    CAPSHIPHEAVYMISSILE = 0x400,
    SPECIALMISSILE = 0x800,

    AUTOTRACKING = 0x1000
};

template<typename Enumeration>
auto as_integer(Enumeration const value)
-> typename std::underlying_type<Enumeration>::type {
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

MOUNT_SIZE getMountSizeFromItsValue(int value);
MOUNT_SIZE getMountSize(const std::string &mount_string);
int getMountSizes(const std::string &mounts_string);
std::string getMountSizeString(const int mount);

bool isNormalGunMount(const int mount);
bool isNormalMissileMount(const int mount);

bool isNormalMount(const int mount);

bool isSpecialMount(const int mount);
bool isSpecialGunMount(const int mount);
bool isSpecialMissileMount(const int mount);

bool isGunMount(const int mount);
bool isMissileMount(const int mount);

bool isAutoTrackingMount(const int mount);

class MountSize {
public:
    MountSize();
};

#endif //VEGA_STRIKE_ENGINE_CMD_MOUNT_SIZE_H
