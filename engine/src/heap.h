/*
 * heap.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file; Claudio Freire specifically
 * Current development team: Roy Falk, Benjamen R. Meyer, Stephen G. Tuggy
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
#ifndef VEGA_STRIKE_ENGINE_HEAP_H
#define VEGA_STRIKE_ENGINE_HEAP_H

#include <vector>
#include <algorithm>
#include <functional>

namespace VS {

template<typename T, typename Comp = std::less<T> >
class priority_queue : protected std::vector<T> {
    Comp comp;

    typedef typename std::vector<T>::iterator iterator;

    iterator _begin() {
        return std::vector<T>::begin();
    }

    iterator _end() {
        return std::vector<T>::end();
    }

public:
    typedef typename std::vector<T>::value_type value_type;
    typedef typename std::vector<T>::reference reference;
    typedef typename std::vector<T>::const_reference const_reference;
    typedef typename std::vector<T>::const_iterator const_iterator;
    typedef typename std::vector<T>::size_type size_type;

    priority_queue() {
    }

    explicit priority_queue(const Comp &comp_) : comp(comp_) {
    }

    template<typename RanIT>
    explicit priority_queue(RanIT start, RanIT end)
            : std::vector<T>(start, end) {
        std::make_heap(begin(), end(), comp);
    }

    template<typename RanIT>
    explicit priority_queue(RanIT start, RanIT end, const Comp &comp_)
            : std::vector<T>(start, end), comp(comp_) {
        std::make_heap(_begin(), _end(), comp);
    }

    void clear() {
        std::vector<T>::clear();
    }

    size_type size() const {
        return std::vector<T>::size();
    }

    void push(const_reference value) {
        std::vector<T>::push_back(value);
        std::push_heap(_begin(), _end(), comp);
    }

    const_reference top() const {
        return std::vector<T>::front();
    }

    void pop() {
        std::pop_heap(_begin(), _end(), comp);
        std::vector<T>::pop_back();
    }

    const_iterator begin() const {
        return std::vector<T>::begin();
    }

    const_iterator end() const {
        return std::vector<T>::end();
    }
};

}

#endif //VEGA_STRIKE_ENGINE_HEAP_H
