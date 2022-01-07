/**
 * SourceTemplate.cpp
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


//
// C++ Implementation: Audio::SourceTemplate
//

#include "SourceTemplate.h"

#include <math.h>

namespace Audio {

SourceTemplate::SourceTemplate(const std::string &sound, VSFileSystem::VSFileType type, bool _looping) :
        soundName(sound),
        soundType(type),
        cosAngleRange(-1, -1),
        pfRadiusRatios(1, 1),
        referenceFreqs(250, 5000),
        gain(1)
{
    setLooping(_looping);
    setStreaming(false);
    setRelative(false);
    setAttenuated(true);
}

SourceTemplate::~SourceTemplate()
{
}

Range<Scalar> SourceTemplate::getAngleRange() const
{
    return Range<Scalar>(Scalar(acos(cosAngleRange.min)),
                         Scalar(acos(cosAngleRange.max)));
}

void SourceTemplate::setAngleRange(Range<Scalar> r)
{
    cosAngleRange.min = Scalar(cos(r.min));
    cosAngleRange.max = Scalar(cos(r.max));
}

};
