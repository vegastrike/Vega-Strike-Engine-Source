/*
 * preferred_types.h
 *
 * Copyright (C) 2001-2022 Daniel Horn, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef VEGA_STRIKE_PREFERRED_TYPES_H
#define VEGA_STRIKE_PREFERRED_TYPES_H

//#include <deque>
//#include <vector>
#include <stack>
//#include <map>
//#include <unordered_map>
#include <boost/smart_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/container/deque.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/stable_vector.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/map.hpp>
#include <boost/container/set.hpp>

namespace vega_types {

template<typename T>
using SharedPtr = boost::shared_ptr<T>;
template<typename T>
using WeakPtr = boost::weak_ptr<T>;
template<typename T>
using AutoPtr = boost::scoped_ptr<T>;

template<class T>
using EnableSharedFromThis = boost::enable_shared_from_this<T>;

template< class T, class... Args >
SharedPtr<T> MakeShared( Args&&... args ) {
    return boost::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T>
using SequenceContainer = boost::container::deque<T>;
template<typename T>
using ContiguousSequenceContainer = boost::container::vector<T>;

template<typename T>
using Deque = boost::container::deque<T>;

/// Leaving out the following, because it conflicts with the math-type Vector typename
//template<typename T>
//using Vector = boost::container::vector<T>;

template<typename T>
using Stack = std::stack<T>;

template<typename K, typename V>
using OrderedAssociativeContainer = boost::container::map<K, V>;
template<typename K, typename V>
using UnorderedAssociativeContainer = boost::container::flat_map<K, V>;

template<typename K, typename V>
using Map = boost::container::map<K, V>;
template<typename K, typename V>
using FlatMap = boost::container::flat_map<K, V>;

template<class Key, class Compare = std::less<Key>>
using Set = boost::container::set<Key, Compare>;

}

#endif //VEGA_STRIKE_PREFERRED_TYPES_H
