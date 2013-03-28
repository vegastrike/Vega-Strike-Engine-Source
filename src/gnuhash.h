#ifndef _GNUHASH_H_
#define _GNUHASH_H_
#include "config.h"
//The following block is to only use tr1 from at least 4.3 since 4.2 apparently bugs out.
//Windows is untested at the moment.
#ifdef HAVE_TR1_UNORDERED_MAP
#ifndef WIN32
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 3)
#undef HAVE_TR1_UNORDERED_MAP
#endif
#endif
#endif


#ifdef _WIN32
#ifdef HAVE_TR1_UNORDERED_MAP
#if defined(_MSC_VER) && _MSC_VER >= 1600
#include <unordered_map>  //MSVC doesn't use tr1 dirs
#else
#include <tr1/unordered_map>
#endif
#define vsUMap std::tr1::unordered_map
#define vsHashComp std::tr1::hash_compare
#define vsHash std::tr1::hash
#else
#include <hash_map>
#define vsUMap stdext::hash_map
#define vsHashComp stdext::hash_compare
#define vsHash stdext::hash
#endif

#ifdef HAVE_NO_HASH_COMPARE
#undef vsHashComp
#define vsHashComp vsHash
#endif


#else //#ifdef _WIN32 { ... } else ...
#if __GNUC__ == 2
#include <map>
#define hash_map map
#define stdext std
namespace stdext
{
template < class Key, class Traits = std::less< Key > >
class hash_compare
{
public:
    static const size_t bucket_size = 4;
    static const size_t min_buckets = 8;
};
}
#ifdef HAVE_TR1_UNORDERED_MAP
#define vsUMap std::tr1::unordered_map
#define vsHashComp std::tr1::hash_compare
#define vsHash std::tr1::hash
#else
#define vsUMap stdext::hash_map
#define vsHashComp stdext::hash_compare
#define vsHash stdext::hash
#endif

#ifdef HAVE_NO_HASH_COMPARE
#undef vsHashComp
#define vsHashComp vsHash
#endif

#include "hashtable.h"
#else //if __GNUC__ == 2 { ... } else ...
#ifdef HAVE_TR1_UNORDERED_MAP
#include <tr1/unordered_map>

#define vsUMap std::tr1::unordered_map
#define vsHashComp std::tr1::hash_compare
#define vsHash std::tr1::hash

#ifdef HAVE_NO_HASH_COMPARE
#undef vsHashComp
#define vsHashComp vsHash
#endif

#include "hashtable.h"
class Unit;
namespace std
{
namespace tr1
{
#else //if HAVE_TR1_UNORDERED_MAP { ... } else ...
#include <ext/hash_map>
#define stdext __gnu_cxx
#define vsUMap stdext::hash_map
#define vsHashComp stdext::hash_compare
#define vsHash stdext::hash

#ifdef HAVE_NO_HASH_COMPARE
#undef vsHashComp
#define vsHashComp vsHash
#endif

#include "hashtable.h"

class Unit;

namespace stdext
{
template < >
class hash< std::string >
{
public:
    size_t operator()( const std::string &key ) const
    {
        size_t _HASH_INTSIZE = (sizeof (size_t)*8);
        size_t _HASH_SALT_0  = 0x7EF92C3B;
        size_t _HASH_SALT_1  = 0x9B;
        size_t k = 0;
        
        for (unsigned int i = 0; i < key.size(); ++i) {
            k ^= (key[i]&_HASH_SALT_1);
            k ^= _HASH_SALT_0;
            k  = ( ( (k>>4)&0xF )|( k<<(_HASH_INTSIZE-4) ) );
            k ^= key[i];
        }
        return k;
    }
};
#endif //if HAVE_TR1_UNORDERED_MAP { ... } else ...

template < >
class hash< void* >
{
    hash< size_t >a;
public:
    size_t operator()( const void *key ) const
    {
        return a( (size_t) key );
    }
};
template < >
class hash< const void* >
{
    hash< size_t >a;
public:
    size_t operator()( const void*const &key ) const
    {
        return a( (size_t) key );
    }
};
template < >
class hash< const Unit* >
{
    hash< size_t >a;
public:
    size_t operator()( const Unit*const &key ) const
    {
        return a( (size_t) key>>4 );
    }
};
template < >
class hash< std::pair< Unit*, Unit* > >
{
    hash< size_t >a;
public:
    size_t operator()( const std::pair< Unit*, Unit* > &key ) const
    {
        return (size_t) (size_t) ( a( (int) ( ( (size_t) key.first )>>4 ) )
                                  ^a( (int) ( ( (size_t) key.second )>>4 ) ) );
    }
};
//Minimum declaration needed by SharedPool.h
template < class Key, class Traits = std::less< Key > >
class hash_compare
{
public:
    static const size_t bucket_size = 4;
    static const size_t min_buckets = 8;
};
}
#ifdef HAVE_TR1_UNORDERED_MAP
}
#endif

#endif //if __GNUC__ == 2 { ... } else ...
#endif //#ifdef _WIN32 { ... } else ...
#endif //def _GNUHASH_H_

