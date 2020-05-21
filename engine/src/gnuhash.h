#ifndef _GNUHASH_H_
#define _GNUHASH_H_
#include "config.h"
#include <unordered_map>
#include <functional>

#define vsUMap     std::unordered_map
#define vsHashComp std::hash_compare
#define vsHash     std::hash

namespace std
{
//Minimum declaration needed by SharedPool.h
    template < class Key, class Traits = std::less< Key > >
        class hash_compare
        {
            public:
                static const size_t bucket_size = 4;
                static const size_t min_buckets = 8;
        };
}

#endif //def _GNUHASH_H_

