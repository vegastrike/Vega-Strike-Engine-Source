#ifndef __UNIT_CONST_CACHE_H
#define __UNIT_CONST_CACHE_H
#include "config.h"
#include "hashtable.h"
#include <string>
#include <gnuhash.h>

class Mesh;
class ConstHasher;

class StringIntKey
{
    friend class ConstHasher;
    std::string key;
    int fac;
public: StringIntKey( std::string k, int f )
    {
        key = k;
        fac = f;
    }
    bool operator==( const StringIntKey &b ) const
    {
        return fac == b.fac && key == b.key;
    }
    bool operator<( const StringIntKey &b ) const
    {
        if (fac != b.fac)
            return fac < b.fac;
        return key < b.key;
    }
    operator size_t() const {
        return Hashtable< std::string, int, (1<<30) >::hash( key )^fac;
    }
};

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

typedef ClassCache< Unit, StringIntKey >UnitConstCache;
typedef ClassCache< Mesh, std::string > WeaponMeshCache;

#endif

