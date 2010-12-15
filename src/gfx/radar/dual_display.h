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
public:
    DualDisplayBase();

    void OnPauseBegin();
    void OnPauseEnd();

protected:
    float Jitter(float errorOffset, float errorRange);
    void Jitter(float errorOffset, float errorRange, Vector& position);
    void SetViewArea(VSSprite *, ViewArea&);

protected:
    ViewArea leftRadar;
    ViewArea rightRadar;
    bool isPaused;

    boost::mt19937 randomEngine;
    boost::normal_distribution<float> randomDistribution;
    boost::variate_generator<boost::mt19937&, boost::normal_distribution<float> > randomGenerator;
private:
    float lastRandomNumber;
    Vector lastRandomVector;
};

} // namespace Radar

#endif
