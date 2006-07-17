#ifndef __UNIT_CONST_CACHE_H
#define __UNIT_CONST_CACHE_H

#include <string>
#include <gnuhash.h>
class ConstHasher;
class StringIntKey {
  friend class ConstHasher;
  std::string key;
  int fac;
public:
  StringIntKey (std::string k, int f) {
    key=k;
    fac=f;
  }
  bool operator == (const StringIntKey &b) const {
    return fac==b.fac&&key==b.key;
  }
  bool operator < (const StringIntKey &b) const {
    if (fac!=b.fac)
      return fac<b.fac;
    return key < b.key;
  }
};
class ConstHasher {
public:
  template <class T> size_t operator () (const T&key)const{
    return stdext::hash<T>()(key);
  }
  size_t operator () (const StringIntKey &key)const{
    return stdext::hash<std::string>()(key.key)^stdext::hash<size_t>()((size_t)key.fac);
  }
};
template <class Typ,class Key> class ClassCache {

  static stdext::hash_map<Key, Typ *, ConstHasher> unit_cache;
 public:
  static const Typ *getCachedConst (Key k) {
    return getCachedMutable(k);
  }
  static Typ *getCachedMutable (const Key &k) {
    typename stdext::hash_map<Key,Typ *,ConstHasher>::iterator i=unit_cache.find(k);
    if (i!=unit_cache.end())
      return (*i).second;
    return NULL;
  }
  static Typ *setCachedMutable (const Key &k, Typ *un) {
    unit_cache.insert (pair<Key,Typ *> (k,un));
    return un;
  }
  static const Typ *setCachedConst (const Key &k, Typ *un) {
    return setCachedMutable (k,un);
  }
  static void purgeCache(void (*Kill) (Typ * un)) {
    typename stdext::hash_map<Key,Typ *,ConstHasher>::iterator i=unit_cache.begin();
    for (;i!=unit_cache.end();++i) {
      (*Kill) ((*i).second);
    }
    unit_cache.clear();
  }
};

#if (defined(__GNUC__)&& ((__GNUC__ == 3 && __GNUC_MINOR__ >= 4)|| __GNUC__>3))
template <class Typ,class Key> stdext::hash_map<Key,Typ*,ConstHasher> ClassCache<Typ,Key>::unit_cache;
#endif

typedef ClassCache<Unit,StringIntKey> UnitConstCache;
typedef ClassCache<Mesh,std::string> WeaponMeshCache;

#endif

