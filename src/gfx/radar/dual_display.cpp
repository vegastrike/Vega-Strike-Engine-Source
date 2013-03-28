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
