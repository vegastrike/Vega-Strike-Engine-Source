/*
 * Vega Strike
 * Copyright (C) 2001-2002 Alan Shieh
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "mission/include/hashtable.h"
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

