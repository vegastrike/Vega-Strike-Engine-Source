#include <math.h>
#include <list>
#include <string>

using namespace std;

const int hashsize = 1001;

//Hashtable doesn't grow
template<class KEY, class VALUE> class Hashtable {
	class HashElement {
	public:
		KEY key;
		VALUE *value;
		HashElement(KEY k, VALUE *v) {key = k; value = v;}
	};
	list<HashElement> table[hashsize];

	int hash(const string &key) {
		int k = 0;
		char *start = (char*)key.c_str();
		char *end = start + strlen(start);

		for(;start!=end; start++) {
			k += (k << 3) + *start;
		}
		k %= hashsize;
		return abs(k);
	}
public:

	Hashtable()
	{
		
	}

	VALUE *Get(const KEY &key)
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

	void Put(const KEY &key, VALUE *value)
	{
		int hashval = hash(key);
		table[hashval].push_front(HashElement(key, value));
	}

	void Delete(const KEY &key)
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

/*
	VALUE *Get(const KEY &key);
	void Put(const KEY &key, VALUE *value);
	void Delete(const KEY &key);
*/
};
