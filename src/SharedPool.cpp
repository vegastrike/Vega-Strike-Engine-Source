#include "SharedPool.h"

StringPool* StringPool::ms_singleton = 0;

template<typename T> SharedPool<T>::SharedPool()
{
	if (ms_singleton == 0)
		ms_singleton = this;
}

template<typename T> SharedPool<T>::~SharedPool()
{
	if (ms_singleton == this)
		ms_singleton = 0;
}


class SharedPoolInitializer
{
	StringPool stringPool;
};

static SharedPoolInitializer sharedPoolInitializer;
