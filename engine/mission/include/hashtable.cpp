/*
 * hashtable.cpp
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
#include "hashtable.h"
/*
template<class KEY, class VALUE, class SIZ> VALUE *Hashtable<KEY, VALUE, SIZ>::Get(const KEY &key)
{
	int hashval = hash(key);
	list<HashElement>::iterator iter = table[hashval].begin(), end = table[hashval].end();

	for(;iter!=end;iter++)
		if((*iter).key == key)
			break;
	if(iter==end)
		return NULL;
	else
		return (*iter).value;
}

template<class KEY, class VALUE, class SIZ> void Hashtable<KEY, VALUE, SIZ>::Put(const KEY &key, VALUE *value)
{
	int hashval = hash(key);
	table[hashval].push_front(HashElement(key, value));
}

template<class KEY, class VALUE, class SIZ> void Hashtable<KEY, VALUE, SIZ>::Delete(const KEY &key)
{
	int hashval = hash(key);
	list<HashElement>::iterator iter = table[hashval].begin(), end = table[hashval].end();

	for(;iter!=end;iter++)
		if((*iter).key == key)
			break;
	if(iter==end)
		return NULL;
	else {
		table[hashval].erase(iter);
	}
}
*/

