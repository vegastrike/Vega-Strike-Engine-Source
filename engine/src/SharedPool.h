/*
 * SharedPool.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
 *
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef VEGA_STRIKE_ENGINE_STRINGPOOL_H
#define VEGA_STRIKE_ENGINE_STRINGPOOL_H

#include <string>
#include "gnuhash.h"

#ifndef INITIAL_STRINGPOOL_SIZE
#define INITIAL_STRINGPOOL_SIZE (1<<15)
#endif

//Need reference counted strings, or we'll eat memory like crazy
template<class T, class RefcounterTraits = vsHashComp<T> >
class SharedPool {
public:
    typedef vsUMap<T, unsigned int> ReferenceCounter;
    typedef SharedPool<T, RefcounterTraits> PoolType;

private:
    ReferenceCounter referenceCounter;
    static PoolType *ms_singleton;

public:
    typedef T ValueType;
    typedef RefcounterTraits RefocounterTraitsType;

    static PoolType &getSingleton() {
        return *ms_singleton;
    }

    static PoolType *getSingletonPtr() {
        return ms_singleton;
    }

    SharedPool();
    ~SharedPool();

public:
    class Reference {
        typename ReferenceCounter::iterator _it;
        ReferenceCounter *_rc;

        void unref() {
            if (_rc && (_it != _rc->end())) {
                if ((_it->second == 0) || ((--(_it->second)) == 0)) {
                    _rc->erase(_it);
                }
                _it = _rc->end(); // Should this be inside the if block?
            }
        }

        void ref() {
            if (_rc && (_it != _rc->end())) {
                ++(_it->second);
            }
        }

    public:
        Reference() :
                _it(SharedPool::getSingleton().referenceCounter.end()),
                _rc(&SharedPool::getSingleton().referenceCounter) {
        }

        explicit Reference(const T &s) :
                _it(SharedPool::getSingleton().referenceCounter.end()),
                _rc(&SharedPool::getSingleton().referenceCounter) {
            set(s);
        }

        explicit Reference(ReferenceCounter *pool) :
                _it(pool->end()), _rc(pool) {
        }

        Reference(ReferenceCounter *pool, const T &s) :
                _it(pool->end()), _rc(pool) {
            set(s);
        }

        Reference(const Reference &other) :
                _it(other._it), _rc(other._rc) {
            ref();
        }

        ~Reference() {
            unref();
        }

        const T &get() const {
            static T empty_value;
            if (_rc && (_it != _rc->end()) && (_it->second > 0)) {
                return _it->first;
            } else {
                return empty_value;
            }
        }

        Reference &set(const T &s) {
            unref();
            if (_rc) {
                _it = _rc->insert(std::pair<T, unsigned int>(s, 0)).first;
                ref();
            }
            return *this;
        }

        operator const T &() const {
            return get();
        }

        Reference &operator=(const T &s) {
            return set(s);
        }

        Reference &operator=(const Reference &s) {
            if (this == &s) {
                return *this;
            }
            if (s._rc == _rc) {
                unref();
                _it = s._it;
                ref();
            } else {
                set(s.get());
            }
            return *this;
        }

        bool operator==(const T &s) const {
            return get() == s;
        }

        bool operator==(const Reference &r) const {
            if (_rc && (_rc == r._rc)) {
                return _it == r._it;
            } else {
                return get() == r.get();
            }
        }

        bool operator<(const T &s) const {
            return get() < s;
        }

        bool operator<(const Reference &r) const {
            return get() < r.get();
        }

        bool operator!=(const T &s) const {
            return get() != s;
        }

        bool operator!=(const Reference &r) const {
            return !(*this == r);
        }
    };

    Reference get(const T &s) {
        return Reference(&referenceCounter, s);
    }

    Reference get() {
        return Reference(&referenceCounter);
    }

    friend class PoolType::Reference;
};

class StringpoolTraits : public vsHashComp<std::string> {
public:
    enum { min_buckets = INITIAL_STRINGPOOL_SIZE };
};

typedef SharedPool<std::string, StringpoolTraits> StringPool;

static StringPool stringPool;

inline std::string operator+(const std::string &s, const StringPool::Reference &r) {
    return s + r.get();
}

inline std::string operator+(const StringPool::Reference &r, const std::string &s) {
    return r.get() + s;
}

template<typename T>
inline T &operator<<(T &stream, const StringPool::Reference &ref) {
    return stream << ref.get();
}

#include "SharedPool.cpp"

#endif //VEGA_STRIKE_ENGINE_STRINGPOOL_H
