/**
 * vectorizable.h
 *
 * Copyright (c) 2001-2002 Daniel Horn
 * Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
 * Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
 * Copyright (C) 2022-2023 Stephen G. Tuggy, Benjamen R. Meyer
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
#ifndef VEGA_STRIKE_ENGINE_GFX_VECTORIZABLE_H
#define VEGA_STRIKE_ENGINE_GFX_VECTORIZABLE_H

#include <boost/static_assert.hpp>

#include <vector>
#include "aligned.h"

/**
 * Defines classes as vectorizable, and defines means of vectorization.
 * The default implementation makes all classes non-vectorizable, so vectorizable
 * classes must be defined as such.
 */
template<typename T>
class vectorizable_traits {
public:
    static const bool vectorizable = false;
    static const size_t components_per_element = 1;
    typedef T component_type;
    typedef T element_type;
    typedef T vector_type;

    /**
     * Returns the number of elements a vectorizable instance has.
     * Each element has components_per_element components.
     */
    static size_t element_count(const vector_type &) {
        return 1;
    }

    /**
     * Returns a pointer to the first component on the vectorized
     * view. There will be components_per_element * element_count
     * consecutive elements.
     */
    static const component_type *begin(const vector_type &v) {
        return &v;
    }

    /**
     * Returns a pointer to the first component on the vectorized
     * view. There will be components_per_element * element_count
     * consecutive elements.
     */
    static component_type *begin(vector_type &v) {
        return &v;
    }

    /**
     * Returns a pointer to the last component on the vectorized
     * view. Equivalent to begin() + be components_per_element * element_count
     */
    static const component_type *end(const vector_type &v) {
        return begin(v) + element_count(v) * components_per_element;
    }

    /**
     * Returns a pointer to the last component on the vectorized
     * view. Equivalent to begin() + be components_per_element * element_count
     */
    static component_type *end(vector_type &v) {
        return begin(v) + element_count(v) * components_per_element;
    }
};

/**
 * All primitive types are trivially vectorizable
 */
template<typename T>
class primitive_vectorizable_traits {
public:
    static const bool vectorizable = true;
    static const size_t components_per_element = 1;
    typedef T component_type;
    typedef T element_type;
    typedef T vector_type;

    /**
     * Returns the number of elements a vectorizable instance has.
     * Each element has components_per_element components.
     */
    static size_t element_count(const vector_type &) {
        return 1;
    }

    /**
     * Returns a pointer to the first component on the vectorized
     * view. There will be components_per_element * element_count
     * consecutive elements.
     */
    static const component_type *begin(const vector_type &v) {
        return &v;
    }

    /**
     * Returns a pointer to the first component on the vectorized
     * view. There will be components_per_element * element_count
     * consecutive elements.
     */
    static component_type *begin(vector_type &v) {
        return &v;
    }

    /**
     * Returns a pointer to the last component on the vectorized
     * view. Equivalent to begin() + be components_per_element * element_count
     */
    static const component_type *end(const vector_type &v) {
        return begin(v) + element_count(v) * components_per_element;
    }

    /**
     * Returns a pointer to the last component on the vectorized
     * view. Equivalent to begin() + be components_per_element * element_count
     */
    static component_type *end(vector_type &v) {
        return begin(v) + element_count(v) * components_per_element;
    }
};

/**
 * Helper base class for struct-based vectorizables
 */
template<typename E, typename T, int COMPS>
class struct_vectorizable_traits {
public:
    static const bool vectorizable = true;
    static const size_t components_per_element = COMPS;
    typedef T component_type;
    typedef E element_type;
    typedef E vector_type;

private:
    BOOST_STATIC_ASSERT(sizeof(element_type) == (sizeof(component_type) * COMPS));

public:

    /**
     * Returns the number of elements a vectorizable instance has.
     * Each element has components_per_element components.
     */
    static size_t element_count(const vector_type &) {
        return 1;
    }

    /**
     * Returns a pointer to the first component on the vectorized
     * view. There will be components_per_element * element_count
     * consecutive elements.
     */
    static const component_type *begin(const vector_type &v) {
        return &v;
    }

    /**
     * Returns a pointer to the first component on the vectorized
     * view. There will be components_per_element * element_count
     * consecutive elements.
     */
    static component_type *begin(vector_type &v) {
        return &v;
    }

    /**
     * Returns a pointer to the last component on the vectorized
     * view. Equivalent to begin() + be components_per_element * element_count
     */
    static const component_type *end(const vector_type &v) {
        return begin(v) + element_count(v) * components_per_element;
    }

    /**
     * Returns a pointer to the last component on the vectorized
     * view. Equivalent to begin() + be components_per_element * element_count
     */
    static component_type *end(vector_type &v) {
        return begin(v) + element_count(v) * components_per_element;
    }
};

template<>
class vectorizable_traits<unsigned char> : public primitive_vectorizable_traits<unsigned char> {};
template<>
class vectorizable_traits<char> : public primitive_vectorizable_traits<char> {};
template<>
class vectorizable_traits<unsigned short> : public primitive_vectorizable_traits<unsigned short> {};
template<>
class vectorizable_traits<short> : public primitive_vectorizable_traits<short> {};
template<>
class vectorizable_traits<unsigned int> : public primitive_vectorizable_traits<unsigned int> {};
template<>
class vectorizable_traits<int> : public primitive_vectorizable_traits<int> {};
template<>
class vectorizable_traits<unsigned long> : public primitive_vectorizable_traits<unsigned long> {};
template<>
class vectorizable_traits<long> : public primitive_vectorizable_traits<long> {};
template<>
class vectorizable_traits<float> : public primitive_vectorizable_traits<float> {};
template<>
class vectorizable_traits<double> : public primitive_vectorizable_traits<double> {};

/**
 * All vectors of vectorizable types are vectorizable
 */
template<typename T, typename A>
class vectorizable_traits<std::vector<T, A> > {
public:
    static const bool vectorizable = true;
    static const size_t components_per_element = 1;
    typedef T element_type;
    typedef std::vector<T, A> vector_type;
    typedef typename vectorizable_traits<T>::component_type component_type;

    /**
     * Returns the number of elements a vectorizable instance has.
     * Each element has components_per_element components.
     */
    static size_t element_count(const vector_type &v) {
        return v.size();
    }

    /**
     * Returns a pointer to the first component on the vectorized
     * view. There will be components_per_element * element_count
     * consecutive elements.
     */
    static component_type *begin(vector_type &v) {
        return (component_type *) coll_start_pointer(v);
    }

    /**
     * Returns a pointer to the first component on the vectorized
     * view. There will be components_per_element * element_count
     * consecutive elements.
     */
    static const component_type *begin(const vector_type &v) {
        return (component_type *) coll_start_pointer(v);
    }

    /**
     * Returns a pointer to the last component on the vectorized
     * view. Equivalent to begin() + be components_per_element * element_count
     */
    static component_type *end(vector_type &v) {
        return begin(v) + element_count(v) * components_per_element;
    }

    /**
     * Returns a pointer to the last component on the vectorized
     * view. Equivalent to begin() + be components_per_element * element_count
     */
    static const component_type *end(const vector_type &v) {
        return begin(v) + element_count(v) * components_per_element;
    }
};

#endif //VEGA_STRIKE_ENGINE_GFX_VECTORIZABLE_H
