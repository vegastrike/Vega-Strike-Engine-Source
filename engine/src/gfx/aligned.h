/**
* aligned.h
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
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

#ifndef __ALIGNED_H
#define __ALIGNED_H

#include <memory>
#include <cstddef>

#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7)
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

#if defined(__GNUC__)
#define _ALIGNED(x) __attribute__ ((aligned(x)))
#else
#if defined(_MSC_VER)
#define _ALIGNED(x) __declspec(align(x))
#endif
#endif

template <typename T, int ALIGN=16> class aligned_allocator : public std::allocator<T>
{
public:
    typedef typename std::allocator<T>::pointer pointer;
    typedef typename std::allocator<T>::const_pointer const_pointer;
    typedef typename std::allocator<T>::reference reference;
    typedef typename std::allocator<T>::const_reference const_reference;
    typedef typename std::allocator<T>::value_type value_type;
    typedef typename std::allocator<T>::size_type size_type;
    typedef typename std::allocator<T>::difference_type difference_type;
    
    static const int _OVERHEAD = (sizeof(T) + ALIGN-1) / ALIGN + sizeof(size_t);
    
    aligned_allocator() {}
    template<typename A> explicit aligned_allocator(const A &other) : std::allocator<T>(other) {}
    
    typename std::allocator<T>::pointer address ( typename std::allocator<T>::reference x ) const
    {
        if (sizeof(T) % ALIGN) {
            return std::allocator<T>::address(x);
        } else {
            return __alpn(std::allocator<T>::address(x), ALIGN);
        }
    }
    
    typename std::allocator<T>::const_pointer address ( typename std::allocator<T>::const_reference x ) const
    {
        if (sizeof(T) % ALIGN) {
            return std::allocator<T>::address(x);
        } else {
            return __alpn(std::allocator<T>::address(x), ALIGN);
        }
    }
    
    typename std::allocator<T>::pointer _align (typename std::allocator<void>::const_pointer p)
    {
        if (p == 0)
            return p;
        
        char *vrv;
        
        // Make room for the offset
        vrv = (char*)((size_t*) p + 1);
        
        // Align
        vrv += (ALIGN - (vrv - (char*)NULL) % ALIGN) % ALIGN;
        
        // Store offset
        *((size_t*)vrv - 1) = vrv - (char*)p;
        
        return __alpn(vrv, ALIGN);
    }

    void _dealign (typename std::allocator<T>::pointer p)
    {
        if (p == 0)
            return p;
        
        // De-align
        char *vrv = (char*)p - *((size_t*)p - 1);
        
        return (typename std::allocator<T>::pointer)vrv;
    }
    
    typename std::allocator<T>::pointer allocate (typename std::allocator<T>::size_type n, typename std::allocator<void>::const_pointer hint=0)
    {
        return __alpn(_align(std::allocator<T>::allocate(n+_OVERHEAD, _dealign(hint))), ALIGN);
    }
    
    void deallocate (typename std::allocator<T>::pointer p, typename std::allocator<T>::size_type n)
    {
        std::allocator<T>::deallocate(_dealign(p), n+_OVERHEAD);
    }
};

template<typename ALLOC> class aligned_allocator_traits
{
public:
    static typename ALLOC::pointer 
    ifaligned_start_pointer(const ALLOC &a, typename ALLOC::pointer p)
    {
        return p;
    }

    static typename ALLOC::pointer 
    ifaligned_start_pointer(const ALLOC &a, typename ALLOC::reference r)
    {
        return a.address(r);
    }

    static typename ALLOC::const_pointer 
    ifaligned_start_pointer(const ALLOC &a, typename ALLOC::const_pointer p)
    {
        return p;
    }

    static typename ALLOC::const_pointer 
    ifaligned_start_pointer(const ALLOC &a, typename ALLOC::const_reference r)
    {
        return a.address(r);
    }
};

template<typename T, int ALIGN> class aligned_allocator_traits<aligned_allocator<T, ALIGN> >
{
public:
    static typename aligned_allocator<T, ALIGN>::pointer 
    ifaligned_start_pointer(const aligned_allocator<T, ALIGN> &a, typename aligned_allocator<T, ALIGN>::pointer p)
    {
        return __alpn(p, ALIGN);
    }

    static typename aligned_allocator<T, ALIGN>::pointer 
    ifaligned_start_pointer(const aligned_allocator<T, ALIGN> &a, typename aligned_allocator<T, ALIGN>::reference r)
    {
        return __alpn(a.address(r), ALIGN);
    }

    static typename aligned_allocator<T, ALIGN>::const_pointer 
    ifaligned_start_pointer(const aligned_allocator<T, ALIGN> &a, typename aligned_allocator<T, ALIGN>::const_pointer p)
    {
        return __alpn(p, ALIGN);
    }

    static typename aligned_allocator<T, ALIGN>::const_pointer 
    ifaligned_start_pointer(const aligned_allocator<T, ALIGN> &a, typename aligned_allocator<T, ALIGN>::const_reference r)
    {
        return __alpn(a.address(r), ALIGN);
    }
};

template <typename ALLOC, typename AP, typename UP> 
AP ifaligned_start_pointer(const ALLOC &a, UP p)
{
    return aligned_allocator_traits<ALLOC>::ifaligned_start_pointer(a, p);
}

template <typename COLL> 
typename COLL::const_pointer coll_start_pointer(const COLL &a)
{
    return aligned_allocator_traits<typename COLL::allocator_type>::ifaligned_start_pointer(a.get_allocator(), a[0]);
}

template <typename COLL> 
typename COLL::pointer coll_start_pointer(COLL &a)
{
    return aligned_allocator_traits<typename COLL::allocator_type>::ifaligned_start_pointer(a.get_allocator(), a[0]);
}

#endif

