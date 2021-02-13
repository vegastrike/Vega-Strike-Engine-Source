#include "unit_const_cache.h"

template<> UnitConstCache::cache_map UnitConstCache::unit_cache = UnitConstCache::cache_map();
template<> WeaponMeshCache::cache_map WeaponMeshCache::unit_cache = WeaponMeshCache::cache_map();

bool operator==(const StringIntKey& a, const StringIntKey& b)
{
    return a.fac == b.fac && a.key == b.key;
}

bool operator<(const StringIntKey& a, const StringIntKey& b)
{
    if (a.fac != b.fac)
        return a.fac < b.fac;
    return a.key < b.key;
}
