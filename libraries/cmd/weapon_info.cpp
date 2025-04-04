/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#include "cmd/weapon_info.h"

#include "root_generic/vs_globals.h"
#include "root_generic/options.h"
#include "cmd/unit_const_cache.h"

// TODO: this should not be here
#include "root_generic/vsfilesystem.h"
#include "gfx_generic/mesh.h"

#include <boost/algorithm/string.hpp>

namespace alg = boost::algorithm;

Hashtable<std::string, WeaponInfo, 257> lookuptable;

WeaponInfo::WeaponInfo() {
}

WeaponInfo::WeaponInfo(WEAPON_TYPE type) {
    this->type = type;
}

WeaponInfo::WeaponInfo(const WeaponInfo &tmp) {
    *this = tmp;
}

float WeaponInfo::Refire() const {
    unsigned int len = name.length();
    // TODO: what is this???
    if (g_game.difficulty > .98 || len < 9 || name[len - 8] != 'C' || name[len - 9] != '_' || name[len - 7]
            != 'o' || name[len - 6] != 'm' || name[len - 5] != 'p' || name[len - 4] != 'u' || name[len - 3]
            != 't' || name[len - 2] != 'e' || name[len - 1] != 'r') {
        return refire_rate;
    }
    return this->refire_rate * (game_options()->refire_difficulty_scaling
            / (1.0f + (game_options()->refire_difficulty_scaling - 1.0f) * g_game.difficulty));
}

bool WeaponInfo::isMissile() const {
    if (vega_config::config->graphics.hud.projectile_means_missile && this->type == WEAPON_TYPE::PROJECTILE) {
        return true;
    }
    if (!vega_config::config->graphics.hud.projectile_means_missile && this->size >= MOUNT_SIZE::LIGHTMISSILE) {
        return true;
    }
    return false;
}

// TODO: this should not be here
using namespace VSFileSystem;

WeaponInfo *getWeapon(const std::string &key) {
    WeaponInfo *wi = lookuptable.Get(boost::to_upper_copy(key));
    if (wi) {
        if (!WeaponMeshCache::getCachedMutable(wi->name)) {
            static FileLookupCache lookup_cache;
            string meshname = boost::to_lower_copy(key) + ".bfxm";
            if (CachedFileLookup(lookup_cache, meshname, MeshFile) <= Ok) {
                WeaponMeshCache::setCachedMutable(wi->name, wi->gun =
                        Mesh::LoadMesh(meshname.c_str(), Vector(1, 1, 1), 0, NULL));
                WeaponMeshCache::setCachedMutable(wi->name + "_flare", wi->gun1 =
                        Mesh::LoadMesh(meshname.c_str(), Vector(1, 1, 1), 0, NULL));
            }
        }
    }
    return wi;
}

void WeaponInfo::netswap() {
    //Enum elements are the size of an int
    //byte order swap doesn't work with ENUM - MAY NEED TO FIND A WORKAROUND SOMEDAY
    //type = VSSwapHostIntToLittle( type);
    //size = VSSwapHostIntToLittle( size);

    damage = VSSwapHostFloatToLittle(damage);
    energy_rate = VSSwapHostFloatToLittle(energy_rate);
    length = VSSwapHostFloatToLittle(length);
    lock_time = VSSwapHostFloatToLittle(lock_time);
    long_range = VSSwapHostFloatToLittle(long_range);
    offset.netswap();
    phase_damage = VSSwapHostFloatToLittle(phase_damage);
    pulse_speed = VSSwapHostFloatToLittle(pulse_speed);
    radial_speed = VSSwapHostFloatToLittle(radial_speed);
    radius = VSSwapHostFloatToLittle(radius);
    range = VSSwapHostFloatToLittle(range);
    refire_rate = VSSwapHostFloatToLittle(refire_rate);
    //role_bits         = VSSwapHostIntToLittle( role_bits );
    stability = VSSwapHostFloatToLittle(stability);
    sound = VSSwapHostIntToLittle(sound);
    speed = VSSwapHostFloatToLittle(speed);
    texture_stretch = VSSwapHostFloatToLittle(texture_stretch);
    volume = VSSwapHostFloatToLittle(volume);

    r = VSSwapHostFloatToLittle(r);
    g = VSSwapHostFloatToLittle(g);
    b = VSSwapHostFloatToLittle(b);
    a = VSSwapHostFloatToLittle(a);
}
