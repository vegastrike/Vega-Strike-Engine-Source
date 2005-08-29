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
#include <algorithm>

#define HASH_INTSIZE (sizeof(int)*8)
#define HASH_SALT_0 0x7EF92C3B
#define HASH_SALT_1 0x9B

//const int hashsize = 1001;
using namespace std;
//Hashtable doesn't grow
template<class KEY, class VALUE, int SIZ> class Hashtable {

    typedef std::pair<KEY,VALUE*>                             HashElement;
    typedef typename std::vector<HashElement>::iterator       It;
    typedef typename std::vector<HashElement>::const_iterator CIt;

    struct HashElementEq {
        const KEY& key;
        HashElementEq( const KEY& k ) : key(k) { }
        bool operator()( const HashElement& elem ) const {
            return ( key == elem.first );
        }
    };

	std::vector<HashElement> table[SIZ];

	static int hash(const int key) {
		unsigned int k = key;
		k%=SIZ;
		return k;
	}
	static int hash(const std::string &key) {
		unsigned int k = 0;
		for(typename std::string::const_iterator start = key.begin(); start!=key.end(); start++) {
            k ^= (*start&HASH_SALT_1);
            k ^= HASH_SALT_0;
            k  = (((k>>4)&0xF)|(k<<(HASH_INTSIZE-4)));
            k ^= *start;
		}
		k %= SIZ;
		return k;
	}	
public:

	Hashtable()
	{
	}
	Hashtable( const Hashtable& orig )
	{
        for( int i=0; i<SIZ; i++ ) {
            table[i] = orig.table[i];
        }
	}
	std::vector <VALUE *> GetAll() const
	{
	  std::vector <VALUE *> retval;
	  for (unsigned int hashval=0;hashval<SIZ;hashval++) {
	    CIt iter = table[hashval].begin(), end = table[hashval].end();
	    for(;iter!=end;iter++) {
	      retval.push_back (iter->second);
	    }
	  }
	  return retval;
	}

	VALUE *Get(const KEY &key) const
	{
        HashElementEq eq(key);
		int hashval = hash(key);
	    CIt iter = table[hashval].begin(), end = table[hashval].end();
	    for(;iter!=end;iter++) {
			if( eq(*iter) ) break;
		}
		if(iter==end)
			return NULL;
		else
			return iter->second;
	}

	void Put(const KEY &key, VALUE *value)
	{
        int hashval = hash(key);
		table[hashval].push_back(HashElement(key, value));
	}

	void Delete(const KEY &key)
	{
        HashElementEq eq(key);
		int hashval = hash(key);
        It iter;
        It end = table[hashval].end();
        iter = find_if( table[hashval].begin(), end, eq );
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

private:
	Hashtable& operator=( const Hashtable& orig );
};

#endif
