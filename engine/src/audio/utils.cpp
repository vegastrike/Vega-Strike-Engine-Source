/*
 * utils.cpp
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


//
// C++ Implementation: Audio::TemplateManager
//

#include "utils.h"

#include "universe_util.h"
#include "lin_time.h"

#include "Source.h"
#include "Listener.h"

namespace Audio {

Timestamp getGameTime() {
    return Timestamp(UniverseUtil::GetGameTime());
}

Timestamp getRealTime() {
    return Timestamp(realTime());
}

Scalar estimateGain(const Source &src, const Listener &listener) {
    // Base priority is source gain
    Scalar gain = src.getGain();

    // Account for distance attenuation
    LScalar distance = listener.getPosition().distance(src.getPosition())
            - listener.getRadius()
            - src.getRadius();
    LScalar ref = listener.getRadius();
    LScalar rolloff = listener.getRadius() / src.getRadius();
    gain *= (distance <= 0) ? 1.f : float(ref / (ref + rolloff * distance));

    // Account for dispersion/sensing angle limitations
    Scalar cosangle = listener.getAtDirection().dot(src.getDirection());
    if (cosangle < listener.getCosAngleRange().min) {
        gain *= listener.getCosAngleRange().phase(cosangle);
    }
    if (cosangle < src.getCosAngleRange().min) {
        gain *= src.getCosAngleRange().phase(cosangle);
    }

    return gain;
}

void sleep(unsigned int ms) {
    micro_sleep(ms * 1000);
}
};

