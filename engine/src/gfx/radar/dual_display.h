/**
* dual_display.h
*
* Copyright (c) 2001-2002 Daniel Horn
* Copyright (c) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (c) 2019-2021 Stephen G. Tuggy, and other Vega Strike Contributors
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

#ifndef VEGASTRIKE_GFX_RADAR_DUALDISPLAY_H
#define VEGASTRIKE_GFX_RADAR_DUALDISPLAY_H

#include <boost/random.hpp>

#include "radar.h"
#include "viewarea.h"

namespace Radar
{

class DualDisplayBase : public Display
{
    typedef boost::rand48 RandomEngine;
    typedef boost::normal_distribution<float> RandomDistribution;
    typedef boost::variate_generator<RandomEngine&, RandomDistribution> RandomGenerator;
    
public:
    DualDisplayBase();

    void OnPauseBegin();
    void OnPauseEnd();

protected:
    float Jitter(float errorOffset, float errorRange);
    void Jitter(float errorOffset, float errorRange, Vector& position);

protected:
    ViewArea leftRadar;
    ViewArea rightRadar;
    bool isPaused;

    RandomEngine randomEngine;
    RandomDistribution randomDistribution;
    RandomGenerator randomGenerator;

private:
    float lastRandomNumber;
    Vector lastRandomVector;
};

} // namespace Radar

#endif
