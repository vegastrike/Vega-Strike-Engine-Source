/* 
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
 * You should have recvbufd a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef CLIENT_PTR_H
#define CLIENT_PTR_H

#include <map>
#include <list>
#include <vector>
class Client;

#ifndef USE_VALGRINDABLE_CLIENTPTR

#include "boost/smart_ptr.hpp"
#include "boost/weak_ptr.hpp"

typedef boost::shared_ptr<Client>          ClientPtr;
//typedef boost::weak_ptr<Client>            ClientWeakPtr;
#else

template <class T> class NormalPtr {
	T *val;
public:
	NormalPtr(T*val) :val(val) {}
	NormalPtr(const NormalPtr<T> &p) : val(p.val) {}
	NormalPtr() :val(NULL) {}
	~NormalPtr() {}
	T& operator*() {return *val;}
	const T& operator*() const {return *val;}
	T* operator->() {return val;}
	const T* operator->() const {return val;}
	T* get() { return val; }
	const T* get() const { return val; }
	void reset() { val=NULL; }
	bool expired() { return false; }
	operator bool() const { return val?true:false; }
	bool operator !() const { return val?false:true; }
};

typedef NormalPtr<Client> ClientPtr;
typedef NormalPtr<Client> ClientWeakPtr;
#endif

typedef std::map<int,ClientPtr>            ClientMap;
typedef std::pair<int,ClientPtr>           ClientPair;
typedef std::map<int,ClientPtr>::iterator  ClientIt;

typedef std::list<ClientPtr>               ClientList;
// typedef std::list<ClientWeakPtr>           ClientWeakList;
typedef ClientList::iterator               ClientListIt, LI;
//typedef ClientWeakList::iterator           CWLI;
typedef std::vector<ClientList*>::iterator VCLI; //VLI;


#endif /* CLIENT_PTR_H */
