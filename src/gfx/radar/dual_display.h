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
