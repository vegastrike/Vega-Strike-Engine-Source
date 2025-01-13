/*
 * weapon_info.h
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
#ifndef VEGA_STRIKE_ENGINE_CMD_WEAPON_INFO_H
#define VEGA_STRIKE_ENGINE_CMD_WEAPON_INFO_H

#include "weapon_type.h"
#include "gfx/vec.h"
#include "mount_size.h"

#include <string>

struct WeaponInfo {
    // Fields
    std::string name;
    WEAPON_TYPE type;
    MOUNT_SIZE size = MOUNT_SIZE::NOWEAP;

    // Make const again
    /*const*/ float damage = 1.8;
    /*const*/ float energy_rate = 18;
    /*const*/ float length = 5;
    /*const*/ float lock_time = 0;
    /*const*/ float long_range = .5;
    /*const*/ Vector offset = Vector();
    /*const*/ float phase_damage = 0;
    /*const*/ float pulse_speed = 15;
    /*const*/ float radial_speed = 1;
    /*const*/ float radius = 0.5;
    /*const*/ float range = 100;
    /*const*/ float refire_rate = .2;
    /*const*/ float stability = 60;
    /*const*/ int sound = -1;
    /*const*/ float speed = 10;
    /*const*/ float texture_stretch = 1;

    /*const*/ float volume = 0;

    /*const*/ float r = 127;
    /*const*/ float g = 127;
    /*const*/ float b = 127;
    /*const*/ float a = 127;

    mutable class Mesh *gun = nullptr;      //requires nonconst to add to orig drawing queue when drawing
    mutable class Mesh *gun1 = nullptr;     //requires nonconst to add to orig drawing queue when drawing

    // Constructors
    WeaponInfo();
    WeaponInfo(WEAPON_TYPE type,
            std::string name,
            MOUNT_SIZE mount_size,
            float damage,
            float energy_rate,
            float length,
            float lock_time,
            float long_range,
            Vector offset,
            float phase_damage,
            float pulse_speed,
            float radial_speed,
            float radius,
            float refire_rate,
            int role_bits,
            float stability,
            int sound,
            float speed,
            float texture_stretch,
            float volume,
            float appearance,

            float r,
            float g,
            float b,
            float a);

    WeaponInfo(WEAPON_TYPE type);
    WeaponInfo(const WeaponInfo &tmp);


    // Methods

    friend void beginElement(void *userData, const char *name, const char **atts);

    float Refire() const;
    bool isMissile() const;
    std::string file;

    void netswap();
private:

};

WeaponInfo *getWeapon(const std::string &key);

#endif //VEGA_STRIKE_ENGINE_CMD_WEAPON_INFO_H
