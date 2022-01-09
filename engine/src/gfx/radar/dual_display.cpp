/**
* dual_display.cpp
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

// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "dual_display.h"

namespace Radar
{

DualDisplayBase::DualDisplayBase()
    : isPaused(false),
      randomEngine(),
      randomDistribution(),
      randomGenerator(randomEngine, randomDistribution),
      lastRandomNumber(0.0)
{
}

void DualDisplayBase::OnPauseBegin()
{
    isPaused = true;
}

void DualDisplayBase::OnPauseEnd()
{
    isPaused = false;
}

float DualDisplayBase::Jitter(float errorOffset, float errorRange)
{
    if (!isPaused)
    {
        lastRandomNumber = randomGenerator();
    }
    return errorOffset + errorRange * lastRandomNumber;
}

void DualDisplayBase::Jitter(float errorOffset, float errorRange, Vector& position)
{
    if (!isPaused)
    {
        lastRandomVector = Vector(randomGenerator(), randomGenerator(), randomGenerator());
    }
    position.x += (-0.5 * errorOffset + (errorRange * lastRandomVector.x)) * position.x;
    position.y += (-0.5 * errorOffset + (errorRange * lastRandomVector.y)) * position.y;
    position.z += (-0.5 * errorOffset + (errorRange * lastRandomVector.z)) * position.z;
}

} // namespace Radar
