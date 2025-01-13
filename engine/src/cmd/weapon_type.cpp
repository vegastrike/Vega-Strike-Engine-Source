/*
 * weapon_type.cpp
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


#include "weapon_type.h"

WEAPON_TYPE getWeaponTypeFromString(std::string type) {
    if (type == "Bolt") {
        return WEAPON_TYPE::BOLT;
    }

    if (type == "Ball") {
        return WEAPON_TYPE::BALL;
    }

    if (type == "Beam") {
        return WEAPON_TYPE::BEAM;
    }

    if (type == "Missile") {
        return WEAPON_TYPE::PROJECTILE;
    }

    return WEAPON_TYPE::UNKNOWN;
}

std::string getFilenameFromWeaponType(WEAPON_TYPE type) {
    switch (type) {
        case WEAPON_TYPE::BEAM:
            return "beamtexture.bmp";
        case WEAPON_TYPE::BALL:
            return "ball.ani";
        case WEAPON_TYPE::PROJECTILE:
            return "missile.bfxm";

            // Note these two types fall through to default.
        case WEAPON_TYPE::UNKNOWN:
        case WEAPON_TYPE::BOLT:
        default:
            return "";
    }
}
