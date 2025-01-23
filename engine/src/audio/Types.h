/*
 * Types.h
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
#ifndef VEGA_STRIKE_ENGINE_AUDIO_TYPES_H
#define VEGA_STRIKE_ENGINE_AUDIO_TYPES_H

//
// C++ Interface: Audio::Codec
//

#include <boost/smart_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

// Some compilers don't like template typedefs
#define SharedPtr boost::shared_ptr
#define WeakPtr boost::weak_ptr
#define AutoPtr boost::scoped_ptr
#define SharedFromThis boost::enable_shared_from_this

#include "Vector.h"
#include "Matrix.h"

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

/** FP 3x3 matrix */
typedef TMatrix3<Scalar> Matrix3;

/** Long FP 3x3 matrix */
typedef TMatrix3<LScalar> LMatrix3;

/** Per-frequency data usually comes in lf/hf bundles */
template<typename T>
struct PerFrequency {
    T lf;
    T hf;

    PerFrequency(T _lf, T _hf) : lf(_lf), hf(_hf) {
    }
};

/** Range data usually comes in min/max bundles */
template<typename T>
struct Range {
    T min;
    T max;

    Range(T mn, T mx) : min(mn), max(mx) {
    }

    T span() const {
        return max - min;
    }

    float phase(T x) const {
        if (min < max) {
            if (x <= min) {
                return 0.f;
            } else if (x >= max) {
                return 1.f;
            } else {
                return float(x - min) / float(max - min);
            }
        } else {
            if (x <= max) {
                return 1.f;
            } else if (x >= min) {
                return 0.f;
            } else {
                return float(x - min) / float(max - min);
            }
        }
    }
};

/** Generic user data container interface */
class UserData {
public:
    virtual ~UserData() {
    };
};

};

#endif //VEGA_STRIKE_ENGINE_AUDIO_TYPES_H
