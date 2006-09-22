#ifndef __STRINGPOOL_H__INCLUDED__
#define __STRINGPOOL_H__INCLUDED__

#include <string>
#include "gnuhash.h"

// Need reference counted strings, or we'll eat memory like crazy
template<class T> class SharedPool {
public:
	typedef stdext::hash_map<T,unsigned int> ReferenceCounter;

private:
	ReferenceCounter referenceCounter;
	static SharedPool<T> *ms_singleton;

public:
	static SharedPool<T>& getSingleton()
	{
		return *ms_singleton;
	}

	static SharedPool<T>* getSingletonPtr()
	{
		return ms_singleton;
	}

	SharedPool();
	~SharedPool();

public:
	class Reference
	{
		typename ReferenceCounter::iterator _it;
		ReferenceCounter *_rc;

		void unref()
		{
			if ( _rc && (_it != _rc->end()) ) {
				if ( (_it->second==0) || ((--(_it->second))==0) )
					_rc->erase(_it);
				_it = _rc->end();
			}
		}

		void ref()
		{
			if (_rc && (_it != _rc->end()))
				++(_it->second);
		}

	public:
		Reference() : 
			_it(SharedPool<T>::getSingleton().referenceCounter.end()), 
			_rc(&SharedPool<T>::getSingleton().referenceCounter)
		{
		}

		explicit Reference(const T& s) :
			_it(SharedPool<T>::getSingleton().referenceCounter.end()), 
			_rc(&SharedPool<T>::getSingleton().referenceCounter)
		{
			set(s);
		}

		explicit Reference(ReferenceCounter *pool) :
			_it(pool->end()), _rc(pool)
		{
		}

		Reference(ReferenceCounter *pool, const T &s) :
			_it(pool->end()), _rc(pool)
		{
			set(s);
		}

		Reference(const Reference &other) :
			_it(other._it), _rc(other._rc)
		{
			ref();
		}

		~Reference()
		{
			unref();
		}

		const T& get() const
		{
			static T empty_value;
			return (_rc&&(_it!=_rc->end()))?_it->first:empty_value;
		}

		Reference& set(const T& s)
		{
			unref();
			if (_rc) {
				_it = _rc->insert(std::pair<T,unsigned int>(s,0)).first;
				ref();
			}
			return *this;
		}

		operator const T&() const
		{
			return get();
		}

		Reference& operator=(const T& s) 
		{
			return set(s);
		}

		Reference& operator=(const Reference& s) 
		{
			if (this == &s)
				return *this;
			if (s._rc == _rc) {
				unref();
				_it = s._it;
				ref();
			} else {
				set(s.get());
			}
			return *this;
		}

		bool operator==(const T& s) const
		{
			return get()==s;
		}

		bool operator==(const Reference& r) const
		{
			if (_rc && (_rc == r._rc))
				return _it == r._it; else
				return get() == r.get();
		}

		bool operator<(const T& s) const
		{
			return get()<s;
		}

		bool operator<(const Reference& r) const
		{
			return get()<r.get();
		}

		bool operator!=(const T& s) const
		{
			return get()!=s;
		}

		bool operator!=(const Reference& r) const
		{
			return !(*this==r);
		}

	};

	Reference get(const T &s)
	{
		return Reference(&referenceCounter,s);
	}

	Reference get()
	{
		return Reference(&referenceCounter);
	}
	friend class SharedPool<T>::Reference;
};

typedef SharedPool<std::string> StringPool;

static StringPool stringPool;

inline std::string operator+(const std::string &s, const StringPool::Reference &r)
{
	return s+r.get();
}

inline std::string operator+(const StringPool::Reference &r, const std::string &s)
{
	return r.get()+s;
}

template<typename T> inline T& operator<<(T& stream, const StringPool::Reference& ref)
{
	return stream << ref.get();
}

template<typename T> bool operator==(const typename SharedPool<T>::Reference &r, const T& s) 
{
	return r.get()==s;
}

#endif//__STRINGPOOL_H__INCLUDED__
