/*
 * hashtable.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file. Specifically: Alan Shieh
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
#ifndef VEGA_STRIKE_ENGINE_MISSION_HASHTABLE_H
#define VEGA_STRIKE_ENGINE_MISSION_HASHTABLE_H

#include <math.h>
#include <list>
#include <string>

using namespace std;

//const int hashsize = 1001;

//Hashtable doesn't grow
template<class KEY, class VALUE, class SIZ>
class Hashtable {
    class HashElement {
    public:
        KEY key;
        VALUE *value;

        HashElement(KEY k, VALUE *v) {
            key = k;
            value = v;
        }
    };
    list<HashElement> table[sizeof(SIZ)];

    static int hash(const int key) {
        return key % sizeof(SIZ);
    }

    static int hash(const string &key) {
        int k = 0;
        char *start = (char *) key.c_str();
        char *end = start + strlen(start);

        for (; start != end; start++) {
            k += (k << 3) + *start;
        }
        k %= sizeof(SIZ);
        return abs(k);
    }

public:

    Hashtable() {
    }

    VALUE *Get(const KEY &key) const {
        int hashval = hash(key);
        list<HashElement>::const_iterator iter = table[hashval].begin(), end = table[hashval].end();

        for (; iter != end; iter++) {
            if ((*iter).key == key) {
                break;
            }
        }
        if (iter == end) {
            return NULL;
        } else {
            return (*iter).value;
        }
    }

    void Put(const KEY &key, VALUE *value) {
        int hashval = hash(key);
        table[hashval].push_front(HashElement(key, value));
    }

    void Delete(const KEY &key) {
        int hashval = hash(key);
        list<HashElement>::iterator iter = table[hashval].begin(), end = table[hashval].end();

        for (; iter != end; iter++) {
            if ((*iter).key == key) {
                break;
            }
        }
        if (iter == end) {
            return;
        } else {
            table[hashval].erase(iter);
        }
    }

/*
	VALUE *Get(const KEY &key);
	void Put(const KEY &key, VALUE *value);
	void Delete(const KEY &key);
*/
};

#endif //VEGA_STRIKE_ENGINE_MISSION_HASHTABLE_H
