#include "hashtable.h"
/*
template<class KEY, class VALUE> VALUE *Hashtable<KEY, VALUE>::Get(const KEY &key)
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

template<class KEY, class VALUE> void Hashtable<KEY, VALUE>::Put(const KEY &key, VALUE *value)
{
	int hashval = hash(key);
	table[hashval].push_front(HashElement(key, value));
}

template<class KEY, class VALUE> void Hashtable<KEY, VALUE>::Delete(const KEY &key)
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
