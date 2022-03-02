/**
 * Renderer.cpp
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
// C++ Implementation: Audio::Renderer
//

#include "Renderer.h"

namespace Audio {

Renderer::Renderer() {
}

Renderer::~Renderer() {
}

void Renderer::setMeterDistance(Scalar distance) {
    meterDistance = distance;
}

Scalar Renderer::getMeterDistance() const {
    return meterDistance;
}

void Renderer::setDopplerFactor(Scalar factor) {
    dopplerFactor = factor;
}

Scalar Renderer::getDopplerFactor() const {
    return dopplerFactor;
}

void Renderer::setOutputFormat(const Format &format) {
    outputFormat = format;
}

const Format &Renderer::getOutputFormat() const {
    return outputFormat;
}

void Renderer::beginTransaction() {
    // intentionally blank
}

void Renderer::commitTransaction() {
    // intentionally blank
}
};
