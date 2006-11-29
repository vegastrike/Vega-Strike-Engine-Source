#include "SharedPool.h"
#include <string>

template <typename T, typename RT> SharedPool<T,RT> * SharedPool<T,RT>::ms_singleton = 0;

template<typename T, typename RT> SharedPool<T,RT>::SharedPool()
#ifdef __GLIBC__
	: referenceCounter(RT::min_buckets)
#endif
{
	if (ms_singleton == 0)
		ms_singleton = this;
}

template<typename T, typename RT> SharedPool<T,RT>::~SharedPool()
{
	if (ms_singleton == this)
		ms_singleton = 0;
}


class SharedPoolInitializer
{
	StringPool stringPool;
};

static SharedPoolInitializer sharedPoolInitializer;
