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

#include "gnuhash.h"
#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#define HASH_INTSIZE (sizeof(int)*8)
#define HASH_SALT_0 0x7EF92C3B
#define HASH_SALT_1 0x9B
class Unit;
//const int hashsize = 1001;

//Hashtable doesn't grow
template<class KEY, class VALUE, int SIZ> class Hashtable :public stdext::hash_map<KEY,VALUE*> {

  typedef std::pair<KEY,VALUE*> HashElement;
  typedef ::stdext::hash_map<KEY,VALUE* > supertype;
  
public:
	static int hash(const int key) {
		unsigned int k = key;
		k%=SIZ;
		return k;
	}
        static int hash(const char *key) {
          unsigned int k = 0;
          for(const char * start = key; *start!='\0'; ++start) {
            k ^= (*start&HASH_SALT_1);
            k ^= HASH_SALT_0;
            k  = (((k>>4)&0xF)|(k<<(HASH_INTSIZE-4)));
            k ^= *start;
          }
          k %= SIZ;
          return k;
	}
	static int hash(const std::string &key) {
          unsigned int k = 0;
          for(typename std::string::const_iterator start = key.begin(); start!=key.end(); ++start) {
            k ^= (*start&HASH_SALT_1);
            k ^= HASH_SALT_0;
            k  = (((k>>4)&0xF)|(k<<(HASH_INTSIZE-4)));
            k ^= *start;
          }
          k %= SIZ;
          return k;
	}
	std::vector <VALUE *> GetAll() const
	{
	  std::vector <VALUE *> retval(this->size());
          typename supertype::const_iterator iter=this->begin();
          typename supertype::const_iterator end=this->end();
          size_t i=0;
          for (;iter!=end;++iter,++i) {
            retval[i]=iter->second;
          }
	  return retval;
	}
  
	VALUE *Get(const KEY &key) const
	{
          typename supertype::const_iterator iter=this->find(key);
          typename supertype::const_iterator end=this->end();
          if (iter!=end) return iter->second;
          return NULL;
	}

	void Put(const KEY &key, VALUE *value)
	{
          (*this)[key]=value;
	}

	void Delete(const KEY &key)
	{
			 typename supertype::iterator iter=this->find(key);
		  if(iter == this->end()){
//		  	fprintf(stderr,"failed to remove item in hash_map\n");
		  	return;
			}
          this->erase(iter);
	}
};

#endif
