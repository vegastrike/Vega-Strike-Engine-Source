#include "unit_const_cache.h"

template<> UnitConstCache::cache_map UnitConstCache::unit_cache = UnitConstCache::cache_map();
template<> WeaponMeshCache::cache_map WeaponMeshCache::unit_cache = WeaponMeshCache::cache_map();
