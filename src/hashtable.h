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

#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <math.h>
#include <string>
#include <vector>

//const int hashsize = 1001;
using namespace std;
//Hashtable doesn't grow
template<class KEY, class VALUE, class SIZ> class Hashtable {
	class HashElement {
	public:
		KEY key;
		VALUE *value;
		HashElement(KEY k, VALUE *v) {key = k; value = v;}
	};
	std::vector<HashElement> table[sizeof (SIZ)];
	static int hash(const int key) {
	  return key%sizeof(SIZ);
	}
	static int hash(const std::string &key) {
		unsigned int k = 0;
		typename std::string::const_iterator start = key.begin();
		for(;start!=key.end(); start++) {
			k += (k * 128) + *start;
		}
		k %= sizeof(SIZ);
		return k;
	}	
public:

	Hashtable()
	{
	}
	std::vector <VALUE *> GetAll() const
	{
	  std::vector <VALUE *> retval;
	  for (unsigned int hashval=0;hashval<sizeof(SIZ);hashval++) {
	    typename std::vector<HashElement>::const_iterator iter = table[hashval].begin(), end = table[hashval].end();
	    for(;iter!=end;iter++) {
	      retval.push_back ((*iter).value);
	    }
	  }
	  return retval;
	}
	VALUE *Get(const KEY &key) const
	{
		int hashval = hash(key);
		typename std::vector<HashElement>::const_iterator iter = table[hashval].begin(), end = table[hashval].end();

		for(;iter!=end;iter++)
			if((*iter).key == key)
				break;
		if(iter==end)
			return NULL;
		else
			return (*iter).value;
	}

	void Put(const KEY &key, VALUE *value)
	{
	        int hashval = hash(key);
		table[hashval].push_back(HashElement(key, value));
	}

	void Delete(const KEY &key)
	{
		int hashval = hash(key);
		typename std::vector<HashElement>::iterator iter = table[hashval].begin(), end = table[hashval].end();

		for(;iter!=end;iter++)
			if((*iter).key == key)
				break;
		if(iter==end)
			return;
		else {
			table[hashval].erase(iter);
		}
	}

/*
	VALUE *Get(const KEY &key);
	void Put(const KEY &key, VALUE *value);
	void Delete(const KEY &key);
*/
};

#endif
