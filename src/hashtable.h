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
#include <map>

using namespace std;

template<typename KEY, int SIZ> struct ValueHashtableKey
{
    // partial specialization for int and string exists below
	static inline int hash(const KEY& key);
};

template<int SIZ> struct ValueHashtableKey<int,SIZ>
{
	static inline int hash(const int& key) {
	    return key%SIZ;
	}
};

template<int SIZ> struct ValueHashtableKey<long,SIZ>
{
	static inline long hash(const long& key) {
	    return key%SIZ;
	}
};

template<int SIZ> struct ValueHashtableKey<string,SIZ>
{
	static inline int hash(const std::string& key) {
		unsigned int k = 0;
		typename std::string::const_iterator start = key.begin();
		for(;start!=key.end(); start++) {
			k += (k * 128) + *start;
		}
		k %= SIZ;
		return k;
	}	
};

template <typename VALUE> struct ValueHashtableValue
{
    static inline VALUE invariant( ) {
        return VALUE();
    }
};

template <typename VALUE> struct ValueHashtableValue<VALUE*>
{
    static inline VALUE* invariant( ) {
        return NULL;
    }
};

template<typename KEY, class VALUE, int SIZ> class ValueHashtable
{
    typedef VALUE                                              HashElement;
    typedef std::map<KEY,HashElement>                          Slot;
    typedef typename std::map<KEY,HashElement>::iterator       SlotIt;
    typedef typename std::map<KEY,HashElement>::const_iterator SlotCit;
    typedef std::pair<KEY,HashElement>                         SlotPair;

    Slot table[SIZ];

	static inline int hash(const KEY& key) {
        return ValueHashtableKey<KEY,SIZ>::hash( key );
    }

    static inline VALUE invariant( ) {
        return ValueHashtableValue<VALUE>::invariant( );
    }

public:
	ValueHashtable()
	{
	}

	ValueHashtable( const ValueHashtable& orig )
	{
        for( int i=0; i<SIZ; i++ ) {
            table[i] = orig.table[i];
        }
	}

	std::vector<VALUE> GetAll() const
	{
	  std::vector <VALUE> retval;
	  for (unsigned int hashval=0;hashval<SIZ;hashval++) {
	    SlotCit iter = table[hashval].begin();
        SlotCit end  = table[hashval].end();
	    for(;iter!=end;iter++) {
	      retval.push_back(iter->second);
	    }
	  }
	  return retval;
	}

	VALUE Get(const KEY &key) const
	{
		int hashval = hash(key);
		SlotCit iter;
        iter = table[hashval].find(key);
        if( iter != table[hashval].end() ) {
            return iter->second;
        } else {
			return invariant();
        }
	}

	VALUE Get(const KEY &key, const KEY& backupkey ) const
	{
        int hashval = hash(key);
		SlotCit iter;
        iter = table[hashval].find(key);
        if( iter != table[hashval].end() ) {
            return iter->second;
        } else {
            return Get( backupkey );
        }
	}

	void Put(const KEY &key, VALUE value)
	{
		int hashval = hash(key);
		table[hashval].insert( SlotPair(key,value) );
	}

	void Delete(const KEY &key)
	{
		int hashval = hash(key);
		SlotIt iter;
        iter = table[hashval].find(key);
        if( iter != table[hashval].end() ) {
			table[hashval].erase(iter);
        }
	}

private:
    ValueHashtable& operator=( const ValueHashtable& );
};

template<class KEY, class VALUE, int SIZ> class Hashtable
    : public ValueHashtable<KEY,VALUE*,SIZ>
{
public:
    Hashtable( )
        : ValueHashtable<KEY,VALUE*,SIZ>( )
    { }

    Hashtable( const Hashtable& orig )
        : ValueHashtable<KEY,VALUE*,SIZ>( orig )
    { }

private:
    Hashtable& operator=( const Hashtable& );
};

#endif

