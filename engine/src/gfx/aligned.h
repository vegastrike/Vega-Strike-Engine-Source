/*
 * aligned.h
 *
 * Vega Strike - Space Simulation, Combat and Trading
 * Copyright (C) 2001-2025 The Vega Strike Contributors:
 * Project creator: Daniel Horn
 * Original development team: As listed in the AUTHORS file; klaussfreire specifically
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
#ifndef VEGA_STRIKE_ENGINE_GFX_ALIGNED_H
#define VEGA_STRIKE_ENGINE_GFX_ALIGNED_H

#include <memory>
#include <cstddef>
#include <cstdint>

#if defined(__cpp_lib_assume_aligned)
    #define __alpn(x,a) std::assume_aligned<a>(x)
    #define __alprn(x,a) std::assume_aligned<a>(x)
    #define __alp(x) __alpn(x,16)
    #define __alpr(x) __alprn(x,16)
#elif __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7)
    #define __alpn(x,a) x
    #define __alprn(x,a) x
    #define __alp(x) x
    #define __alpr(x) x
#else
    #define __alpn(x,a) ((typeof(x))(__builtin_assume_aligned(x,a)))
    #define __alprn(x,a) ((typeof(x) __restrict__)(__builtin_assume_aligned(x,a)))
    #define __alp(x) __alpn(x,16)
    #define __alpr(x) __alprn(x,16)
#endif

#define _ALIGNED(x) alignas(x)

template<typename T, typename U>
  using Rebind = typename std::allocator_traits<T>::template rebind_alloc<U>;

template<typename T, typename = T>
  struct HasRebind {
    using value_type = T;
    template<typename U> struct rebind { using other = HasRebind<U>; };
  };

template<typename T, int ALIGN = 16>
class aligned_allocator : public std::allocator<T> {
public:
    typedef typename std::allocator<T>::pointer pointer;
    typedef typename std::allocator<T>::const_pointer const_pointer;
    typedef typename std::allocator<T>::reference reference;
    typedef typename std::allocator<T>::const_reference const_reference;
    typedef typename std::allocator<T>::value_type value_type;
    typedef typename std::allocator<T>::size_type size_type;
    typedef typename std::allocator<T>::difference_type difference_type;

    static const int _OVERHEAD = (sizeof(T) + ALIGN - 1) / ALIGN + sizeof(size_t);

    aligned_allocator() = default;

    template<typename U> struct rebind {
        using value_type = T;
        using other = aligned_allocator<U>;
    };

    template<typename A>
    explicit aligned_allocator(const A &other) : std::allocator<T>(other) {
    }

    typename std::allocator<T>::pointer address(typename std::allocator<T>::reference x) const {
        if (sizeof(T) % ALIGN) {
            return std::allocator<T>::address(x);
        } else {
            return __alpn(std::allocator<T>::address(x), ALIGN);
        }
    }

    typename std::allocator<T>::const_pointer address(typename std::allocator<T>::const_reference x) const {
        if (sizeof(T) % ALIGN) {
            return std::allocator<T>::address(x);
        } else {
            return __alpn(std::allocator<T>::address(x), ALIGN);
        }
    }

    typename std::allocator<T>::pointer _align(typename std::allocator<void>::const_pointer p) {
        if (p == 0) {
            return static_cast<typename std::allocator<T>::pointer>(const_cast<typename std::allocator<void>::pointer>(p));
        }

        uint8_t *vrv;

        // Make room for the offset
        vrv = (uint8_t *) ((size_t *) p + 1);

        // Align
        vrv += (ALIGN - (vrv - (uint8_t *) NULL) % ALIGN) % ALIGN;

        // Store offset
        *((size_t *) vrv - 1) = vrv - (uint8_t *) p;

        void *vrv2 = static_cast<void *>(vrv);
        typename std::allocator<T>::pointer vrv3 = static_cast<typename std::allocator<T>::pointer>(vrv2);

        return __alpn(vrv3, ALIGN);
    }

    typename std::allocator<T>::pointer _dealign(typename std::allocator<T>::pointer p) {
        if (p == 0) {
            return p;
        }

        // Convert our input to pointer-to-void as an intermediate step
        void *p2 = static_cast<void *>(p);

        // De-align
        uint8_t *vrv = (uint8_t *) p2 - *((size_t *) p2 - 1);

        // Now convert back to pointer-to-void on the way out
        void *vrv2 = static_cast<void *>(vrv);

        // And back to the specific pointer type desired
        typename std::allocator<T>::pointer vrv3 = static_cast<typename std::allocator<T>::pointer>(vrv2);

        // And return
        return vrv3;
    }

    typename std::allocator<T>::pointer
    allocate(typename std::allocator<T>::size_type n, typename std::allocator<void>::const_pointer hint = 0) {
        typename std::allocator<void>::pointer hint2 = const_cast<typename std::allocator<void>::pointer>(hint);
        typename std::allocator<T>::pointer hint3 = static_cast<typename std::allocator<T>::pointer>(hint2);

        return __alpn(_align(std::allocator<T>::allocate(n + _OVERHEAD, _dealign(hint3))), ALIGN);
    }

    void deallocate(typename std::allocator<T>::pointer p, typename std::allocator<T>::size_type n) {
        std::allocator<T>::deallocate(_dealign(p), n + _OVERHEAD);
    }
};

template<typename ALLOC>
class aligned_allocator_traits {
public:
    static typename ALLOC::pointer
    ifaligned_start_pointer(const ALLOC &a, typename ALLOC::pointer p) {
        return p;
    }

    static typename ALLOC::pointer
    ifaligned_start_pointer(const ALLOC &a, typename ALLOC::reference r) {
        return a.address(r);
    }

    static typename ALLOC::const_pointer
    ifaligned_start_pointer(const ALLOC &a, typename ALLOC::const_pointer p) {
        return p;
    }

    static typename ALLOC::const_pointer
    ifaligned_start_pointer(const ALLOC &a, typename ALLOC::const_reference r) {
        return a.address(r);
    }
};

template<typename T, int ALIGN>
class aligned_allocator_traits<aligned_allocator<T, ALIGN> > {
public:
    static typename aligned_allocator<T, ALIGN>::pointer
    ifaligned_start_pointer(const aligned_allocator<T, ALIGN> &a, typename aligned_allocator<T, ALIGN>::pointer p) {
        return __alpn(p, ALIGN);
    }

    static typename aligned_allocator<T, ALIGN>::pointer
    ifaligned_start_pointer(const aligned_allocator<T, ALIGN> &a, typename aligned_allocator<T, ALIGN>::reference r) {
        return __alpn(a.address(r), ALIGN);
    }

    static typename aligned_allocator<T, ALIGN>::const_pointer
    ifaligned_start_pointer(const aligned_allocator<T, ALIGN> &a,
            typename aligned_allocator<T, ALIGN>::const_pointer p) {
        return __alpn(p, ALIGN);
    }

    static typename aligned_allocator<T, ALIGN>::const_pointer
    ifaligned_start_pointer(const aligned_allocator<T, ALIGN> &a,
            typename aligned_allocator<T, ALIGN>::const_reference r) {
        return __alpn(a.address(r), ALIGN);
    }
};

template<typename ALLOC, typename AP, typename UP>
AP ifaligned_start_pointer(const ALLOC &a, UP p) {
    return aligned_allocator_traits<ALLOC>::ifaligned_start_pointer(a, p);
}

template<typename COLL>
typename COLL::const_pointer coll_start_pointer(const COLL &a) {
    return aligned_allocator_traits<typename COLL::allocator_type>::ifaligned_start_pointer(a.get_allocator(), a[0]);
}

template<typename COLL>
typename COLL::pointer coll_start_pointer(COLL &a) {
    return aligned_allocator_traits<typename COLL::allocator_type>::ifaligned_start_pointer(a.get_allocator(), a[0]);
}

#endif //VEGA_STRIKE_ENGINE_GFX_ALIGNED_H
