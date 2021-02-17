#ifndef __UNIT_CONST_CACHE_H
#define __UNIT_CONST_CACHE_H
#include "hashtable.h"
#include <string>
#include <gnuhash.h>
#include <functional>

class Mesh;

class StringIntKey
{
public:
    std::string key;
    int fac;
public: StringIntKey( std::string k, int f )
    {
        key = k;
        fac = f;
    }
};

extern bool operator==(const StringIntKey& a, const StringIntKey& b);

extern bool operator<(const StringIntKey& a, const StringIntKey& b);

namespace std
{
    template<> struct hash<StringIntKey>
    {
        std::size_t operator()(StringIntKey const& s) const noexcept
        {
            return (std::hash<std::string>{}(s.key)) ^ s.fac;
        }
    };
}



#if HAVE_TR1_UNORDERED_MAP || (!defined (_WIN32) && __GNUC__ != 2)
class ConstHasher
{
public:
    template < class T >
    size_t operator()( const T &key ) const
    {
        return vsHash< T > () ( key );
    }
    size_t operator()( const StringIntKey &key ) const
    {
        return vsHash< std::string > () ( key.key )^vsHash< size_t > () ( (size_t) key.fac );
    }
};
#endif

template < class Typ, class Key >
class ClassCache
{
#if HAVE_TR1_UNORDERED_MAP || (!defined (_WIN32) && __GNUC__ != 2)
    typedef vsUMap< Key, Typ*, ConstHasher > cache_map;
#else
    typedef vsUMap< Key, Typ* > cache_map;
#endif
	static cache_map unit_cache;
public:
    static const Typ * getCachedConst( Key k )
    {
        return getCachedMutable( k );
    }
    static Typ * getCachedMutable( const Key &k )
    {
        typename cache_map::iterator i = unit_cache.find( k );
        if ( i != unit_cache.end() )
            return (*i).second;
        return NULL;
    }
    static Typ * setCachedMutable( const Key &k, Typ *un )
    {
        unit_cache.insert( std::pair< Key, Typ* > ( k, un ) );
        return un;
    }
    static const Typ * setCachedConst( const Key &k, Typ *un )
    {
        return setCachedMutable( k, un );
    }
    static void purgeCache( void (*Kill)( Typ *un ) )
    {
        typename cache_map::iterator i = unit_cache.begin();
        for (; i != unit_cache.end(); ++i)
            (*Kill)( (*i).second );
        unit_cache.clear();
    }
};

typedef ClassCache< Unit, StringIntKey > UnitConstCache;
typedef ClassCache< Mesh, std::string > WeaponMeshCache;

#endif

