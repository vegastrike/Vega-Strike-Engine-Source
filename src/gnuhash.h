#ifndef _GNUHASH_H_
#define _GNUHASH_H_
#ifdef _WIN32
#include <hash_map>
#else
#if __GNUC__ == 2
#include <map>
#define hash_map map
#define stdext std
namespace stdext {
    template<class Key, class Traits = std::less<Key> > class hash_compare
	{
	public:
		static const size_t bucket_size = 4;
		static const size_t min_buckets = 8;
	};
}

#include "hashtable.h"
#else
#include <ext/hash_map>
#define stdext __gnu_cxx
#include "hashtable.h"
class Unit;
namespace stdext{
  template<> class hash<std::string> {
  public:
    size_t operator () (const std::string&key) const{
      size_t _HASH_INTSIZE =(sizeof(size_t)*8);
      size_t _HASH_SALT_0 =0x7EF92C3B;
      size_t _HASH_SALT_1 =0x9B;
      size_t k = 0;
      for(std::string::const_iterator start = key.begin(); start!=key.end(); start++) {
        k ^= (*start&_HASH_SALT_1);
        k ^= _HASH_SALT_0;
        k  = (((k>>4)&0xF)|(k<<(_HASH_INTSIZE-4)));
        k ^= *start;
      }
      return k;   
    }
  };

  template<> class hash<void *> {
    hash<size_t> a;
  public:
    size_t operator () (const void *key) const{
      return a((size_t)key);
    }
  };
  template<> class hash<const void *> {
    hash<size_t> a;
  public:
    size_t operator () (const void * const &key) const{
      return a((size_t)key);
    }
  };

  template<> class hash<const Unit *> {
    hash<size_t> a;
  public:
    size_t operator () (const Unit * const &key) const{
      return a((size_t)key>>4);
    }
  };
  template<> class hash<std::pair<Unit *,Unit*> > {
    hash<size_t> a;
  public:
    size_t operator () (const std::pair<Unit*,Unit*> &key) const{
      return (size_t)(size_t)(a((int)(((size_t)key.first)>>4))^
                              a((int)(((size_t)key.second)>>4)));
    }
  };

	// Minimum declaration needed by SharedPool.h
    template<class Key, class Traits = std::less<Key> > class hash_compare
	{
	public:
		static const size_t bucket_size = 4;
		static const size_t min_buckets = 8;
	};

}
#endif
#endif
#endif
