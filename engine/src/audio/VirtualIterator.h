/**
 * VirtualIterator.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 * Copyright (C) 2022 Stephen G. Tuggy
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


#ifndef __AUDIO_VIRTUALITERATOR_H__INCLUDED__
#define __AUDIO_VIRTUALITERATOR_H__INCLUDED__

#include "Types.h"
#include <iterator>

namespace Audio {

/** Used to derive virtual iterator types
@remarks
    Since all operators are virtual, it is recommended not to use these iterators
    for iterating through large collections...
@note
    By necessity, due to design constraints, no postfix increment/decrement operators
    are defined: those don't get along with virtuality.
*/
template<class _T, class _Rt = _T &, class _Pt = _T *>
class VirtualIterator : public std::iterator<std::bidirectional_iterator_tag, _T> {
public:
    typedef _T value_type;
    typedef _Rt reference_type;
    typedef _Pt pointer_type;
    typedef VirtualIterator<_T, _Rt, _Pt> iterator_type;

    VirtualIterator() {
    }

    virtual ~VirtualIterator() {
    }

    virtual reference_type operator*() = 0;
    virtual pointer_type operator->() = 0;

    virtual iterator_type &operator++() = 0;
    virtual iterator_type &operator--() = 0;

    virtual SharedPtr<iterator_type> clone() const = 0;

    /// End-of-sequence
    virtual bool eos() const = 0;

    /// Start-of-sequence
    virtual bool sos() const = 0;

    /* Aliases
     *
     * since virtual iterators must be handled through pointers, the standard
     * operator interface is highly inconvenient
     */

    reference_type get() {
        return operator*();
    }

    pointer_type getPtr() {
        return operator->();
    }

    iterator_type &next() {
        return operator++();
    }

    iterator_type &prev() {
        return operator--();
    }
};

template<typename _It>
class VirtualStandardIterator :
        public VirtualIterator<
                typename std::iterator_traits<_It>::value_type,
                typename std::iterator_traits<_It>::reference,
                typename std::iterator_traits<_It>::pointer
        > {
    _It begin;
    _It end;
    _It cur;

public:
    // They should be inherited, but somehow gcc 4.1.0 doesn't understand that >:9
    typedef typename std::iterator_traits<_It>::value_type value_type;
    typedef typename std::iterator_traits<_It>::reference reference_type;
    typedef typename std::iterator_traits<_It>::pointer pointer_type;
    typedef VirtualIterator<value_type, reference_type, pointer_type> iterator_type;

    VirtualStandardIterator(const _It &_begin, const _It &_end) : begin(_begin), end(_end), cur(_begin) {
    }

    VirtualStandardIterator(const VirtualStandardIterator<_It> &o) : begin(o.begin), end(o.end), cur(o.cur) {
    }

    virtual reference_type operator*() {
        return *cur;
    };

    virtual pointer_type operator->() {
        return cur.operator->();
    };

    virtual iterator_type &operator++() {
        ++cur;
        return *this;
    };

    virtual iterator_type &operator--() {
        --cur;
        return *this;
    };

    virtual SharedPtr<iterator_type> clone() const {
        return SharedPtr<iterator_type>(new VirtualStandardIterator<_It>(*this));
    };

    virtual bool eos() const {
        return cur == end;
    }

    virtual bool sos() const {
        return cur == begin;
    };
};

template<typename _It, typename _T, typename _Rt=_T &, typename _Pt=_T *>
class VirtualMappingIterator :
        public VirtualIterator<_T, _Rt, _Pt> {
protected:
    VirtualStandardIterator<_It> it;

public:
    // They should be inherited, but somehow gcc 4.1.0 doesn't understand that >:9
    typedef _T value_type;
    typedef _Rt reference_type;
    typedef _Pt pointer_type;
    typedef VirtualIterator<_T, _Rt, _Pt> iterator_type;

    VirtualMappingIterator(const _It &_begin, const _It &_end) :
            it(_begin, _end) {
    }

    VirtualMappingIterator(const iterator_type &o) :
            it(o.it) {
    }

    VirtualMappingIterator(const VirtualStandardIterator<_It> &o) :
            it(o) {
    }

    virtual iterator_type &operator++() {
        ++it;
        return *this;
    };

    virtual iterator_type &operator--() {
        --it;
        return *this;
    };

    virtual bool eos() const {
        return it.eos();
    }

    virtual bool sos() const {
        return it.sos();
    };
};

template<typename _It>
class VirtualValuesIterator :
        public VirtualMappingIterator<_It, typename _It::value_type::second_type> {
public:
    // They should be inherited, but somehow gcc 4.1.0 doesn't understand that >:9
    typedef typename _It::value_type::second_type value_type;
    typedef value_type &reference_type;
    typedef value_type *pointer_type;
    typedef VirtualIterator<value_type, reference_type, pointer_type> iterator_type;

    VirtualValuesIterator(const _It &_begin, const _It &_end) :
            VirtualMappingIterator<_It, value_type>(_begin, _end) {
    }

    VirtualValuesIterator(const VirtualValuesIterator<_It> &o) :
            VirtualMappingIterator<_It, value_type>(o.it) {
    }

    VirtualValuesIterator(const VirtualStandardIterator<_It> &o) :
            VirtualMappingIterator<_It, value_type>(o) {
    }

    virtual reference_type operator*() {
        return VirtualMappingIterator<_It, value_type>::it->second;
    };

    virtual pointer_type operator->() {
        return &(VirtualMappingIterator<_It, value_type>::it->second);
    };

    virtual SharedPtr<iterator_type> clone() const {
        return SharedPtr<iterator_type>(new VirtualValuesIterator(*this));
    }
};

template<typename _It>
class VirtualKeysIterator :
        public VirtualMappingIterator<_It, typename _It::value_type::first_type> {
public:
    // They should be inherited, but somehow gcc 4.1.0 doesn't understand that >:9
    typedef typename _It::value_type::first_type value_type;
    typedef value_type &reference_type;
    typedef value_type *pointer_type;
    typedef VirtualIterator<value_type, reference_type, pointer_type> iterator_type;

    VirtualKeysIterator(const _It &_begin, const _It &_end) :
            VirtualMappingIterator<_It, value_type>(_begin, _end) {
    }

    VirtualKeysIterator(const VirtualKeysIterator<_It> &o) :
            VirtualMappingIterator<_It, value_type>(o.it) {
    }

    VirtualKeysIterator(const VirtualStandardIterator<_It> &o) :
            VirtualMappingIterator<_It, value_type>(o) {
    }

    virtual reference_type operator*() {
        return VirtualMappingIterator<_It, value_type>::it->first;
    };

    virtual pointer_type operator->() {
        return VirtualMappingIterator<_It, value_type>::it->first;
    };

    virtual SharedPtr<iterator_type> clone() const {
        return SharedPtr<iterator_type>(new VirtualKeysIterator(*this));
    }
};

template<typename _It1, typename _It2=_It1>
class ChainingIterator :
        public VirtualIterator<
                typename _It1::value_type,
                typename _It1::reference_type,
                typename _It1::pointer_type
        > {
    _It1 it1;
    _It2 it2;

public:
    // They should be inherited, but somehow gcc 4.1.0 doesn't understand that >:9
    typedef typename _It1::value_type value_type;
    typedef typename _It1::reference_type reference_type;
    typedef typename _It1::pointer_type pointer_type;
    typedef VirtualIterator<value_type, reference_type, pointer_type> iterator_type;

    ChainingIterator() {
    }

    ChainingIterator(const ChainingIterator<_It1, _It2> &o) : it1(o.it1), it2(o.it2) {
    }

    ChainingIterator(_It1 _it1, _It2 _it2) : it1(_it1), it2(_it2) {
    }

    virtual ~ChainingIterator() {
    }

    virtual reference_type operator*() {
        return (it1.eos() ? *it2 : *it1);
    }

    virtual pointer_type operator->() {
        return (it1.eos() ? it2.operator->() : it1.operator->());
    }

    virtual iterator_type &operator++() {
        if (it1.eos()) {
            ++it2;
        } else {
            ++it1;
        }
        return *this;
    }

    virtual iterator_type &operator--() {
        if (it1.eos() && !it2.sos()) {
            --it2;
        } else {
            --it1;
        }
        return *this;
    }

    virtual SharedPtr<iterator_type> clone() const {
        return SharedPtr<iterator_type>(new ChainingIterator<_It1, _It2>(it1, it2));
    }

    virtual bool eos() const {
        return (it1.eos() && it2.eos());
    }

    virtual bool sos() const {
        return (it1.sos() && it2.sos());
    }
};

}

#endif//__AUDIO_VIRTUALITERATOR_H__INCLUDED__
