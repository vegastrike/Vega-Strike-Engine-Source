/**
 * unit_const_cache.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
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


#ifndef VEGA_STRIKE_ENGINE_CMD_UNIT_CONST_CACHE_H
#define VEGA_STRIKE_ENGINE_CMD_UNIT_CONST_CACHE_H
#include "src/hashtable.h"
#include <string>
#include <gnuhash.h>
#include <functional>

class Mesh;

class StringIntKey {
public:
    std::string key;
    int fac;
public:
    StringIntKey(std::string k, int f) {
        key = k;
        fac = f;
    }
};

extern bool operator==(const StringIntKey &a, const StringIntKey &b);

extern bool operator<(const StringIntKey &a, const StringIntKey &b);

namespace std {
template<>
struct hash<StringIntKey> {
    std::size_t operator()(StringIntKey const &s) const noexcept {
        return (std::hash<std::string>{}(s.key)) ^ s.fac;
    }
};
}

#if HAVE_TR1_UNORDERED_MAP || (!defined (_WIN32) && __GNUC__ != 2)
class ConstHasher {
public:
    template<class T>
    size_t operator()(const T &key) const {
        return vsHash<T>()(key);
    }

    size_t operator()(const StringIntKey &key) const {
        return vsHash<std::string>()(key.key) ^ vsHash<size_t>()((size_t) key.fac);
    }
};
#endif

template<class Typ, class Key>
class ClassCache {
#if HAVE_TR1_UNORDERED_MAP || (!defined (_WIN32) && __GNUC__ != 2)
    typedef vsUMap<Key, Typ *, ConstHasher> cache_map;
#else
    typedef vsUMap< Key, Typ* > cache_map;
#endif
    static cache_map unit_cache;
public:
    static const Typ *getCachedConst(Key k) {
        return getCachedMutable(k);
    }

    static Typ *getCachedMutable(const Key &k) {
        typename cache_map::iterator i = unit_cache.find(k);
        if (i != unit_cache.end()) {
            return (*i).second;
        }
        return NULL;
    }

    static Typ *setCachedMutable(const Key &k, Typ *un) {
        unit_cache.insert(std::pair<Key, Typ *>(k, un));
        return un;
    }

    static const Typ *setCachedConst(const Key &k, Typ *un) {
        return setCachedMutable(k, un);
    }

    static void purgeCache(void (*Kill)(Typ *un)) {
        typename cache_map::iterator i = unit_cache.begin();
        for (; i != unit_cache.end(); ++i) {
            (*Kill)((*i).second);
        }
        unit_cache.clear();
    }
};

typedef ClassCache<Unit, StringIntKey> UnitConstCache;
typedef ClassCache<Mesh, std::string> WeaponMeshCache;

#endif //VEGA_STRIKE_ENGINE_CMD_UNIT_CONST_CACHE_H
