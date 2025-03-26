/**
 * unit_const_cache.cpp
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


#include "cmd/unit_const_cache.h"

template<> UnitConstCache::cache_map UnitConstCache::unit_cache = UnitConstCache::cache_map();
template<> WeaponMeshCache::cache_map WeaponMeshCache::unit_cache = WeaponMeshCache::cache_map();

bool operator==(const StringIntKey &a, const StringIntKey &b) {
    return a.fac == b.fac && a.key == b.key;
}

bool operator<(const StringIntKey &a, const StringIntKey &b) {
    if (a.fac != b.fac) {
        return a.fac < b.fac;
    }
    return a.key < b.key;
}
