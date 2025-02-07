/*
 * hashtable.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file; Alan Shieh specifically
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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
#ifndef VEGA_STRIKE_ENGINE_HASHTABLE_H
#define VEGA_STRIKE_ENGINE_HASHTABLE_H

#include "gnuhash.h"
#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#define HASH_INTSIZE (sizeof (int)*8)
#define HASH_SALT_0 0x7EF92C3B
#define HASH_SALT_1 0x9B

class Unit;
//const int hashsize = 1001;
//Hashtable doesn't grow

template<class KEY, class VALUE, int SIZ>
class Hashtable : public vsUMap<KEY, VALUE *> {
    typedef std::pair<KEY, VALUE *> HashElement;
    typedef vsUMap<KEY, VALUE *> supertype;
public:
    static size_t hash(const int key) {
        unsigned int k = key;
        k %= SIZ;
        return k;
    }

    static size_t hash(const char *key) {
        unsigned int k = 0;
        for (const char *start = key; *start != '\0'; ++start) {
            k ^= (*start & HASH_SALT_1);
            k ^= HASH_SALT_0;
            k = (((k >> 4) & 0xF) | (k << (HASH_INTSIZE - 4)));
            k ^= *start;
        }
        k %= SIZ;
        return k;
    }

    static size_t hash(const std::string &key) {
        unsigned int k = 0;
        for (typename std::string::const_iterator start = key.begin(); start != key.end(); ++start) {
            k ^= (*start & HASH_SALT_1);
            k ^= HASH_SALT_0;
            k = (((k >> 4) & 0xF) | (k << (HASH_INTSIZE - 4)));
            k ^= *start;
        }
        k %= SIZ;
        return k;
    }

    std::vector<VALUE *> GetAll() const {
        std::vector<VALUE *> retval(this->size());
        typename supertype::const_iterator iter = this->begin();
        typename supertype::const_iterator end = this->end();
        size_t i = 0;
        for (; iter != end; ++iter, ++i) {
            retval[i] = iter->second;
        }
        return retval;
    }

    VALUE *Get(const KEY &key) const {
        typename supertype::const_iterator iter = this->find(key);
        typename supertype::const_iterator end = this->end();
        if (iter != end) {
            return iter->second;
        }
        return NULL;
    }

    void Put(const KEY &key, VALUE *value) {
        (*this)[key] = value;
    }

    void Delete(const KEY &key) {
        typename supertype::iterator iter = this->find(key);
        if (iter == this->end()) {
            return;
        }
        this->erase(iter);
    }
};

#endif //VEGA_STRIKE_ENGINE_HASHTABLE_H
