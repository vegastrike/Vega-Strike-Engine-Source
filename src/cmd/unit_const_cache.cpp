#include <map>
#include <string>
#include <vector>
#include "unit_generic.h"
class keyval {
  std::string key;
  int fac;
public:
  keyval (std::string k, int f) {
    key=k;
    fac=f;
  }
  bool operator < (const keyval &b) const {
    if (fac!=b.fac)
      return fac<b.fac;
    return key < b.key;
  }
};

typedef std::map <keyval, Unit *> UnitCacheType;

static UnitCacheType unit_cache;
const Unit * getCachedConstUnit (std::string name, int faction) {
  keyval kv (name,faction);
  UnitCacheType::const_iterator i = unit_cache.find (kv);
  if (i!=unit_cache.end())
    return (*i).second;
  return NULL;
}
const Unit * setCachedConstUnit (Unit * un) {
  unit_cache.insert (pair<keyval,Unit *> (keyval(un->name,un->faction),un));
  return un;
}
void purgeCache( ) {
    UnitCacheType::iterator i=unit_cache.begin();
    for (;i!=unit_cache.end();++i) {
      (*i).second->Kill();
    }
    unit_cache.clear();
}
