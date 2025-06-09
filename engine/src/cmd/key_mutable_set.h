/*
 * key_mutable_set.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
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
#ifndef VEGA_STRIKE_ENGINE_CMD_KEY_MUTABLE_SET_H
#define VEGA_STRIKE_ENGINE_CMD_KEY_MUTABLE_SET_H

#include <set>
#include <assert.h>
#include <list>

template<class T>
class MutableShell {
    mutable T t;
public:
//flatten restores sortedness--in this case it does nothing
    void flatten() {
    }

    MutableShell(const T &t) : t(t) {
    }

    T &get() const {
        return t;
    }

    T &operator*() const {
        return t;
    }

    T *operator->() const {
        return &t;
    }

    operator T &() const {
        return t;
    }

    bool operator<(const MutableShell<T> &other) const {
        return t < other.t;
    }
};

///This set inherits from the STL multiset, with a slight variation: The value is nonconst--that means you are allowed to change things but must not alter the key.
///This set inherits from the STL multiset, with a slight variation: You are allowed to update the key of a particular iterator that you have obtained.
/** Note: T is the type that each element is pointing to. */
template<class T, class _Compare = std::less<MutableShell<T> > >
class KeyMutableSet : public std::multiset<MutableShell<T>, _Compare> {
    typedef std::multiset<MutableShell<T>, _Compare> SUPER;
public:
/// This just checks the order of the set for testing purposes..
    void checkSet() {
        _Compare comparator;
        if (this->begin() != this->end()) {
            for (typename SUPER::iterator newiter = this->begin(), iter = newiter++; newiter != this->end();
                    iter = newiter++)
                assert(!comparator(*newiter, *iter));
        }
    }
///Given an iterator you can alter that iterator's key to be the one passed in.
/** The type must have a function called changeKey(const Key &newKey) that
 *       changes its key to the specified new key.
 */

    void changeKey(typename SUPER::iterator &iter,
            const T &newKey,
            typename SUPER::iterator &templess,
            typename SUPER::iterator &rettempmore) {
        MutableShell<T> newKeyShell(newKey);
        templess = rettempmore = iter;
        ++rettempmore;
        typename SUPER::iterator tempmore = rettempmore;
        if (tempmore == this->end()) {
            --tempmore;
        }
        if (templess != this->begin()) {
            --templess;
        }
        _Compare comparator;

        //O(1) amortized time on the insertion - Yippie!
        bool byebye = false;
        if (comparator(newKeyShell, *templess)) {
            rettempmore = templess = this->insert(newKeyShell, templess);
            byebye = true;
        } else if (comparator(*tempmore, newKeyShell)) {
            rettempmore = templess = this->insert(newKeyShell, tempmore);
            byebye = true;
        } else {
            (*iter).get() = newKey;
        }
        if (byebye) {
            this->erase(iter);
            iter = templess;
            ++rettempmore;
            if (templess != this->begin()) {
                --templess;
            }
        }
    }

    typename SUPER::iterator insert(const T &newKey, typename SUPER::iterator hint) {
        return this->SUPER::insert(hint, newKey);
    }

    typename SUPER::iterator insert(const T &newKey) {
        return this->SUPER::insert(newKey);
    }

    typename SUPER::iterator changeKey(typename SUPER::iterator iter, const T &newKey) {
        typename SUPER::iterator templess = iter, tempmore = iter;
        changeKey(iter, newKey, templess, tempmore);
        return iter;
    }
};

template<class T, class _Compare = std::less<T> >
class ListMutableSet : public std::list<T> {
    typedef std::list<T> SUPER;
public:
//flatten restores sortedness--in this case it does nothing
    void flatten() {
    }

/// This just checks the order of the set for testing purposes..
    void checkSet() {
        _Compare comparator;
        if (this->begin() != this->end()) {
            for (typename SUPER::iterator newiter = this->begin(), iter = newiter++; newiter != this->end();
                    iter = newiter++)
                assert(!comparator(*newiter, *iter));
        }
    }
///Given an iterator you can alter that iterator's key to be the one passed in.
/** The type must have a function called changeKey(const Key &newKey) that
 *       changes its key to the specified new key.
 */
    void changeKey(typename SUPER::iterator &iter,
            const T &newKey,
            typename SUPER::iterator &templess,
            typename SUPER::iterator &rettempmore) {
        MutableShell<T> newKeyShell(newKey);
        templess = rettempmore = iter;
        ++rettempmore;
        typename SUPER::iterator tempmore = rettempmore;
        if (tempmore == this->end()) {
            --tempmore;
        }
        if (templess != this->begin()) {
            --templess;
        }
        _Compare comparator;
        if (comparator(newKeyShell, *templess) || comparator(*tempmore, newKeyShell)) {
            rettempmore = templess = iter = this->insert(newKeyShell, this->erase(iter));
            ++rettempmore;
            if (templess != this->begin()) {
                --templess;
            }
        } else {
            **iter = newKey;
            //return iter;
        }
    }

    typename SUPER::iterator changeKey(typename SUPER::iterator iter, const T &newKey) {
        typename SUPER::iterator templess = iter, tempmore = iter;
        changeKey(iter, newKey, templess, tempmore);
        return iter;
    }

    typename SUPER::iterator insert(const T &newKey, typename SUPER::iterator hint) {
        bool gequal = false, lequal = false;
        _Compare comparator;
        while (1) {
            if (hint != this->end()) {
                bool tlequal = !comparator(*hint, newKey);
                bool tgequal = !comparator(newKey, *hint);
                if (tlequal) {
                    if (gequal || tgequal || hint == this->begin()) {
                        return this->SUPER::insert(hint, newKey);
                    } else {
                        lequal = true;
                        --hint;
                    }
                }
                if (tgequal) {
                    gequal = true;
                    ++hint;
                    if (lequal || tlequal) {
                        return this->SUPER::insert(hint, newKey);
                    }
                }
            } else if (hint == this->begin()) {
                this->SUPER::insert(hint, newKey);
            } else {
                if (gequal) {
                    return this->SUPER::insert(hint, newKey);
                } else {
                    --hint;
                }
            }
        }
    }

    typename SUPER::iterator insert(const T &newKey) {
        return this->insert(newKey, this->begin());
    }
};

#endif //VEGA_STRIKE_ENGINE_CMD_KEY_MUTABLE_SET_H
