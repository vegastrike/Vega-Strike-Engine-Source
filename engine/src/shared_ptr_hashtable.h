/*
 * shared_ptr_hashtable.h
 *
 * Copyright (C) 2001-2022 Daniel Horn, Stephen G. Tuggy,
 * and other Vega Strike contributors
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


#ifndef VEGA_STRIKE_SHARED_PTR_HASHTABLE_H
#define VEGA_STRIKE_SHARED_PTR_HASHTABLE_H

#include "preferred_types.h"
#include <deque>
#include <memory>
#include <utility>
#include <algorithm>
#include <vector>
#include <string>
#include <math.h>
#include "gnuhash.h"

#define HASH_INTSIZE (sizeof (int)*8)
#define HASH_SALT_0 0x7EF92C3B
#define HASH_SALT_1 0x9B

template<class KEY, class VALUE, int SIZ>
class SharedPtrHashtable : public vsUMap<KEY, vega_types::SharedPtr<VALUE>> {
    typedef std::pair<KEY, vega_types::SharedPtr<VALUE>> HashElement;
    typedef vsUMap<KEY, vega_types::SharedPtr<VALUE>> supertype;
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
        for (char ch : key) {
            k ^= (ch & HASH_SALT_1);
            k ^= HASH_SALT_0;
            k = (((k >> 4) & 0xF) | (k << (HASH_INTSIZE - 4)));
            k ^= ch;
        }
        k %= SIZ;
        return k;
    }

    vega_types::SequenceContainer<vega_types::SharedPtr<VALUE>> GetAll() const {
        vega_types::SequenceContainer<vega_types::SharedPtr<VALUE>> retval(this->size());
        typename supertype::const_iterator iter = this->begin();
        typename supertype::const_iterator end = this->end();
        size_t i = 0;
        for (; iter != end; ++iter, ++i) {
            retval[i] = iter->second;
        }
        return retval;
    }

    // TODO: Optimize?
    vega_types::SharedPtr<VALUE> Get(const KEY &key) const {
        typename supertype::const_iterator iter = this->find(key);
        typename supertype::const_iterator end = this->end();
        if (iter != end) {
            return iter->second;
        }
        return nullptr;
    }

    void Put(const KEY &key, vega_types::SharedPtr<VALUE> value) {
        (*this)[key] = value;
    }

    // TODO: Optimize?
    void Delete(const KEY &key) {
        typename supertype::iterator iter = this->find(key);
        if (iter == this->end()) {
            return;
        }
        this->erase(iter);
    }
};

#endif //VEGA_STRIKE_SHARED_PTR_HASHTABLE_H
