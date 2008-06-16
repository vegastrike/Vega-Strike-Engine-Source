//
// C++ Interface: Audio::Codec
//
#ifndef __AUDIO_TYPES_H__INCLUDED__
#define __AUDIO_TYPES_H__INCLUDED__

#include <boost/smart_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

// Some compilers don't like template typedefs
#define SharedPtr boost::shared_ptr
#define WeakPtr boost::weak_ptr
#define AutoPtr boost::scoped_ptr
#define SharedFromThis boost::enable_shared_from_this

#include "Vector.h"

namespace Audio {

    /** Generic fp scalar type */
    typedef float Scalar;
    
    /** Long fp scalar type */
    typedef double LScalar;
    
    /** Tiemstamp type */
    typedef LScalar Timestamp;

    /** Duration type */
    typedef Scalar Duration;

    /** FP 3D vector */
    typedef TVector3<Scalar> Vector3;
    
    /** Long FP 3D vector */
    typedef TVector3<LScalar> LVector3;

    /** Per-frequency data usually comes in lf/hf bundles */
    template<typename T> struct PerFrequency {
        T lf;
        T hf;
        
        PerFrequency(T _lf, T _hf) : lf(_lf), hf(_hf) {}
    };
    
    /** Range data usually comes in min/max bundles */
    template<typename T> struct Range {
        T min;
        T max;
        
        Range(T mn, T mx) : min(mn), max(mx) {}
    };
    
    /** Generic user data container interface */
    class UserData {
    public:
        virtual ~UserData() {};
    };

};

#endif//__AUDIO_TYPES_H__INCLUDED__
